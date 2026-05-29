#include "Misc/AutomationTest.h"

#include "IdleGameInstanceTestHelpers.h"
#include "GameCore/LeaderboardService.h"
#include "GameCore/IdleGameInstance.h"
#include "Internationalization/IdleLocalization.h"
#include "UI/IdleHUD.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLeaderboardServiceParseListTest,
	"IdleProject.Leaderboard.ParseListJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLeaderboardServiceParseListTest::RunTest(const FString& Parameters)
{
	ULeaderboardService* Service = NewObject<ULeaderboardService>();
	const FString Json = TEXT("{\"ok\":true,\"data\":[{\"characterId\":\"char-a\",\"score\":\"922337203685477000\",\"rank\":1},{\"characterId\":\"char-b\",\"score\":2147483648,\"rank\":2}]}");

	const TArray<FLeaderboardEntry> Entries = Service->ParseListJson(Json, ELeaderboardKind::Power);

	TestEqual(TEXT("Two leaderboard entries parse"), Entries.Num(), 2);
	if (Entries.Num() != 2)
	{
		return false;
	}

	TestEqual(TEXT("First character id parses"), Entries[0].CharacterId, FString(TEXT("char-a")));
	TestEqual(TEXT("String int64 score parses"), Entries[0].Score, static_cast<int64>(922337203685477000LL));
	TestEqual(TEXT("First rank parses"), Entries[0].Rank, 1);
	TestEqual(TEXT("Numeric score above int32 parses"), Entries[1].Score, static_cast<int64>(2147483648LL));
	TestEqual(TEXT("Power entries are cached"), Service->GetEntries(ELeaderboardKind::Power).Num(), 2);
	TestEqual(TEXT("Rebirth cache remains independent"), Service->GetEntries(ELeaderboardKind::Rebirth).Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLeaderboardServiceParseMyRankTest,
	"IdleProject.Leaderboard.ParseMyRankJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLeaderboardServiceParseMyRankTest::RunTest(const FString& Parameters)
{
	ULeaderboardService* Service = NewObject<ULeaderboardService>();
	const FString Json = TEXT("{\"ok\":true,\"data\":{\"characterId\":\"self\",\"score\":\"1234567890123\",\"rank\":42}}");

	const FLeaderboardEntry Entry = Service->ParseMyRankJson(Json, ELeaderboardKind::Rebirth);

	TestEqual(TEXT("My rank character id parses"), Entry.CharacterId, FString(TEXT("self")));
	TestEqual(TEXT("My rank int64 score parses"), Entry.Score, static_cast<int64>(1234567890123LL));
	TestEqual(TEXT("My rank parses"), Entry.Rank, 42);
	TestEqual(TEXT("My rebirth rank caches"), Service->GetMyEntry(ELeaderboardKind::Rebirth).Rank, 42);
	TestEqual(TEXT("Power my rank remains default"), Service->GetMyEntry(ELeaderboardKind::Power).Rank, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLeaderboardServiceGracefulJsonTest,
	"IdleProject.Leaderboard.GracefulJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLeaderboardServiceGracefulJsonTest::RunTest(const FString& Parameters)
{
	ULeaderboardService* Service = NewObject<ULeaderboardService>();
	Service->ParseListJson(TEXT("{\"ok\":true,\"data\":[{\"characterId\":\"cached\",\"score\":50,\"rank\":1}]}"), ELeaderboardKind::Power);
	Service->ParseMyRankJson(TEXT("{\"ok\":true,\"data\":{\"characterId\":\"cached\",\"score\":50,\"rank\":1}}"), ELeaderboardKind::Power);

	TestEqual(TEXT("Invalid list returns empty array"), Service->ParseListJson(TEXT("{bad json"), ELeaderboardKind::Power).Num(), 0);
	TestEqual(TEXT("Invalid list clears cached rows gracefully"), Service->GetEntries(ELeaderboardKind::Power).Num(), 0);
	TestEqual(TEXT("Non-ok list returns empty array"), Service->ParseListJson(TEXT("{\"ok\":false,\"data\":[]}"), ELeaderboardKind::Power).Num(), 0);

	const FLeaderboardEntry Missing = Service->ParseMyRankJson(TEXT("{\"ok\":true,\"data\":null}"), ELeaderboardKind::Power);
	TestEqual(TEXT("Missing my rank uses rank zero"), Missing.Rank, 0);
	TestEqual(TEXT("Missing my rank uses zero score"), Missing.Score, static_cast<int64>(0));
	TestTrue(TEXT("Missing my rank has no character id"), Missing.CharacterId.IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLeaderboardServiceRankBoundaryTest,
	"IdleProject.Leaderboard.RankBoundaryJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLeaderboardServiceRankBoundaryTest::RunTest(const FString& Parameters)
{
	ULeaderboardService* Service = NewObject<ULeaderboardService>();
	const FString Json = TEXT("{\"ok\":true,\"data\":[{\"characterId\":\"unranked\",\"score\":0,\"rank\":0},{\"characterId\":\"negative-rank\",\"score\":-1,\"rank\":-10},{\"characterId\":\"huge-rank\",\"score\":\"10\",\"rank\":\"2147483650\"}]}");

	const TArray<FLeaderboardEntry> Entries = Service->ParseListJson(Json, ELeaderboardKind::Rebirth);

	TestEqual(TEXT("Boundary entries parse"), Entries.Num(), 3);
	if (Entries.Num() != 3)
	{
		return false;
	}

	TestEqual(TEXT("Rank zero remains the unranked sentinel"), Entries[0].Rank, 0);
	TestEqual(TEXT("Zero score parses"), Entries[0].Score, static_cast<int64>(0));
	TestEqual(TEXT("Negative score parses without overflow"), Entries[1].Score, static_cast<int64>(-1));
	TestEqual(TEXT("Negative rank clamps to unranked sentinel"), Entries[1].Rank, 0);
	TestEqual(TEXT("Rank above int32 clamps safely"), Entries[2].Rank, MAX_int32);
	TestEqual(TEXT("Rebirth boundary entries are cached"), Service->GetEntries(ELeaderboardKind::Rebirth).Num(), 3);

	const FLeaderboardEntry MyEntry = Service->ParseMyRankJson(
		TEXT("{\"ok\":true,\"data\":{\"characterId\":\"self\",\"score\":0,\"rank\":0}}"),
		ELeaderboardKind::Rebirth);
	TestEqual(TEXT("My rank zero remains the unranked sentinel"), MyEntry.Rank, 0);
	TestEqual(TEXT("My score zero parses"), MyEntry.Score, static_cast<int64>(0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLeaderboardRefreshNoApiClientGracefulTest,
	"IdleProject.Leaderboard.RefreshNoApiClientGraceful",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLeaderboardRefreshNoApiClientGracefulTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	FIdleGameInstanceWorldContextAccessor::Attach(GameInstance, nullptr);

	GameInstance->RefreshLeaderboard(ELeaderboardKind::Power);

	ULeaderboardService* Service = GameInstance->GetLeaderboardService();
	TestNotNull(TEXT("RefreshLeaderboard creates service without ApiClient"), Service);
	if (!Service)
	{
		return false;
	}

	TestEqual(TEXT("No ApiClient leaves top list empty"), Service->GetEntries(ELeaderboardKind::Power).Num(), 0);
	TestEqual(TEXT("No ApiClient leaves my rank at zero"), Service->GetMyEntry(ELeaderboardKind::Power).Rank, 0);
	TestEqual(TEXT("No ApiClient leaves my score at zero"), Service->GetMyEntry(ELeaderboardKind::Power).Score, static_cast<int64>(0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLeaderboardPanelViewModelTest,
	"IdleProject.UI.HUD.LeaderboardPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLeaderboardPanelViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	ULeaderboardService* Service = NewObject<ULeaderboardService>();
	Service->ParseListJson(TEXT("{\"ok\":true,\"data\":[{\"characterId\":\"char-a\",\"score\":1500,\"rank\":1},{\"characterId\":\"self\",\"score\":1200,\"rank\":2}]}"), ELeaderboardKind::Power);
	Service->ParseMyRankJson(TEXT("{\"ok\":true,\"data\":{\"characterId\":\"self\",\"score\":1200,\"rank\":2}}"), ELeaderboardKind::Power);

	const FIdleHUDLeaderboardPanelViewModel ViewModel = IdleProject::UI::BuildLeaderboardPanelViewModel(
		*Service,
		ELeaderboardKind::Power,
		7,
		false,
		false);

	TestEqual(TEXT("Leaderboard panel title localizes"), ViewModel.Title.ToString(), FString(TEXT("Leaderboard")));
	TestEqual(TEXT("Leaderboard season label formats"), ViewModel.SeasonLabel.ToString(), FString(TEXT("Season 7")));
	TestEqual(TEXT("Power tab label localizes"), ViewModel.PowerTabLabel.ToString(), FString(TEXT("Power")));
	TestEqual(TEXT("Rebirth tab label localizes"), ViewModel.RebirthTabLabel.ToString(), FString(TEXT("Rebirth")));
	TestEqual(TEXT("Top entries become rows"), ViewModel.Rows.Num(), 2);
	TestEqual(TEXT("Top row rank formats"), ViewModel.Rows[0].RankLabel.ToString(), FString(TEXT("#1")));
	TestEqual(TEXT("Top row score formats"), ViewModel.Rows[0].ScoreLabel.ToString(), FString(TEXT("Score 1,500")));
	TestFalse(TEXT("Non-self row is not highlighted"), ViewModel.Rows[0].bSelf);
	TestTrue(TEXT("My top-N row is highlighted"), ViewModel.Rows[1].bSelf);
	TestEqual(TEXT("My rank title localizes"), ViewModel.MyRankTitle.ToString(), FString(TEXT("My Rank")));
	TestEqual(TEXT("My rank row formats"), ViewModel.MyEntry.RankLabel.ToString(), FString(TEXT("#2")));
	TestEqual(TEXT("Refresh action localizes"), ViewModel.RefreshLabel.ToString(), FString(TEXT("Refresh")));

	const FIdleHUDLeaderboardPanelViewModel OfflineViewModel = IdleProject::UI::BuildLeaderboardPanelViewModel(
		*Service,
		ELeaderboardKind::Rebirth,
		7,
		true,
		true);
	TestTrue(TEXT("Offline state is exposed"), OfflineViewModel.bOffline);
	TestTrue(TEXT("Loading state is exposed"), OfflineViewModel.bLoading);
	TestEqual(TEXT("Offline label localizes"), OfflineViewModel.OfflineLabel.ToString(), FString(TEXT("Offline")));
	TestEqual(TEXT("Loading label localizes"), OfflineViewModel.LoadingLabel.ToString(), FString(TEXT("Loading...")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLeaderboardServiceParseWeeklyDamageListTest,
	"IdleProject.Leaderboard.ParseWeeklyDamageListJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLeaderboardServiceParseWeeklyDamageListTest::RunTest(const FString& Parameters)
{
	ULeaderboardService* Service = NewObject<ULeaderboardService>();
	const FString Json = TEXT("{\"ok\":true,\"data\":[{\"characterId\":\"boss-a\",\"score\":\"987654321098765\",\"rank\":1},{\"characterId\":\"boss-b\",\"score\":4294967296,\"rank\":2}]}");

	const TArray<FLeaderboardEntry> Entries = Service->ParseListJson(Json, ELeaderboardKind::WeeklyDamage);

	TestEqual(TEXT("Two weekly damage entries parse"), Entries.Num(), 2);
	if (Entries.Num() != 2)
	{
		return false;
	}

	TestEqual(TEXT("First weekly character id parses"), Entries[0].CharacterId, FString(TEXT("boss-a")));
	TestEqual(TEXT("Weekly string int64 damage parses"), Entries[0].Score, static_cast<int64>(987654321098765LL));
	TestEqual(TEXT("First weekly rank parses"), Entries[0].Rank, 1);
	TestEqual(TEXT("Numeric weekly damage above int32 parses"), Entries[1].Score, static_cast<int64>(4294967296LL));
	TestEqual(TEXT("Weekly entries are cached"), Service->GetEntries(ELeaderboardKind::WeeklyDamage).Num(), 2);
	TestEqual(TEXT("Power cache remains independent of weekly"), Service->GetEntries(ELeaderboardKind::Power).Num(), 0);
	TestEqual(TEXT("Rebirth cache remains independent of weekly"), Service->GetEntries(ELeaderboardKind::Rebirth).Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLeaderboardServiceParseMyWeeklyRankTest,
	"IdleProject.Leaderboard.ParseMyWeeklyRankJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLeaderboardServiceParseMyWeeklyRankTest::RunTest(const FString& Parameters)
{
	ULeaderboardService* Service = NewObject<ULeaderboardService>();
	const FString Json = TEXT("{\"ok\":true,\"data\":{\"characterId\":\"self\",\"score\":\"55555555555\",\"rank\":7}}");

	const FLeaderboardEntry Entry = Service->ParseMyRankJson(Json, ELeaderboardKind::WeeklyDamage);

	TestEqual(TEXT("My weekly character id parses"), Entry.CharacterId, FString(TEXT("self")));
	TestEqual(TEXT("My weekly int64 damage parses"), Entry.Score, static_cast<int64>(55555555555LL));
	TestEqual(TEXT("My weekly rank parses"), Entry.Rank, 7);
	TestEqual(TEXT("My weekly rank caches"), Service->GetMyEntry(ELeaderboardKind::WeeklyDamage).Rank, 7);
	TestEqual(TEXT("Power my rank remains default after weekly"), Service->GetMyEntry(ELeaderboardKind::Power).Rank, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLeaderboardServiceWeeklyDamageGracefulTest,
	"IdleProject.Leaderboard.WeeklyDamageGracefulJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLeaderboardServiceWeeklyDamageGracefulTest::RunTest(const FString& Parameters)
{
	ULeaderboardService* Service = NewObject<ULeaderboardService>();
	Service->ParseListJson(TEXT("{\"ok\":true,\"data\":[{\"characterId\":\"cached\",\"score\":\"99\",\"rank\":1}]}"), ELeaderboardKind::WeeklyDamage);
	Service->ParseMyRankJson(TEXT("{\"ok\":true,\"data\":{\"characterId\":\"cached\",\"score\":\"99\",\"rank\":1}}"), ELeaderboardKind::WeeklyDamage);

	TestEqual(TEXT("Invalid weekly list returns empty array"), Service->ParseListJson(TEXT("{bad json"), ELeaderboardKind::WeeklyDamage).Num(), 0);
	TestEqual(TEXT("Invalid weekly list clears cached rows gracefully"), Service->GetEntries(ELeaderboardKind::WeeklyDamage).Num(), 0);
	TestEqual(TEXT("Non-ok weekly list returns empty array"), Service->ParseListJson(TEXT("{\"ok\":false,\"data\":[]}"), ELeaderboardKind::WeeklyDamage).Num(), 0);

	const FLeaderboardEntry Missing = Service->ParseMyRankJson(TEXT("{\"ok\":true,\"data\":null}"), ELeaderboardKind::WeeklyDamage);
	TestEqual(TEXT("Missing weekly rank uses rank zero"), Missing.Rank, 0);
	TestEqual(TEXT("Missing weekly rank uses zero damage"), Missing.Score, static_cast<int64>(0));
	TestTrue(TEXT("Missing weekly rank has no character id"), Missing.CharacterId.IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLeaderboardRefreshWeeklyDamageNoApiClientGracefulTest,
	"IdleProject.Leaderboard.RefreshWeeklyDamageNoApiClientGraceful",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLeaderboardRefreshWeeklyDamageNoApiClientGracefulTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	FIdleGameInstanceWorldContextAccessor::Attach(GameInstance, nullptr);

	GameInstance->RefreshLeaderboard(ELeaderboardKind::WeeklyDamage);

	ULeaderboardService* Service = GameInstance->GetLeaderboardService();
	TestNotNull(TEXT("Weekly refresh creates service without ApiClient"), Service);
	if (!Service)
	{
		return false;
	}

	TestEqual(TEXT("No ApiClient leaves weekly top list empty"), Service->GetEntries(ELeaderboardKind::WeeklyDamage).Num(), 0);
	TestEqual(TEXT("No ApiClient leaves weekly my rank at zero"), Service->GetMyEntry(ELeaderboardKind::WeeklyDamage).Rank, 0);
	TestEqual(TEXT("No ApiClient leaves weekly my damage at zero"), Service->GetMyEntry(ELeaderboardKind::WeeklyDamage).Score, static_cast<int64>(0));

	return true;
}

#endif
