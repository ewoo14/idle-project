#include "Misc/AutomationTest.h"

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

// ── ③ 세이브 v17 라운드트립(CachedGuildId/Rank) ────────────────────────────────
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGuildSaveRoundTripTest,
	"IdleProject.Guild.SaveRoundTripV17",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGuildSaveRoundTripTest::RunTest(const FString& Parameters)
{
	// 캡처: GuildService 캐시 → 세이브(v17).
	UIdleGameInstance* SourceInstance = NewObject<UIdleGameInstance>();
	SourceInstance->InitializeGuildServiceForTests();

	FGuildSnapshot Snapshot;
	Snapshot.bHasGuild = true;
	Snapshot.MyRank = EGuildRank::Officer;
	Snapshot.Guild.Id = TEXT("guild-roundtrip");
	SourceInstance->GetGuildService()->ApplySnapshot(Snapshot);

	UIdleSaveGame* SaveGame = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture to save succeeds"), SourceInstance->CaptureToSave(SaveGame));
	TestEqual(TEXT("Captured save writes V17"), SaveGame->SaveVersion, static_cast<int32>(17));
	TestEqual(TEXT("Captured guild id"), SaveGame->CachedGuildId, FString(TEXT("guild-roundtrip")));
	TestEqual(TEXT("Captured guild rank officer"), static_cast<int32>(SaveGame->CachedGuildRank), static_cast<int32>(EGuildRank::Officer));

	// 복원: 세이브(v17) → 새 인스턴스 GuildService 캐시.
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

#endif
