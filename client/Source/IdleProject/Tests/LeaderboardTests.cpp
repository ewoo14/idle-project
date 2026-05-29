#include "Misc/AutomationTest.h"

#include "GameCore/LeaderboardService.h"

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

	TestEqual(TEXT("Invalid list returns empty array"), Service->ParseListJson(TEXT("{bad json"), ELeaderboardKind::Power).Num(), 0);
	TestEqual(TEXT("Non-ok list returns empty array"), Service->ParseListJson(TEXT("{\"ok\":false,\"data\":[]}"), ELeaderboardKind::Power).Num(), 0);

	const FLeaderboardEntry Missing = Service->ParseMyRankJson(TEXT("{\"ok\":true,\"data\":null}"), ELeaderboardKind::Power);
	TestEqual(TEXT("Missing my rank uses rank zero"), Missing.Rank, 0);
	TestEqual(TEXT("Missing my rank uses zero score"), Missing.Score, static_cast<int64>(0));
	TestTrue(TEXT("Missing my rank has no character id"), Missing.CharacterId.IsEmpty());

	return true;
}

#endif
