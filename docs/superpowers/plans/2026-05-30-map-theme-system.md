# 맵 테마 시스템 (챕터별 시각 차별화) 구현 Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans. Steps use checkbox (`- [ ]`).

**Goal:** 챕터별 맵 테마(조명+바닥색+절차적 프롭)를 데이터 구동으로 정의하고, GameMode가 챕터 변경 시 적용한다. 클라 전용 시각 — 서버/세이브 무변경.

**Architecture:** `FMapTheme`/`FMapProp`(soft 메시=하이브리드) + `FMapThemeLibrary`(8 챕터 테마) + GameMode가 spawn한 Sun/Sky/Ground/Props 참조를 보관하고 `ApplyMapTheme(chapter)`로 갱신, `StageService::OnStageChanged` 바인딩으로 챕터 전환 감지. 조명이 주 무드 동인(헤드리스 테스트), 프롭은 실루엣.

**Tech Stack:** UE5 C++ (StaticMeshActor/DirectionalLight/SkyLight, Engine BasicShapes). 게이트 `tools/ci/ue-automation.ps1`.

**스펙:** `docs/superpowers/specs/2026-05-30-map-theme-system-design.md`. 브랜치 `feat/map-theme-system`.
**선행 코드:** `IdleProjectGameModeBase.cpp SpawnDefaultEnvironment()`(Sun/Sky/Ground spawn), `StageService::OnStageChanged(FStageInfo)`.

**제외:** 임포트 아트 에셋(후속 soft 경로 교체), 서버/세이브/parity 변경, 안개/큐브맵, 게임플레이 지형(프롭 NoCollision).

---

## Task 1: 데이터 타입 + 테마 라이브러리

**Files:** Create `client/Source/IdleProject/GameCore/MapThemeTypes.h`, `MapThemeLibrary.h`, `MapThemeLibrary.cpp`; Test `client/Source/IdleProject/Tests/MapThemeTests.cpp`.

- [ ] **Step 1: 타입 헤더**

`MapThemeTypes.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "MapThemeTypes.generated.h"

// 단일 데코 프롭. Mesh 는 soft 경로(지금=Engine 기본 셰이프, 후속=임포트 에셋 교체=하이브리드).
USTRUCT(BlueprintType)
struct FMapProp
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FVector Scale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FLinearColor Color = FLinearColor::White;
};

// 챕터 1개 맵 테마. 조명이 주 무드, 프롭이 실루엣.
USTRUCT(BlueprintType)
struct FMapTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FLinearColor SunColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	float SunIntensity = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FRotator SunRotation = FRotator(-45.0f, 45.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	float SkyIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FLinearColor SkyColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FLinearColor GroundColor = FLinearColor(0.2f, 0.2f, 0.2f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	TArray<FMapProp> Props;
};
```

- [ ] **Step 2: 라이브러리 헤더**

`MapThemeLibrary.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameCore/MapThemeTypes.h"

/** 챕터별 맵 테마 제공(클라 전용 시각). 8 챕터 + 범위 클램프. */
class IDLEPROJECT_API FMapThemeLibrary
{
public:
	static constexpr int32 ThemeCount = 8;

	// 챕터(1~8) 테마. 범위 밖은 [1,8] 로 클램프.
	static FMapTheme GetTheme(int32 Chapter);
};
```

- [ ] **Step 3: 라이브러리 구현 (8 팔레트)**

`MapThemeLibrary.cpp`. 기본 셰이프 경로: `/Engine/BasicShapes/Cylinder.Cylinder`, `Cone.Cone`, `Cube.Cube`, `Sphere.Sphere`. 프롭 헬퍼로 8 테마 작성:

```cpp
#include "GameCore/MapThemeLibrary.h"

namespace
{
	FMapProp MakeProp(const TCHAR* MeshPath, const FVector& Loc, const FVector& Scale, const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator)
	{
		FMapProp P;
		P.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(MeshPath));
		P.Location = Loc;
		P.Scale = Scale;
		P.Color = Color;
		P.Rotation = Rot;
		return P;
	}

	const TCHAR* Cyl = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
	const TCHAR* Cone = TEXT("/Engine/BasicShapes/Cone.Cone");
	const TCHAR* Cube = TEXT("/Engine/BasicShapes/Cube.Cube");
}

FMapTheme FMapThemeLibrary::GetTheme(int32 Chapter)
{
	const int32 C = FMath::Clamp(Chapter, 1, ThemeCount);
	FMapTheme T;
	switch (C)
	{
	case 1: // 초원/자연 — 밝은 따뜻한 녹색
		T.SunColor = FLinearColor(1.0f, 0.96f, 0.82f); T.SunIntensity = 6.0f; T.SunRotation = FRotator(-45, 45, 0);
		T.SkyIntensity = 1.2f; T.SkyColor = FLinearColor(0.7f, 0.85f, 1.0f); T.GroundColor = FLinearColor(0.25f, 0.45f, 0.2f);
		T.Props = {
			MakeProp(Cyl,  FVector(-700, 300, 60),  FVector(0.6f, 0.6f, 1.2f), FLinearColor(0.35f, 0.22f, 0.1f)),
			MakeProp(Cone, FVector(-700, 300, 160), FVector(1.4f, 1.4f, 1.6f), FLinearColor(0.2f, 0.5f, 0.18f)),
			MakeProp(Cyl,  FVector(750, -260, 60),  FVector(0.6f, 0.6f, 1.2f), FLinearColor(0.35f, 0.22f, 0.1f)),
			MakeProp(Cone, FVector(750, -260, 160), FVector(1.4f, 1.4f, 1.6f), FLinearColor(0.2f, 0.5f, 0.18f)),
			MakeProp(Cone, FVector(0, 520, 90),     FVector(1.0f, 1.0f, 1.0f), FLinearColor(0.25f, 0.55f, 0.2f)),
		};
		break;
	case 2: // 숲 심부 — 짙은 녹
		T.SunColor = FLinearColor(0.85f, 0.92f, 0.7f); T.SunIntensity = 4.5f; T.SunRotation = FRotator(-50, 30, 0);
		T.SkyIntensity = 0.9f; T.SkyColor = FLinearColor(0.55f, 0.7f, 0.6f); T.GroundColor = FLinearColor(0.15f, 0.3f, 0.15f);
		T.Props = {
			MakeProp(Cyl,  FVector(-650, 280, 80),  FVector(0.7f, 0.7f, 1.8f), FLinearColor(0.3f, 0.2f, 0.1f)),
			MakeProp(Cone, FVector(-650, 280, 230), FVector(1.6f, 1.6f, 2.2f), FLinearColor(0.12f, 0.35f, 0.12f)),
			MakeProp(Cyl,  FVector(820, -200, 80),  FVector(0.7f, 0.7f, 1.8f), FLinearColor(0.3f, 0.2f, 0.1f)),
			MakeProp(Cone, FVector(820, -200, 230), FVector(1.6f, 1.6f, 2.2f), FLinearColor(0.12f, 0.35f, 0.12f)),
			MakeProp(Cone, FVector(-100, 600, 120), FVector(1.2f, 1.2f, 1.4f), FLinearColor(0.1f, 0.3f, 0.1f)),
			MakeProp(Cone, FVector(300, 540, 100),  FVector(1.0f, 1.0f, 1.2f), FLinearColor(0.14f, 0.36f, 0.14f)),
		};
		break;
	case 3: // 차원 그림자 — 어두운 보라, 각진 큐브
		T.SunColor = FLinearColor(0.5f, 0.4f, 0.7f); T.SunIntensity = 3.0f; T.SunRotation = FRotator(-35, 60, 0);
		T.SkyIntensity = 0.6f; T.SkyColor = FLinearColor(0.3f, 0.25f, 0.45f); T.GroundColor = FLinearColor(0.12f, 0.1f, 0.18f);
		T.Props = {
			MakeProp(Cube, FVector(-680, 240, 90),  FVector(1.0f, 1.0f, 2.4f), FLinearColor(0.18f, 0.14f, 0.28f), FRotator(0, 20, 8)),
			MakeProp(Cube, FVector(780, -220, 70),  FVector(1.2f, 1.2f, 1.8f), FLinearColor(0.16f, 0.12f, 0.26f), FRotator(0, -15, -6)),
			MakeProp(Cube, FVector(120, 560, 120),  FVector(0.8f, 0.8f, 3.0f), FLinearColor(0.2f, 0.15f, 0.3f), FRotator(0, 40, 10)),
			MakeProp(Cube, FVector(-300, 480, 60),  FVector(1.0f, 1.0f, 1.4f), FLinearColor(0.15f, 0.11f, 0.24f), FRotator(0, -30, 5)),
		};
		break;
	case 4: // 폭풍/번개 — 차가운 청회, 바위 첨탑
		T.SunColor = FLinearColor(0.7f, 0.75f, 0.95f); T.SunIntensity = 4.0f; T.SunRotation = FRotator(-40, 50, 0);
		T.SkyIntensity = 0.8f; T.SkyColor = FLinearColor(0.4f, 0.45f, 0.6f); T.GroundColor = FLinearColor(0.18f, 0.18f, 0.22f);
		T.Props = {
			MakeProp(Cone, FVector(-720, 260, 140), FVector(1.0f, 1.0f, 3.0f), FLinearColor(0.22f, 0.24f, 0.3f)),
			MakeProp(Cone, FVector(800, -240, 120), FVector(1.0f, 1.0f, 2.6f), FLinearColor(0.2f, 0.22f, 0.28f)),
			MakeProp(Cone, FVector(0, 580, 160),    FVector(1.2f, 1.2f, 3.4f), FLinearColor(0.24f, 0.26f, 0.32f)),
			MakeProp(Cube, FVector(-250, 420, 50),  FVector(1.4f, 1.4f, 1.0f), FLinearColor(0.2f, 0.2f, 0.24f), FRotator(0, 18, 0)),
		};
		break;
	case 5: // 심연 옥좌 — 짙은 청흑, 높은 기둥
		T.SunColor = FLinearColor(0.3f, 0.4f, 0.65f); T.SunIntensity = 2.5f; T.SunRotation = FRotator(-30, 40, 0);
		T.SkyIntensity = 0.5f; T.SkyColor = FLinearColor(0.15f, 0.2f, 0.4f); T.GroundColor = FLinearColor(0.08f, 0.1f, 0.16f);
		T.Props = {
			MakeProp(Cube, FVector(-700, 280, 140), FVector(0.9f, 0.9f, 4.0f), FLinearColor(0.12f, 0.14f, 0.22f)),
			MakeProp(Cube, FVector(700, -280, 140), FVector(0.9f, 0.9f, 4.0f), FLinearColor(0.12f, 0.14f, 0.22f)),
			MakeProp(Cube, FVector(-100, 620, 170), FVector(1.0f, 1.0f, 4.8f), FLinearColor(0.14f, 0.16f, 0.26f)),
			MakeProp(Cube, FVector(360, 520, 120),  FVector(0.8f, 0.8f, 3.4f), FLinearColor(0.1f, 0.12f, 0.2f)),
		};
		break;
	case 6: // 무너지는 근원 — 회보라, 부서진 큐브
		T.SunColor = FLinearColor(0.6f, 0.5f, 0.7f); T.SunIntensity = 3.0f; T.SunRotation = FRotator(-55, 20, 0);
		T.SkyIntensity = 0.55f; T.SkyColor = FLinearColor(0.35f, 0.3f, 0.4f); T.GroundColor = FLinearColor(0.16f, 0.13f, 0.16f);
		T.Props = {
			MakeProp(Cube, FVector(-680, 260, 50),  FVector(1.4f, 1.4f, 1.2f), FLinearColor(0.2f, 0.17f, 0.22f), FRotator(0, 25, 28)),
			MakeProp(Cube, FVector(760, -240, 40),  FVector(1.6f, 1.6f, 0.9f), FLinearColor(0.18f, 0.15f, 0.2f), FRotator(0, -20, 35)),
			MakeProp(Cube, FVector(80, 560, 90),    FVector(1.0f, 1.0f, 2.2f), FLinearColor(0.22f, 0.18f, 0.24f), FRotator(0, 40, 15)),
			MakeProp(Cube, FVector(-320, 460, 30),  FVector(1.2f, 1.2f, 0.7f), FLinearColor(0.17f, 0.14f, 0.19f), FRotator(0, -35, 42)),
		};
		break;
	case 7: // 균열 — 불길 적색, 떠 있는 뾰족 파편
		T.SunColor = FLinearColor(1.0f, 0.45f, 0.3f); T.SunIntensity = 5.0f; T.SunRotation = FRotator(-40, 70, 0);
		T.SkyIntensity = 0.9f; T.SkyColor = FLinearColor(0.6f, 0.25f, 0.2f); T.GroundColor = FLinearColor(0.2f, 0.1f, 0.1f);
		T.Props = {
			MakeProp(Cube, FVector(-650, 280, 320), FVector(0.4f, 0.4f, 2.2f), FLinearColor(0.5f, 0.18f, 0.12f), FRotator(35, 20, 18)),
			MakeProp(Cube, FVector(720, -220, 360), FVector(0.35f, 0.35f, 2.6f), FLinearColor(0.55f, 0.2f, 0.12f), FRotator(-30, -15, 22)),
			MakeProp(Cube, FVector(60, 600, 420),   FVector(0.45f, 0.45f, 3.0f), FLinearColor(0.6f, 0.22f, 0.14f), FRotator(45, 40, 10)),
			MakeProp(Cube, FVector(-280, 460, 280), FVector(0.3f, 0.3f, 1.8f), FLinearColor(0.5f, 0.18f, 0.1f), FRotator(-25, -30, 30)),
		};
		break;
	case 8: // 후존계 — 차가운 청백 정적, 부서진 기둥
	default:
		T.SunColor = FLinearColor(0.6f, 0.8f, 0.95f); T.SunIntensity = 4.0f; T.SunRotation = FRotator(-60, 35, 0);
		T.SkyIntensity = 1.0f; T.SkyColor = FLinearColor(0.5f, 0.65f, 0.8f); T.GroundColor = FLinearColor(0.18f, 0.22f, 0.26f);
		T.Props = {
			MakeProp(Cube, FVector(-700, 280, 150), FVector(0.9f, 0.9f, 4.2f), FLinearColor(0.3f, 0.36f, 0.42f)),
			MakeProp(Cube, FVector(720, -240, 40),  FVector(1.0f, 1.0f, 0.8f), FLinearColor(0.28f, 0.34f, 0.4f), FRotator(0, 15, 38)),
			MakeProp(Cube, FVector(80, 600, 170),   FVector(0.9f, 0.9f, 4.8f), FLinearColor(0.32f, 0.38f, 0.44f)),
			MakeProp(Cube, FVector(-320, 470, 130), FVector(0.8f, 0.8f, 3.6f), FLinearColor(0.3f, 0.36f, 0.42f)),
		};
		break;
	}
	return T;
}
```

- [ ] **Step 4: 라이브러리 회귀 테스트**

`MapThemeTests.cpp`:

```cpp
#include "Misc/AutomationTest.h"
#include "GameCore/MapThemeLibrary.h"

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

#endif
```

- [ ] **Step 5: 빌드 + 라이브러리 Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.MapTheme"`
Expected: 빌드 성공 + `IdleProject.MapTheme.Library` PASS

- [ ] **Step 6: 커밋**

```bash
git add client/Source/IdleProject/GameCore/MapThemeTypes.h client/Source/IdleProject/GameCore/MapThemeLibrary.h client/Source/IdleProject/GameCore/MapThemeLibrary.cpp client/Source/IdleProject/Tests/MapThemeTests.cpp
git commit -m "feat(client): 맵 테마 타입 + 8 챕터 테마 라이브러리 + 회귀"
```

---

## Task 2: GameMode 적용 + 챕터 전환 바인딩

**Files:** Modify `client/Source/IdleProject/IdleProjectGameModeBase.h`, `.cpp`; Test `MapThemeTests.cpp`(적용 케이스 추가).

- [ ] **Step 1: 헤더 — 참조 멤버 + 메서드**

`IdleProjectGameModeBase.h` 상단 include 에 `#include "GameCore/MapThemeTypes.h"` 추가. 클래스에 추가:

```cpp
public:
	// 챕터 테마 적용(조명/바닥색/프롭 교체). 헤드리스 테스트 진입점.
	void ApplyMapTheme(int32 Chapter);

private:
	UFUNCTION()
	void HandleStageChangedForTheme(FStageInfo NewStageInfo);

	UPROPERTY()
	TObjectPtr<class ADirectionalLight> ThemeSun = nullptr;
	UPROPERTY()
	TObjectPtr<class ASkyLight> ThemeSky = nullptr;
	UPROPERTY()
	TObjectPtr<class AStaticMeshActor> ThemeGround = nullptr;
	UPROPERTY()
	TArray<TObjectPtr<class AStaticMeshActor>> ThemeProps;

	int32 AppliedThemeChapter = -1;
```

> 주: `HandleStageChangedForTheme` 는 `FOnStageChanged`(OneParam FStageInfo)에 바인딩하므로 시그니처가 `(FStageInfo)` 여야 한다. UFUNCTION 필수(DYNAMIC delegate).

- [ ] **Step 2: SpawnDefaultEnvironment 에서 참조 보관**

`SpawnDefaultEnvironment()`의 Sun/Sky/Ground spawn 블록에서 로컬 변수 대신 멤버에 저장:
- `ADirectionalLight* Sun = ...` → `ThemeSun = World->SpawnActor<ADirectionalLight>(...)` 후 `if (ThemeSun)` 으로 컴포넌트 설정.
- `ASkyLight* Sky = ...` → `ThemeSky = ...`.
- `AStaticMeshActor* Ground = ...` → `ThemeGround = ...`.
(기존 설정 로직 유지, 참조만 멤버화.)

- [ ] **Step 3: ApplyMapTheme 구현**

`.cpp`에 추가(상단 include: `Engine/DirectionalLight.h`, `Engine/SkyLight.h`, `Components/DirectionalLightComponent.h`, `Components/SkyLightComponent.h`, `Engine/StaticMeshActor.h`, `Components/StaticMeshComponent.h`, `Materials/MaterialInstanceDynamic.h` — 일부 기존 포함):

```cpp
void AIdleProjectGameModeBase::ApplyMapTheme(int32 Chapter)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FMapTheme Theme = FMapThemeLibrary::GetTheme(Chapter);

	// 조명(주 무드).
	if (ThemeSun)
	{
		if (ULightComponent* SunComp = ThemeSun->GetLightComponent())
		{
			SunComp->SetUseTemperature(false);
			SunComp->SetLightColor(Theme.SunColor);
			SunComp->SetIntensity(Theme.SunIntensity);
		}
		ThemeSun->SetActorRotation(Theme.SunRotation);
	}
	if (ThemeSky)
	{
		if (USkyLightComponent* SkyComp = ThemeSky->GetLightComponent())
		{
			SkyComp->SetIntensity(Theme.SkyIntensity);
			SkyComp->SetLightColor(Theme.SkyColor);
		}
	}
	// 바닥 색(best-effort MID 틴트 — 파라미터 머티리얼 없으면 시각 no-op).
	if (ThemeGround)
	{
		if (UStaticMeshComponent* GroundMesh = ThemeGround->GetStaticMeshComponent())
		{
			if (UMaterialInterface* Base = GroundMesh->GetMaterial(0))
			{
				UMaterialInstanceDynamic* MID = GroundMesh->CreateAndSetMaterialInstanceDynamic(0);
				if (MID)
				{
					MID->SetVectorParameterValue(TEXT("Color"), Theme.GroundColor);
				}
			}
		}
	}

	// 프롭 교체: 기존 제거 → 신규 spawn.
	for (AStaticMeshActor* Old : ThemeProps)
	{
		if (Old)
		{
			Old->Destroy();
		}
	}
	ThemeProps.Reset();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (const FMapProp& Prop : Theme.Props)
	{
		UStaticMesh* Mesh = Prop.Mesh.LoadSynchronous();
		if (!Mesh)
		{
			continue; // 로드 실패 skip(크래시 가드)
		}
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Prop.Location, Prop.Rotation, Params);
		if (!Actor)
		{
			continue;
		}
		Actor->SetMobility(EComponentMobility::Movable);
		if (UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent())
		{
			Comp->SetStaticMesh(Mesh);
			Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 게임플레이 무간섭
			if (UMaterialInterface* Base = Comp->GetMaterial(0))
			{
				UMaterialInstanceDynamic* MID = Comp->CreateAndSetMaterialInstanceDynamic(0);
				if (MID)
				{
					MID->SetVectorParameterValue(TEXT("Color"), Prop.Color);
				}
			}
		}
		Actor->SetActorScale3D(Prop.Scale);
		ThemeProps.Add(Actor);
	}

	AppliedThemeChapter = FMath::Clamp(Chapter, 1, FMapThemeLibrary::ThemeCount);
}

void AIdleProjectGameModeBase::HandleStageChangedForTheme(FStageInfo NewStageInfo)
{
	if (NewStageInfo.Chapter != AppliedThemeChapter)
	{
		ApplyMapTheme(NewStageInfo.Chapter);
	}
}
```

- [ ] **Step 4: 초기 적용 + OnStageChanged 바인딩**

`SpawnDefaultEnvironment()` 호출 직후(또는 PostLogin 에서 StageService 확보 시점)에:

```cpp
	if (UIdleGameInstance* GameInstance = GetGameInstance<UIdleGameInstance>())
	{
		if (UStageService* StageService = GameInstance->GetStageService())
		{
			StageService->OnStageChanged.AddDynamic(this, &AIdleProjectGameModeBase::HandleStageChangedForTheme);
			ApplyMapTheme(StageService->GetCurrentChapter());
		}
	}
```

> 주: 바인딩 위치는 `SpawnDefaultEnvironment` 가 Sun/Sky/Ground 를 이미 만든 뒤여야 한다(ApplyMapTheme 가 참조 사용). 환경 spawn 함수 끝에 넣거나, 환경 spawn 후 호출되는 init 지점에 둔다. StageService 가 아직 없으면(테스트) 바인딩 skip — ApplyMapTheme 는 직접 호출로 검증.

- [ ] **Step 5: 적용 회귀 테스트 추가**

`MapThemeTests.cpp` 의 `#endif` 앞에 추가:

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapThemeApplyTest,
	"IdleProject.MapTheme.Apply",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMapThemeApplyTest::RunTest(const FString& Parameters)
{
	UWorld* World = GetWorld();
	if (!World) { AddError(TEXT("No world")); return false; }

	AIdleProjectGameModeBase* GM = World->SpawnActor<AIdleProjectGameModeBase>();
	if (!GM) { AddError(TEXT("No GM")); return false; }
	GM->SpawnDefaultEnvironmentForTest(); // 주: 환경 spawn 공개 진입점(없으면 SpawnDefaultEnvironment 를 public 화하거나 테스트 헬퍼 추가)

	// ch1 적용 → 프롭 수 = 테마 프롭 수.
	GM->ApplyMapTheme(1);
	const int32 Ch1Props = FMapThemeLibrary::GetTheme(1).Props.Num();
	TestEqual(TEXT("ch1 prop count"), GM->GetThemePropCountForTest(), Ch1Props);

	// ch7 전환 → 프롭 교체(ch7 수).
	GM->ApplyMapTheme(7);
	const int32 Ch7Props = FMapThemeLibrary::GetTheme(7).Props.Num();
	TestEqual(TEXT("ch7 prop count after switch"), GM->GetThemePropCountForTest(), Ch7Props);

	return true;
}
```

> 주: 테스트 진입점 `SpawnDefaultEnvironmentForTest()`/`GetThemePropCountForTest()`는 헤드리스 검증용 thin 게터. `SpawnDefaultEnvironment` 가 private 이면 test-only public 래퍼를 추가하거나, 기존 `IMPLEMENT_GET_PRIVATE` 패턴(코드베이스에 있으면)을 따른다. 프롭 수는 `ThemeProps.Num()` 노출. 조명 적용 단언(Sun 색/강도)도 가능하면 추가(`ThemeSun->GetLightComponent()->ComputeLightBrightness()` 등) — 어려우면 프롭 교체 검증으로 충분.

- [ ] **Step 6: 빌드 + MapTheme Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.MapTheme"`
Expected: 빌드 성공 + Library/Apply PASS

- [ ] **Step 7: 커밋**

```bash
git add client/Source/IdleProject/IdleProjectGameModeBase.h client/Source/IdleProject/IdleProjectGameModeBase.cpp client/Source/IdleProject/Tests/MapThemeTests.cpp
git commit -m "feat(client): GameMode 맵 테마 적용 + 챕터 전환 바인딩 + 회귀"
```

---

## Task 3: 최종 게이트

- [ ] **Step 1: UE 표준 jumbo + 전체 Automation**

Run: `./tools/ci/ue-automation.ps1`
Expected: 빌드 성공(ODR 0) + 전체 Automation GREEN(MapTheme 포함, 기존 회귀 무손상). **세이브·서버 무변경 확인**.

- [ ] **Step 2: 세이브/서버 무변경 확인**

Run: `grep -rn "SaveVersion = 2" client/Source/IdleProject/GameCore/IdleSaveGame.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp`
Expected: 둘 다 29(변경 없음). server/ 디렉터리 미변경.

- [ ] **Step 3: 서버 무관 확인**

Run: `git diff --name-only main | grep "^server/" || echo "서버 무변경"`
Expected: "서버 무변경"

---

## Self-Review (작성자 점검)

**스펙 커버리지:**
- §3 데이터(FMapProp soft 메시/FMapTheme) → Task 1 ✅
- §4 적용(참조 보관/ApplyMapTheme/OnStageChanged 바인딩/초기 적용) → Task 2 ✅
- §5 색상(조명 주 무드 + best-effort MID 틴트) → Task 2 ✅
- §6 8 팔레트(스토리 매칭) → Task 1 Step 3 ✅
- §7 테스트(라이브러리+적용+전환, 헤드리스) → Task 1,2 ✅
- §8 가드(soft 로드 실패 skip/NoCollision/클램프/멱등) → Task 1,2 ✅

**비목표:** 임포트 에셋/서버/세이브/안개/지형 → 미포함(의도적) ✅

**플레이스홀더:** "주:" 는 기존 코드 확인(SpawnDefaultEnvironment private 여부, test 게터, 바인딩 위치). 실코드 제공. TODO/TBD 없음.

**타입 일관성:** `FMapProp`/`FMapTheme`/`FMapThemeLibrary::GetTheme`/`ApplyMapTheme`/`HandleStageChangedForTheme`/`ThemeSun`/`ThemeSky`/`ThemeGround`/`ThemeProps`/`AppliedThemeChapter` 정합. OnStageChanged 시그니처(FStageInfo) 일치.

**무변경:** 세이브(SaveVer 29)/서버/parity 0. 시각 전용. 프롭 NoCollision(게임플레이 0영향).
