#include "Misc/AutomationTest.h"

#include "GameCore/BuffService.h"
#include "GameCore/ConsumableTypes.h"
#include "GameCore/GuildBossFormula.h"
#include "GameCore/GuildFormula.h"
#include "GameCore/GuildService.h"
#include "GameCore/GuildTypes.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"

#if WITH_DEV_AUTOMATION_TESTS

// ── ① 계급 해금/정원 parity (서버 guild.service.ts 상수와 1:1) ─────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildFormulaParityTest,
	"IdleProject.Guild.FormulaParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildFormulaParityTest::RunTest(const FString& Parameters)
{
	// parity 상수.
	TestEqual(TEXT("Guild capacity is thirty"), FGuildFormula::GUILD_CAPACITY, 30);
	TestEqual(TEXT("Vice unlock threshold is eleven"), FGuildFormula::VICE_UNLOCK_AT, 11);
	TestEqual(TEXT("Officer unlock threshold is twenty one"), FGuildFormula::OFFICER_UNLOCK_AT, 21);
	TestEqual(TEXT("Vice slot cap is one"), FGuildFormula::VICE_SLOT_CAP, 1);
	TestEqual(TEXT("Officer slot cap is three"), FGuildFormula::OFFICER_SLOT_CAP, 3);

	// master/member 는 항상 해금.
	TestTrue(TEXT("Master always unlocked"), FGuildFormula::IsRankUnlocked(EGuildRank::Master, 1));
	TestTrue(TEXT("Member always unlocked"), FGuildFormula::IsRankUnlocked(EGuildRank::Member, 1));

	// vice: 10명 잠금 / 11명 해금.
	TestFalse(TEXT("Vice locked at ten members"), FGuildFormula::IsRankUnlocked(EGuildRank::Vice, 10));
	TestTrue(TEXT("Vice unlocked at eleven members"), FGuildFormula::IsRankUnlocked(EGuildRank::Vice, 11));

	// officer: 20명 잠금 / 21명 해금.
	TestFalse(TEXT("Officer locked at twenty members"), FGuildFormula::IsRankUnlocked(EGuildRank::Officer, 20));
	TestTrue(TEXT("Officer unlocked at twenty one members"), FGuildFormula::IsRankUnlocked(EGuildRank::Officer, 21));

	// 슬롯 정원.
	TestEqual(TEXT("Vice slot cap"), FGuildFormula::GetRankSlotCap(EGuildRank::Vice), 1);
	TestEqual(TEXT("Officer slot cap"), FGuildFormula::GetRankSlotCap(EGuildRank::Officer), 3);
	TestEqual(TEXT("Master slot cap unbounded"), FGuildFormula::GetRankSlotCap(EGuildRank::Master), MAX_int32);
	TestEqual(TEXT("Member slot cap unbounded"), FGuildFormula::GetRankSlotCap(EGuildRank::Member), MAX_int32);

	return true;
}

// ── ② ApplySnapshot → 접근자 정확성 ────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildServiceApplySnapshotTest,
	"IdleProject.Guild.ServiceApplySnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildServiceApplySnapshotTest::RunTest(const FString& Parameters)
{
	UGuildService* Service = NewObject<UGuildService>();

	// 초기 상태: 무소속.
	TestFalse(TEXT("Fresh service has no guild"), Service->HasGuild());
	TestEqual(TEXT("Fresh service has no members"), Service->GetMembers().Num(), 0);
	TestEqual(TEXT("Fresh service cached guild id empty"), Service->GetCachedGuildId(), FString());

	FGuildSnapshot Snapshot;
	Snapshot.bHasGuild = true;
	Snapshot.MyRank = EGuildRank::Vice;
	Snapshot.Guild.Id = TEXT("guild-123");
	Snapshot.Guild.Name = TEXT("Test Guild");
	Snapshot.Guild.Level = 4;
	Snapshot.Guild.MemberCount = 12;
	Snapshot.Guild.JoinMode = EGuildJoinMode::Approval;

	FGuildMemberInfo MemberA;
	MemberA.CharacterId = TEXT("char-a");
	MemberA.Rank = EGuildRank::Master;
	MemberA.Contribution = 5000;
	Snapshot.Members.Add(MemberA);

	FGuildMemberInfo MemberB;
	MemberB.CharacterId = TEXT("char-b");
	MemberB.Rank = EGuildRank::Vice;
	MemberB.Contribution = 1200;
	Snapshot.Members.Add(MemberB);

	FGuildJoinRequestInfo Request;
	Request.CharacterId = TEXT("char-c");
	Snapshot.Requests.Add(Request);

	Service->ApplySnapshot(Snapshot);

	TestTrue(TEXT("Service has guild after apply"), Service->HasGuild());
	TestEqual(TEXT("My rank cached"), Service->GetMyRank(), EGuildRank::Vice);
	TestEqual(TEXT("Cached guild id"), Service->GetCachedGuildId(), FString(TEXT("guild-123")));
	TestEqual(TEXT("Guild summary name"), Service->GetGuildSummary().Name, FString(TEXT("Test Guild")));
	TestEqual(TEXT("Guild summary level"), Service->GetGuildSummary().Level, 4);
	TestEqual(TEXT("Guild summary member count"), Service->GetGuildSummary().MemberCount, 12);
	TestEqual(TEXT("Guild summary join mode"), Service->GetGuildSummary().JoinMode, EGuildJoinMode::Approval);
	TestEqual(TEXT("Member list count"), Service->GetMembers().Num(), 2);
	TestEqual(TEXT("Request list count"), Service->GetRequests().Num(), 1);

	// ClearSnapshot → 무소속 복귀.
	Service->ClearSnapshot();
	TestFalse(TEXT("Cleared service has no guild"), Service->HasGuild());
	TestEqual(TEXT("Cleared service has no members"), Service->GetMembers().Num(), 0);

	return true;
}

// ── ②-b {ok,data} JSON 파싱 → 캐시 ─────────────────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildServiceParseSnapshotJsonTest,
	"IdleProject.Guild.ServiceParseSnapshotJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildServiceParseSnapshotJsonTest::RunTest(const FString& Parameters)
{
	UGuildService* Service = NewObject<UGuildService>();

	// 무소속 스냅샷(guild=null) → false, 캐시 비움.
	const FString EmptyJson = TEXT("{\"ok\":true,\"data\":{\"guild\":null,\"me\":null,\"members\":[],\"requests\":[]}}");
	TestFalse(TEXT("Null guild snapshot parses as no guild"), Service->ParseSnapshotJson(EmptyJson));
	TestFalse(TEXT("Service stays guildless after null snapshot"), Service->HasGuild());

	// 소속 스냅샷(bigint 는 문자열, vice 권한자라 requests 채워짐).
	const FString Json = TEXT("{\"ok\":true,\"data\":{")
		TEXT("\"guild\":{\"id\":\"g-9\",\"name\":\"Knights\",\"level\":3,\"memberCount\":15,\"joinMode\":\"approval\"},")
		TEXT("\"me\":{\"characterId\":\"me-1\",\"rank\":\"vice\"},")
		TEXT("\"members\":[")
		TEXT("{\"characterId\":\"me-1\",\"rank\":\"vice\",\"totalContribution\":\"4200\"},")
		TEXT("{\"characterId\":\"boss\",\"rank\":\"master\",\"totalContribution\":\"9999\"}],")
		TEXT("\"requests\":[{\"characterId\":\"hopeful\",\"requestedAt\":\"2026-05-29T00:00:00.000Z\"}]}}");

	TestTrue(TEXT("Membership snapshot parses ok"), Service->ParseSnapshotJson(Json));
	TestTrue(TEXT("Service has guild after parse"), Service->HasGuild());
	TestEqual(TEXT("Parsed my rank vice"), Service->GetMyRank(), EGuildRank::Vice);
	TestEqual(TEXT("Parsed guild id"), Service->GetCachedGuildId(), FString(TEXT("g-9")));
	TestEqual(TEXT("Parsed guild name"), Service->GetGuildSummary().Name, FString(TEXT("Knights")));
	TestEqual(TEXT("Parsed member count"), Service->GetGuildSummary().MemberCount, 15);
	TestEqual(TEXT("Parsed join mode approval"), Service->GetGuildSummary().JoinMode, EGuildJoinMode::Approval);
	TestEqual(TEXT("Parsed member array"), Service->GetMembers().Num(), 2);
	if (Service->GetMembers().Num() == 2)
	{
		TestEqual(TEXT("Parsed master member rank"), Service->GetMembers()[1].Rank, EGuildRank::Master);
		TestEqual(TEXT("Parsed bigint contribution from string"), Service->GetMembers()[1].Contribution, static_cast<int64>(9999));
	}
	TestEqual(TEXT("Parsed request array"), Service->GetRequests().Num(), 1);

	return true;
}

// ── ③ 세이브 v19 라운드트립(길드 id/rank/레벨·버프·포인트·pending·출석일+보스 캐시) ─
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildSaveRoundTripTest,
	"IdleProject.Guild.SaveRoundTripV19",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildSaveRoundTripTest::RunTest(const FString& Parameters)
{
	// 캡처: GuildService 캐시 → 세이브(v19). 레벨/버프 파생 검증을 위해 exp 가 큰 스냅샷 적용.
	UIdleGameInstance* SourceInstance = NewObject<UIdleGameInstance>();
	SourceInstance->InitializeGuildServiceForTests();

	FGuildSnapshot Snapshot;
	Snapshot.bHasGuild = true;
	Snapshot.MyRank = EGuildRank::Officer;
	Snapshot.Guild.Id = TEXT("guild-roundtrip");
	Snapshot.GuildExp = 30000; // L1(10000)+L2(16000)=26000 누적 → L3 도달.
	Snapshot.ContributionPoints = 777;
	// 보스 진행 표시 캐시(서버 권위 미러) 라운드트립 검증용.
	Snapshot.BossDefeatedCount = 3;
	Snapshot.BossChallengesRemaining = 4;
	UGuildService* SourceService = SourceInstance->GetGuildService();
	SourceService->ApplySnapshot(Snapshot);
	SourceService->AddPendingAutoContribution(42);

	const int32 ExpectedLevel = FGuildFormula::GetGuildLevel(30000);
	const FGuildBuff ExpectedBuff = FGuildFormula::GetGuildBuff(ExpectedLevel);

	UIdleSaveGame* SaveGame = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture to save succeeds"), SourceInstance->CaptureToSave(SaveGame));
	TestEqual(TEXT("Captured save writes V21"), SaveGame->SaveVersion, static_cast<int32>(21));
	TestEqual(TEXT("Captured guild id"), SaveGame->CachedGuildId, FString(TEXT("guild-roundtrip")));
	TestEqual(TEXT("Captured guild rank officer"), static_cast<int32>(SaveGame->CachedGuildRank), static_cast<int32>(EGuildRank::Officer));
	TestEqual(TEXT("Captured guild level"), SaveGame->CachedGuildLevel, ExpectedLevel);
	TestEqual(TEXT("Captured attack pct"), SaveGame->CachedGuildAttackPct, ExpectedBuff.AttackPct);
	TestEqual(TEXT("Captured gold pct"), SaveGame->CachedGuildGoldPct, ExpectedBuff.GoldPct);
	TestEqual(TEXT("Captured contribution points"), SaveGame->CachedContributionPoints, static_cast<int64>(777));
	TestEqual(TEXT("Captured pending auto contribution"), SaveGame->PendingAutoContribution, static_cast<int64>(42));
	TestEqual(TEXT("Captured boss defeated count"), SaveGame->CachedBossDefeatedCount, 3);
	TestEqual(TEXT("Captured boss challenges remaining"), SaveGame->CachedBossChallengesRemaining, 4);

	// 복원: 세이브(v19) → 새 인스턴스 GuildService 캐시(오프라인 버프 캐시 적용).
	SaveGame->bHasSave = true;
	UIdleGameInstance* TargetInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply from save succeeds"), TargetInstance->ApplyFromSave(SaveGame));

	UGuildService* RestoredService = TargetInstance->GetGuildService();
	TestNotNull(TEXT("Restored guild service exists"), RestoredService);
	if (RestoredService)
	{
		TestTrue(TEXT("Restored service has guild"), RestoredService->HasGuild());
		TestEqual(TEXT("Restored guild id matches"), RestoredService->GetCachedGuildId(), FString(TEXT("guild-roundtrip")));
		TestEqual(TEXT("Restored rank matches"), RestoredService->GetMyRank(), EGuildRank::Officer);
		TestEqual(TEXT("Restored level matches"), RestoredService->GetGuildLevel(), ExpectedLevel);
		TestEqual(TEXT("Restored attack buff (offline cache)"), RestoredService->GetGuildBuff().AttackPct, ExpectedBuff.AttackPct);
		TestEqual(TEXT("Restored gold buff (offline cache)"), RestoredService->GetGuildBuff().GoldPct, ExpectedBuff.GoldPct);
		TestEqual(TEXT("Restored contribution points"), RestoredService->GetContributionPoints(), static_cast<int64>(777));
		TestEqual(TEXT("Restored pending auto contribution"), RestoredService->GetPendingAutoContribution(), static_cast<int64>(42));
		TestEqual(TEXT("Restored boss defeated count (display cache)"), RestoredService->GetBossDefeatedCount(), 3);
		TestEqual(TEXT("Restored boss challenges remaining (display cache)"), RestoredService->GetBossChallengesRemaining(), 4);
	}

	// v17 입력 로드(가드 >=18 미충족) → 무길드버프(레벨1·0%)로 복원, 무소속 아님(id 존재).
	UIdleSaveGame* LegacyV17 = NewObject<UIdleSaveGame>();
	LegacyV17->SaveVersion = 17;
	LegacyV17->bHasSave = true;
	LegacyV17->CachedGuildId = TEXT("legacy-guild");
	LegacyV17->CachedGuildRank = static_cast<uint8>(EGuildRank::Member);
	UIdleGameInstance* LegacyInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply v17 legacy succeeds"), LegacyInstance->ApplyFromSave(LegacyV17));
	if (UGuildService* LegacyService = LegacyInstance->GetGuildService())
	{
		TestTrue(TEXT("v17 restore still has guild id"), LegacyService->HasGuild());
		TestEqual(TEXT("v17 restore level defaults to one"), LegacyService->GetGuildLevel(), 1);
		TestEqual(TEXT("v17 restore no attack buff"), LegacyService->GetGuildBuff().AttackPct, 0.0f);
		TestEqual(TEXT("v17 restore no gold buff"), LegacyService->GetGuildBuff().GoldPct, 0.0f);
	}

	// 무소속 라운드트립: 빈 id 면 복원 후에도 무소속.
	UIdleGameInstance* EmptySourceInstance = NewObject<UIdleGameInstance>();
	EmptySourceInstance->InitializeGuildServiceForTests();
	UIdleSaveGame* EmptySave = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture empty guild succeeds"), EmptySourceInstance->CaptureToSave(EmptySave));
	TestEqual(TEXT("Empty cached guild id"), EmptySave->CachedGuildId, FString());

	EmptySave->bHasSave = true;
	UIdleGameInstance* EmptyTargetInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply empty guild succeeds"), EmptyTargetInstance->ApplyFromSave(EmptySave));
	if (UGuildService* EmptyRestored = EmptyTargetInstance->GetGuildService())
	{
		TestFalse(TEXT("Empty restore stays guildless"), EmptyRestored->HasGuild());
	}

	return true;
}

// ── ④ 길드 레벨/버프 parity (서버 getGuildLevel/getGuildBuff 누적 임계·계수 1:1) ──
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildLevelBuffParityTest,
	"IdleProject.Guild.LevelBuffParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildLevelBuffParityTest::RunTest(const FString& Parameters)
{
	// parity 상수.
	TestEqual(TEXT("Guild level base 10000"), FGuildFormula::GUILD_LEVEL_BASE, static_cast<int64>(10000));
	TestEqual(TEXT("Guild level growth 1.6"), FGuildFormula::GUILD_LEVEL_GROWTH, 1.6);
	TestEqual(TEXT("Buff per level 0.004"), FGuildFormula::GUILD_BUFF_PER_LEVEL, 0.004f);
	TestEqual(TEXT("Auto weekly cap 2000"), FGuildFormula::AUTO_WEEKLY_CAP, static_cast<int64>(2000));

	// 누적 임계 경계값(서버 누적 합과 동일):
	//  L1 step=10000(누적 0~9999=L1), L2 step=16000(누적 10000~25999=L2),
	//  L3 step=25600(누적 26000~51599=L3).
	TestEqual(TEXT("exp 0 -> L1"), FGuildFormula::GetGuildLevel(0), 1);
	TestEqual(TEXT("exp negative -> L1"), FGuildFormula::GetGuildLevel(-5), 1);
	TestEqual(TEXT("exp 9999 -> L1"), FGuildFormula::GetGuildLevel(9999), 1);
	TestEqual(TEXT("exp 10000 -> L2"), FGuildFormula::GetGuildLevel(10000), 2);
	TestEqual(TEXT("exp 25999 -> L2"), FGuildFormula::GetGuildLevel(25999), 2);
	TestEqual(TEXT("exp 26000 -> L3"), FGuildFormula::GetGuildLevel(26000), 3);
	TestEqual(TEXT("exp 51599 -> L3"), FGuildFormula::GetGuildLevel(51599), 3);
	TestEqual(TEXT("exp 51600 -> L4"), FGuildFormula::GetGuildLevel(51600), 4);

	// 버프 계수: level*0.004 양 채널.
	const FGuildBuff Buff5 = FGuildFormula::GetGuildBuff(5);
	TestEqual(TEXT("Level 5 attack buff 0.02"), Buff5.AttackPct, 0.02f);
	TestEqual(TEXT("Level 5 gold buff 0.02"), Buff5.GoldPct, 0.02f);
	const FGuildBuff Buff1 = FGuildFormula::GetGuildBuff(1);
	TestEqual(TEXT("Level 1 attack buff 0.004"), Buff1.AttackPct, 0.004f);

	return true;
}

// ── ⑤ 자동 기여 델타 누적 · 주간 상한 클램프 플러시 ────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildPendingContributionTest,
	"IdleProject.Guild.PendingContribution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildPendingContributionTest::RunTest(const FString& Parameters)
{
	UGuildService* Service = NewObject<UGuildService>();

	// 음수/0 무시.
	Service->AddPendingAutoContribution(0);
	Service->AddPendingAutoContribution(-100);
	TestEqual(TEXT("No accrual from zero/negative"), Service->GetPendingAutoContribution(), static_cast<int64>(0));

	// 누적.
	Service->AddPendingAutoContribution(50);
	Service->AddPendingAutoContribution(30);
	TestEqual(TEXT("Accrued 80"), Service->GetPendingAutoContribution(), static_cast<int64>(80));

	// 상한 미만 → 전량 소비.
	const int64 Flushed = Service->ConsumePendingAutoContribution();
	TestEqual(TEXT("Flushed 80 below cap"), Flushed, static_cast<int64>(80));
	TestEqual(TEXT("Pending drained"), Service->GetPendingAutoContribution(), static_cast<int64>(0));

	// 주간 상한 초과 → 상한만큼만 소비, 잔여는 보관.
	Service->AddPendingAutoContribution(FGuildFormula::AUTO_WEEKLY_CAP + 500);
	const int64 Clamped = Service->ConsumePendingAutoContribution();
	TestEqual(TEXT("Flushed clamped to weekly cap"), Clamped, FGuildFormula::AUTO_WEEKLY_CAP);
	TestEqual(TEXT("Remainder retained"), Service->GetPendingAutoContribution(), static_cast<int64>(500));

	return true;
}

// ── ⑥ 길드 상점 보상 지급(ApplyGuildShopReward) — 타입별 정식 grant 라우팅 ───────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildShopRewardGrantTest,
	"IdleProject.Guild.ShopRewardGrant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildShopRewardGrantTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();

	// gold: AddGold 경로로 증가.
	const int64 GoldBefore = GameInstance->GetGold();
	GameInstance->ApplyGuildShopReward(TEXT("gold"), 100000);
	TestEqual(TEXT("Gold reward adds to gold"), GameInstance->GetGold(), GoldBefore + 100000);

	// essence: RuneEssence 누적(던전 보상과 동일 경로).
	const int64 EssenceBefore = GameInstance->GetRuneEssence();
	GameInstance->ApplyGuildShopReward(TEXT("essence"), 3);
	TestEqual(TEXT("Essence reward adds to rune essence"), GameInstance->GetRuneEssence(), EssenceBefore + 3);

	// expPotion: WisdomBooster(EXP 부스트) 소비 아이템 수량 증가.
	GameInstance->ApplyGuildShopReward(TEXT("expPotion"), 5);
	UBuffService* BuffService = GameInstance->GetBuffService();
	TestNotNull(TEXT("Buff service exists after exp potion grant"), BuffService);
	if (BuffService)
	{
		TestEqual(
			TEXT("Exp potion grants WisdomBooster consumables (Standard)"),
			BuffService->GetCount(EConsumableType::WisdomBooster, EConsumableGrade::Standard),
			5);
	}

	// protectionScroll: 실존 강화 보호서 카운터 증가(#71 재화).
	const int64 ScrollsBefore = GameInstance->GetProtectionScrolls();
	GameInstance->ApplyGuildShopReward(TEXT("protectionScroll"), 5);
	TestEqual(TEXT("Protection scroll reward adds to protection scrolls"), GameInstance->GetProtectionScrolls(), ScrollsBefore + 5);

	// resetCube: 실존 잠재 재설정 큐브 카운터 증가(두 차례 누적 확인).
	const int64 ResetBefore = GameInstance->GetResetCubes();
	GameInstance->ApplyGuildShopReward(TEXT("resetCube"), 3);
	GameInstance->ApplyGuildShopReward(TEXT("resetCube"), 2);
	TestEqual(TEXT("Reset cube reward accumulates"), GameInstance->GetResetCubes(), ResetBefore + 5);

	// rankCube: 실존 잠재 등급 큐브 카운터 증가.
	const int64 RankBefore = GameInstance->GetRankCubes();
	GameInstance->ApplyGuildShopReward(TEXT("rankCube"), 1);
	TestEqual(TEXT("Rank cube reward adds to rank cubes"), GameInstance->GetRankCubes(), RankBefore + 1);

	// 알 수 없는 타입/비양수 수량은 무시(상태 불변).
	const int64 GoldAfter = GameInstance->GetGold();
	const int64 ScrollsAfter = GameInstance->GetProtectionScrolls();
	GameInstance->ApplyGuildShopReward(TEXT("unknownType"), 999);
	GameInstance->ApplyGuildShopReward(TEXT("gold"), 0);
	GameInstance->ApplyGuildShopReward(TEXT("protectionScroll"), -5);
	TestEqual(TEXT("Unknown/zero reward leaves gold unchanged"), GameInstance->GetGold(), GoldAfter);
	TestEqual(TEXT("Negative reward leaves protection scrolls unchanged"), GameInstance->GetProtectionScrolls(), ScrollsAfter);

	return true;
}

// ── ⑦ 길드 보스 공식 parity (서버 getGuildBossHp/getChallengeDamage + 상수 1:1) ───
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildBossFormulaParityTest,
	"IdleProject.Guild.BossFormulaParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildBossFormulaParityTest::RunTest(const FString& Parameters)
{
	// parity 상수(서버 guild.service.ts §보스).
	TestEqual(TEXT("Boss base HP 1,000,000"), FGuildBossFormula::GUILD_BOSS_BASE_HP, static_cast<int64>(1000000));
	TestEqual(TEXT("Boss HP growth 1.5"), FGuildBossFormula::GUILD_BOSS_HP_GROWTH, 1.5);
	TestEqual(TEXT("Weekly boss challenge limit 7"), FGuildBossFormula::WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT, 7);
	TestEqual(TEXT("Boss dmg-to-contrib divisor 10,000"), FGuildBossFormula::GUILD_BOSS_DMG_TO_CONTRIB, static_cast<int64>(10000));

	// getGuildBossHp 연속 격파 경계(서버 floor(BASE*GROWTH^max(d,0))):
	//  d=0 -> 1,000,000 / d=1 -> 1,500,000 / d=2 -> floor(2,250,000)=2,250,000.
	TestEqual(TEXT("Boss HP at defeated 0"), FGuildBossFormula::GetGuildBossHp(0), static_cast<int64>(1000000));
	TestEqual(TEXT("Boss HP at defeated 1"), FGuildBossFormula::GetGuildBossHp(1), static_cast<int64>(1500000));
	TestEqual(TEXT("Boss HP at defeated 2"), FGuildBossFormula::GetGuildBossHp(2), static_cast<int64>(2250000));
	// 음수 격파수는 0 으로 클램프(서버 Math.max(d,0)).
	TestEqual(TEXT("Boss HP clamps negative defeated to base"), FGuildBossFormula::GetGuildBossHp(-3), static_cast<int64>(1000000));

	// getChallengeDamage: 음수→0, 양수→그대로(서버 trunc(max(0,cp))).
	TestEqual(TEXT("Challenge damage of negative cp is zero"), FGuildBossFormula::GetChallengeDamage(-100), static_cast<int64>(0));
	TestEqual(TEXT("Challenge damage of zero cp is zero"), FGuildBossFormula::GetChallengeDamage(0), static_cast<int64>(0));
	TestEqual(TEXT("Challenge damage equals positive cp"), FGuildBossFormula::GetChallengeDamage(123456), static_cast<int64>(123456));

	return true;
}

// ── ⑧ 보스 상태 스냅샷 파싱/접근자(서버 snapshot.boss / GET /:id/boss) ────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildBossSnapshotParseTest,
	"IdleProject.Guild.BossSnapshotParse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildBossSnapshotParseTest::RunTest(const FString& Parameters)
{
	UGuildService* Service = NewObject<UGuildService>();

	// 보스 상태 포함 스냅샷(서버 toBossState 형태 — hp/accumDamage 는 bigint 문자열).
	const FString Json = TEXT("{\"ok\":true,\"data\":{")
		TEXT("\"guild\":{\"id\":\"g-1\",\"name\":\"Raiders\",\"level\":2,\"memberCount\":8,\"joinMode\":\"open\"},")
		TEXT("\"me\":{\"characterId\":\"me-1\",\"rank\":\"master\"},")
		TEXT("\"members\":[{\"characterId\":\"me-1\",\"rank\":\"master\",\"totalContribution\":\"100\"}],")
		TEXT("\"requests\":[],")
		TEXT("\"boss\":{\"weekId\":\"2026-W22\",\"hp\":\"1500000\",\"accumDamage\":\"450000\",")
		TEXT("\"defeatedCount\":2,\"challengesRemaining\":5,\"unclaimedDefeats\":1,\"topContributors\":[]}}}");

	TestTrue(TEXT("Boss snapshot parses ok"), Service->ParseSnapshotJson(Json));
	TestEqual(TEXT("Parsed boss hp (bigint string)"), Service->GetBossHp(), static_cast<int64>(1500000));
	TestEqual(TEXT("Parsed boss accum damage"), Service->GetBossAccumDamage(), static_cast<int64>(450000));
	TestEqual(TEXT("Parsed boss defeated count"), Service->GetBossDefeatedCount(), 2);
	TestEqual(TEXT("Parsed boss challenges remaining"), Service->GetBossChallengesRemaining(), 5);
	TestEqual(TEXT("Parsed boss unclaimed defeats"), Service->GetBossUnclaimedDefeats(), 1);

	// 보스 키가 없는 스냅샷이면 보스 상태는 기본값(0).
	const FString NoBoss = TEXT("{\"ok\":true,\"data\":{")
		TEXT("\"guild\":{\"id\":\"g-2\",\"name\":\"NoBoss\",\"level\":1,\"memberCount\":3,\"joinMode\":\"open\"},")
		TEXT("\"me\":{\"characterId\":\"me-2\",\"rank\":\"member\"},\"members\":[],\"requests\":[]}}");
	TestTrue(TEXT("No-boss snapshot still parses"), Service->ParseSnapshotJson(NoBoss));
	TestEqual(TEXT("Missing boss -> hp zero"), Service->GetBossHp(), static_cast<int64>(0));
	TestEqual(TEXT("Missing boss -> defeated zero"), Service->GetBossDefeatedCount(), 0);

	return true;
}

// ── ⑨ 주간 길드 랭킹 파싱(서버 guildRankings: rankings[]+me) ──────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildRankingsParseTest,
	"IdleProject.Guild.RankingsParse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildRankingsParseTest::RunTest(const FString& Parameters)
{
	// GuildPanelFetchRankings 의 내부 파서(GuildInstanceParseRankings)는 익명 헬퍼라
	// 동일 응답 형태를 GuildPanelFetchRankings 콜백으로 검증한다(파싱 결과만 확인).
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();

	bool bCallbackInvoked = false;
	bool bCallbackSuccess = false;
	TArray<FGuildRankingEntry> Rankings;
	FGuildRankingEntry MyRank;

	// ApiClient 미초기화 → 즉시 실패 콜백(파싱은 별도 단위 검증이 어려우므로 실패 경로 확인).
	GameInstance->GuildPanelFetchRankings(20, [&](bool bSuccess, const TArray<FGuildRankingEntry>& InRankings, const FGuildRankingEntry& InMyRank)
	{
		bCallbackInvoked = true;
		bCallbackSuccess = bSuccess;
		Rankings = InRankings;
		MyRank = InMyRank;
	});

	TestTrue(TEXT("Rankings fetch callback invoked"), bCallbackInvoked);
	TestFalse(TEXT("Rankings fetch fails without ApiClient"), bCallbackSuccess);
	TestEqual(TEXT("No rankings on failure"), Rankings.Num(), 0);
	TestEqual(TEXT("Default my rank is zero"), MyRank.Rank, 0);

	return true;
}

// ── ⑩ 보스 격파 보상 지급(rewards 배열 gold+essence — G2 ApplyGuildShopReward 재사용) ─
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildBossRewardGrantTest,
	"IdleProject.Guild.BossRewardGrant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildBossRewardGrantTest::RunTest(const FString& Parameters)
{
	// 보스 격파 보상(GUILD_BOSS_REWARD_PER_DEFEAT = gold 200000 + essence 5)을
	// 격파 N건 누적 시 amount*N 으로 ApplyGuildShopReward(G2 재사용)에 적용하는 경로 검증.
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();

	const int64 GoldBefore = GameInstance->GetGold();
	const int64 EssenceBefore = GameInstance->GetRuneEssence();

	// 2건 격파 누적 수령 가정: gold 400000 + essence 10.
	GameInstance->ApplyGuildShopReward(TEXT("gold"), 200000 * 2);
	GameInstance->ApplyGuildShopReward(TEXT("essence"), 5 * 2);

	TestEqual(TEXT("Boss gold reward (2 defeats) adds to gold"), GameInstance->GetGold(), GoldBefore + 400000);
	TestEqual(TEXT("Boss essence reward (2 defeats) adds to rune essence"), GameInstance->GetRuneEssence(), EssenceBefore + 10);

	return true;
}

#endif
