#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameCore/MapThemeLibrary.h"
#include "IdleProjectGameModeBase.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapThemeLibraryTest,
	"IdleProject.MapTheme.Library",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMapThemeLibraryTest::RunTest(const FString& Parameters)
{
	// 1~8 모두 프롭 보유 + 유효 강도.
	for (int32 C = 1; C <= FMapThemeLibrary::ThemeCount; ++C)
	{
		const FMapTheme T = FMapThemeLibrary::GetTheme(C);
		TestTrue(FString::Printf(TEXT("chapter %d has props"), C), T.Props.Num() > 0);
		TestTrue(FString::Printf(TEXT("chapter %d sun intensity"), C), T.SunIntensity > 0.0f);
		TestTrue(FString::Printf(TEXT("chapter %d fog density"), C), T.FogDensity > 0.0f);
	}
	// 클램프: 0/9 → 1/8 동일.
	const FMapTheme Low = FMapThemeLibrary::GetTheme(0);
	const FMapTheme C1 = FMapThemeLibrary::GetTheme(1);
	TestEqual(TEXT("clamp low to ch1 intensity"), Low.SunIntensity, C1.SunIntensity);
	const FMapTheme High = FMapThemeLibrary::GetTheme(99);
	const FMapTheme C8 = FMapThemeLibrary::GetTheme(8);
	TestEqual(TEXT("clamp high to ch8 intensity"), High.SunIntensity, C8.SunIntensity);
	// 챕터별 차별화: ch1(녹) vs ch7(적) 태양색 상이.
	TestTrue(TEXT("ch1 vs ch7 distinct sun"), !FMapThemeLibrary::GetTheme(1).SunColor.Equals(FMapThemeLibrary::GetTheme(7).SunColor));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapThemeMaterialAssetTest,
	"IdleProject.MapTheme.MaterialAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMapThemeMaterialAssetTest::RunTest(const FString& Parameters)
{
	const FSoftObjectPath Path(TEXT("/Game/Maps/M_MapTheme.M_MapTheme"));
	UMaterialInterface* Mat = Cast<UMaterialInterface>(Path.TryLoad());
	TestNotNull(TEXT("M_MapTheme 로드"), Mat);
	if (!Mat)
	{
		return false;
	}
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Mat, nullptr);
	FLinearColor Out;
	const bool bHas = MID && MID->GetVectorParameterValue(FMaterialParameterInfo(TEXT("Color")), Out);
	TestTrue(TEXT("Color 벡터 파라미터 존재"), bHas);

	// M_MapTheme Metallic 스칼라 파라미터.
	{
		UMaterialInstanceDynamic* M2 = UMaterialInstanceDynamic::Create(Mat, nullptr);
		float V;
		TestTrue(TEXT("M_MapTheme Metallic 파라미터"), M2 && M2->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Metallic")), V));
	}
	// M_Sky 로드 + SkyTint 파라미터.
	UMaterialInterface* SkyMat = Cast<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Maps/M_Sky.M_Sky")).TryLoad());
	TestNotNull(TEXT("M_Sky 로드"), SkyMat);
	if (SkyMat)
	{
		UMaterialInstanceDynamic* SkyMID = UMaterialInstanceDynamic::Create(SkyMat, nullptr);
		FLinearColor Tint;
		TestTrue(TEXT("M_Sky SkyTint 파라미터"), SkyMID && SkyMID->GetVectorParameterValue(FMaterialParameterInfo(TEXT("SkyTint")), Tint));
	}
	// TC_MapSky 큐브맵 로드.
	UObject* CubeObj = FSoftObjectPath(TEXT("/Game/Maps/TC_MapSky.TC_MapSky")).TryLoad();
	TestNotNull(TEXT("TC_MapSky 로드"), CubeObj);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapThemeApplyTest,
	"IdleProject.MapTheme.Apply",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMapThemeApplyTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("transient world"), World);
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleProjectGameModeBase* GM = World->SpawnActor<AIdleProjectGameModeBase>(AIdleProjectGameModeBase::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("game mode"), GM);
	if (!GM)
	{
		World->DestroyWorld(false);
		return false;
	}
	GM->SpawnDefaultEnvironmentForTest(); // 환경 spawn 공개 진입점(Sun/Sky/Ground 보관).

	// ch1 적용 → 프롭 수 = 테마 프롭 수.
	GM->ApplyMapTheme(1);
	const int32 Ch1Props = FMapThemeLibrary::GetTheme(1).Props.Num();
	TestEqual(TEXT("ch1 prop count"), GM->GetThemePropCountForTest(), Ch1Props);

	// ch7 전환 → 프롭 교체(ch7 수).
	GM->ApplyMapTheme(7);
	const int32 Ch7Props = FMapThemeLibrary::GetTheme(7).Props.Num();
	TestEqual(TEXT("ch7 prop count after switch"), GM->GetThemePropCountForTest(), Ch7Props);

	// 에셋이 적용되면 바닥 머티리얼이 M_MapTheme 기반 MID + Color == 테마 GroundColor.
	GM->ApplyMapTheme(1);
	if (UMaterialInterface* GroundMat = GM->GetGroundMaterialForTest())
	{
		FLinearColor GroundColor;
		if (GM->GetGroundColorForTest(GroundColor))
		{
			const FLinearColor Expected = FMapThemeLibrary::GetTheme(1).GroundColor;
			TestTrue(TEXT("ground MID Color == ch1 GroundColor"), GroundColor.Equals(Expected, 0.01f));
		}
	}

	// 안개 밀도 == 테마.
	TestEqual(TEXT("fog density == ch1"), GM->GetFogDensityForTest(), FMapThemeLibrary::GetTheme(1).FogDensity);
	// 스카이 틴트(에셋 유효 시) == 테마.
	FLinearColor SkyTint;
	if (GM->GetSkyTintForTest(SkyTint))
	{
		TestTrue(TEXT("sky tint == ch1"), SkyTint.Equals(FMapThemeLibrary::GetTheme(1).SkyTint, 0.01f));
	}
	// 프롭 0 질감 스칼라(에셋 유효 시) — Roughness 존재.
	float PropRough;
	if (GM->GetPropScalarForTest(0, TEXT("Roughness"), PropRough))
	{
		TestTrue(TEXT("prop0 roughness set"), PropRough >= 0.0f);
	}

	World->DestroyWorld(false);
	return true;
}

#endif
