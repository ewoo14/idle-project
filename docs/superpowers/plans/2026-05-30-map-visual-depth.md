# 맵 시각 심화 구현 플랜 (안개 + 프롭 질감 + 절차 스카이)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 챕터별 안개(ExponentialHeightFog), 프롭 질감 변형(M_MapTheme 스칼라 파라미터), 절차 스카이(TC_MapSky 큐브맵 + M_Sky 스피어)로 맵 시각 깊이를 완성한다.

**Architecture:** `FMapProp`/`FMapTheme` 필드 확장 + `MapThemeLibrary` 8 팔레트 값. `M_MapTheme` 커맨드릿에 Metallic/Roughness/EmissiveStrength 추가(재생성). 신규 `GenerateMapSkyAssets` 커맨드릿이 TC_MapSky(UTextureCube)+M_Sky 생성. GameMode가 Fog/SkySphere actor spawn + `ApplyMapTheme` 확장(에셋 무효 시 요소별 폴백). 전부 클라 전용 시각.

**Tech Stack:** UE5.7 C++ (IdleProject), 에디터 모듈(#105 재사용), Automation, Git LFS(.uasset).

---

## 파일 구조

- `client/Source/IdleProject/GameCore/MapThemeTypes.h` — 수정: FMapProp 질감 3 + FMapTheme 안개/스카이 3 필드.
- `client/Source/IdleProject/GameCore/MapThemeLibrary.cpp` — 수정: MakeProp 확장 + 8 팔레트 안개/스카이/질감 값.
- `client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.cpp` — 수정: 스칼라 파라미터 3종.
- `client/Source/IdleProject/Commandlets/GenerateMapSkyAssetsCommandlet.h/.cpp` — 신규: 스카이 에셋 생성.
- `client/Content/Maps/M_MapTheme.uasset` — 재생성(LFS).
- `client/Content/Maps/TC_MapSky.uasset`, `M_Sky.uasset` — 신규(LFS).
- `client/Source/IdleProject/IdleProjectGameModeBase.h/.cpp` — 수정: Fog/SkySphere 멤버 + spawn + ApplyMapTheme 확장.
- `client/Source/IdleProject/Tests/MapThemeTests.cpp` — 수정: 안개/질감/스카이 회귀.

---

### Task 1: 데이터 모델 + 팔레트 확장

**Files:**
- Modify: `client/Source/IdleProject/GameCore/MapThemeTypes.h`
- Modify: `client/Source/IdleProject/GameCore/MapThemeLibrary.cpp`

- [ ] **Step 1: FMapProp 질감 필드 추가**

`MapThemeTypes.h`의 `FMapProp` 내 `Color` 필드 다음에 추가:
```cpp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	float Metallic = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	float Roughness = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	float EmissiveStrength = 0.15f;
```

- [ ] **Step 2: FMapTheme 안개/스카이 필드 추가**

`FMapTheme` 내 `Props` 필드 **앞**에 추가:
```cpp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FLinearColor FogColor = FLinearColor(0.5f, 0.55f, 0.6f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	float FogDensity = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FLinearColor SkyTint = FLinearColor::White;
```

- [ ] **Step 3: MakeProp 헬퍼에 질감 파라미터 추가**

`MapThemeLibrary.cpp`의 `MakeProp`(5-14행)를 교체:
```cpp
	FMapProp MakeProp(const TCHAR* MeshPath, const FVector& Loc, const FVector& Scale, const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator,
		float Metallic = 0.0f, float Roughness = 0.85f, float Emissive = 0.15f)
	{
		FMapProp P;
		P.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(MeshPath));
		P.Location = Loc;
		P.Scale = Scale;
		P.Color = Color;
		P.Rotation = Rot;
		P.Metallic = Metallic;
		P.Roughness = Roughness;
		P.EmissiveStrength = Emissive;
		return P;
	}
```

- [ ] **Step 4: 8 팔레트에 안개/스카이 값 추가**

각 `case`의 `T.GroundColor = ...;` 라인 다음(같은 줄 끝)에 안개/스카이 3값을 추가한다. 아래 표 그대로:

```
ch1: T.FogColor = FLinearColor(0.70f,0.82f,0.95f); T.FogDensity = 0.012f; T.SkyTint = FLinearColor(0.70f,0.85f,1.0f);
ch2: T.FogColor = FLinearColor(0.45f,0.60f,0.50f); T.FogDensity = 0.020f; T.SkyTint = FLinearColor(0.55f,0.70f,0.60f);
ch3: T.FogColor = FLinearColor(0.30f,0.25f,0.42f); T.FogDensity = 0.030f; T.SkyTint = FLinearColor(0.30f,0.25f,0.45f);
ch4: T.FogColor = FLinearColor(0.40f,0.45f,0.58f); T.FogDensity = 0.025f; T.SkyTint = FLinearColor(0.40f,0.45f,0.60f);
ch5: T.FogColor = FLinearColor(0.12f,0.16f,0.32f); T.FogDensity = 0.040f; T.SkyTint = FLinearColor(0.15f,0.20f,0.40f);
ch6: T.FogColor = FLinearColor(0.32f,0.27f,0.36f); T.FogDensity = 0.035f; T.SkyTint = FLinearColor(0.35f,0.30f,0.40f);
ch7: T.FogColor = FLinearColor(0.55f,0.20f,0.15f); T.FogDensity = 0.030f; T.SkyTint = FLinearColor(0.60f,0.25f,0.20f);
ch8(default): T.FogColor = FLinearColor(0.50f,0.62f,0.75f); T.FogDensity = 0.020f; T.SkyTint = FLinearColor(0.50f,0.65f,0.80f);
```

- [ ] **Step 5: 챕터별 대표 프롭 질감 부여(선택 프롭만)**

다음 MakeProp 호출에 trailing 질감 인자를 추가(나머지 프롭은 기본값 유지):
- ch5(70-78행) 4개 큐브 기둥: 각 호출 끝에 `, 0.6f, 0.4f, 0.1f`(금속 광택 기둥).
- ch7(93-98행) 4개 큐브 파편: 각 호출 끝에 `, 0.0f, 0.25f, 0.7f`(발광 수정 파편).
- ch3(53-58행) 4개 큐브: 각 호출 끝에 `, 0.0f, 0.5f, 0.2f`(흑요석 광택).

예(ch7 첫 프롭):
```cpp
				MakeProp(Cube, FVector(-650, 280, 320), FVector(0.4f, 0.4f, 2.2f), FLinearColor(0.5f, 0.18f, 0.12f), FRotator(35, 20, 18), 0.0f, 0.25f, 0.7f),
```
(MakeProp는 Rot 다음 Metallic/Roughness/Emissive 순. ch5/ch3는 Rot 인자가 없으면 기본 `FRotator::ZeroRotator`를 명시해야 trailing 인자를 줄 수 있다 — ch5는 Rot 없으니 `..., FRotator::ZeroRotator, 0.6f, 0.4f, 0.1f` 형태.)

- [ ] **Step 6: 에디터 빌드 확인**

Run:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
```
Expected: 빌드 성공.

- [ ] **Step 7: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/GameCore/MapThemeTypes.h client/Source/IdleProject/GameCore/MapThemeLibrary.cpp
git commit -m @'
맵 시각 심화(1) — 안개/스카이/프롭 질감 데이터 모델 + 8 팔레트

FMapProp Metallic/Roughness/EmissiveStrength, FMapTheme FogColor/FogDensity/
SkyTint 추가. 8 챕터 안개·스카이 값 + ch3/5/7 대표 프롭 질감(흑요석/금속/발광).

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 2: M_MapTheme 커맨드릿에 스칼라 파라미터 추가 + 재생성

**Files:**
- Modify: `client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.cpp`
- Modify: `client/Content/Maps/M_MapTheme.uasset` (재생성)

- [ ] **Step 1: 커맨드릿에 스칼라 파라미터 3종 추가**

`GenerateMapThemeMaterialCommandlet.cpp`의 include 블록에 추가:
```cpp
#include "Materials/MaterialExpressionScalarParameter.h"
```

`Main`의 Roughness 상수 부분과 Emissive 곱 부분을 스칼라 파라미터로 교체한다. 기존:
- `EmiScale`(Constant 0.15) → `EmissiveStrength`(ScalarParameter, 기본 0.15)로 변경.
- `Rough`(Constant 0.85) → `Roughness`(ScalarParameter, 기본 0.85)로 변경.
- 신규 `Metallic`(ScalarParameter, 기본 0.0) → Metallic 연결 추가.

교체 코드(기존 EmiScale/Mul/Rough 블록 대체):
```cpp
	// Color * EmissiveStrength(스칼라 파라미터) → Emissive
	UMaterialExpressionScalarParameter* EmiParam = Cast<UMaterialExpressionScalarParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionScalarParameter::StaticClass(), -500, 250));
	EmiParam->ParameterName = TEXT("EmissiveStrength");
	EmiParam->DefaultValue = 0.15f;
	UMaterialExpressionMultiply* Mul = Cast<UMaterialExpressionMultiply>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionMultiply::StaticClass(), -250, 150));
	UMaterialEditingLibrary::ConnectMaterialExpressions(ColorParam, TEXT(""), Mul, TEXT("A"));
	UMaterialEditingLibrary::ConnectMaterialExpressions(EmiParam, TEXT(""), Mul, TEXT("B"));
	UMaterialEditingLibrary::ConnectMaterialProperty(Mul, TEXT(""), MP_EmissiveColor);

	// Roughness(스칼라 파라미터)
	UMaterialExpressionScalarParameter* RoughParam = Cast<UMaterialExpressionScalarParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionScalarParameter::StaticClass(), -500, 450));
	RoughParam->ParameterName = TEXT("Roughness");
	RoughParam->DefaultValue = 0.85f;
	UMaterialEditingLibrary::ConnectMaterialProperty(RoughParam, TEXT(""), MP_Roughness);

	// Metallic(스칼라 파라미터)
	UMaterialExpressionScalarParameter* MetalParam = Cast<UMaterialExpressionScalarParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionScalarParameter::StaticClass(), -500, 600));
	MetalParam->ParameterName = TEXT("Metallic");
	MetalParam->DefaultValue = 0.0f;
	UMaterialEditingLibrary::ConnectMaterialProperty(MetalParam, TEXT(""), MP_Metallic);
```
(기존 `ColorParam → MP_BaseColor` 연결은 유지.)

- [ ] **Step 2: 에디터 빌드 + 커맨드릿 재실행**

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\game\idle game\repo\client\IdleProject.uproject" -run=GenerateMapThemeMaterial -unattended -nopause -nosplash -stdout
```
Expected: 빌드 성공 + 로그 `[GenMapThemeMat] 저장 완료`.

- [ ] **Step 3: 커밋(코드 + 재생성 에셋)**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.cpp client/Content/Maps/M_MapTheme.uasset
git commit -m @'
맵 시각 심화(2) — M_MapTheme에 Metallic/Roughness/EmissiveStrength 스칼라 파라미터

프롭별 질감 제어용 스칼라 파라미터 3종 추가 후 에셋 재생성(LFS). Color BaseColor
연결 유지(하위호환).

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 3: 스카이 에셋 커맨드릿(TC_MapSky + M_Sky) + 생성

**Files:**
- Create: `client/Source/IdleProject/Commandlets/GenerateMapSkyAssetsCommandlet.h`
- Create: `client/Source/IdleProject/Commandlets/GenerateMapSkyAssetsCommandlet.cpp`
- Create: `client/Content/Maps/TC_MapSky.uasset`, `M_Sky.uasset` (LFS)

- [ ] **Step 1: 커맨드릿 헤더**

`Commandlets/GenerateMapSkyAssetsCommandlet.h`:
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "GenerateMapSkyAssetsCommandlet.generated.h"

/**
 * 절차 스카이 에셋 생성(에디터 전용): 그라데이션 TextureCube(/Game/Maps/TC_MapSky) +
 * Unlit 스카이 머티리얼(/Game/Maps/M_Sky, SkyCube 큐브 샘플 × SkyTint 벡터 파라미터).
 * 실행: UnrealEditor-Cmd.exe "<uproject>" -run=GenerateMapSkyAssets -unattended -nopause -nosplash
 */
UCLASS()
class UGenerateMapSkyAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};
```

- [ ] **Step 2: 커맨드릿 본문 — TC_MapSky(그라데이션 큐브맵)**

`Commandlets/GenerateMapSkyAssetsCommandlet.cpp` 작성. UTextureCube 6면을 천정(+Z 짙은 청)→수평(밝은 청백)→바닥(-Z 어두움) 그라데이션으로 채운다. UE5.7 `UTextureCube`/`FTextureSource` API는 엔진 헤더(`Engine/TextureCube.h`, `TextureResource.h`)로 시그니처 확인 후 조정:

```cpp
#include "GenerateMapSkyAssetsCommandlet.h"

#if WITH_EDITOR
#include "Factories/MaterialFactoryNew.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSampleParameterCube.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "MaterialEditingLibrary.h"
#include "Engine/TextureCube.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "SceneTypes.h"

namespace
{
	// 면 방향별 대표 상단/하단 색 보간으로 8비트 RGBA 큐브 1면(Size×Size) 채움.
	// 단순 수직 그라데이션(천정 짙은 청 → 바닥 어두움) 중립 톤. 런타임 SkyTint로 챕터색.
	void FillFace(TArray<FColor>& Out, int32 Size)
	{
		Out.SetNumUninitialized(Size * Size);
		const FLinearColor Top(0.15f, 0.30f, 0.55f);
		const FLinearColor Bottom(0.55f, 0.65f, 0.80f);
		for (int32 Y = 0; Y < Size; ++Y)
		{
			const float A = (float)Y / (float)(Size - 1);
			const FLinearColor L = FMath::Lerp(Top, Bottom, A);
			const FColor C = L.ToFColor(false);
			for (int32 X = 0; X < Size; ++X)
			{
				Out[Y * Size + X] = C;
			}
		}
	}
}
#endif

int32 UGenerateMapSkyAssetsCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	const int32 Size = 64;

	// 1) TC_MapSky
	const FString CubeName = TEXT("/Game/Maps/TC_MapSky");
	UPackage* CubePkg = CreatePackage(*CubeName);
	CubePkg->FullyLoad();
	UTextureCube* Cube = NewObject<UTextureCube>(CubePkg, TEXT("TC_MapSky"), RF_Standalone | RF_Public);

	TArray<FColor> Face;
	FillFace(Face, Size);
	TArray<FColor> SixFaces;
	SixFaces.Reserve(Size * Size * 6);
	for (int32 F = 0; F < 6; ++F)
	{
		SixFaces.Append(Face);
	}
	Cube->Source.Init(Size, Size, 6, 1, TSF_BGRA8, (const uint8*)SixFaces.GetData());
	Cube->UpdateResource();
	Cube->PostEditChange();
	FAssetRegistryModule::AssetCreated(Cube);
	{
		const FString File = FPackageName::LongPackageNameToFilename(CubeName, FPackageName::GetAssetPackageExtension());
		FSavePackageArgs Args; Args.TopLevelFlags = RF_Standalone | RF_Public;
		if (!UPackage::SavePackage(CubePkg, Cube, *File, Args))
		{
			UE_LOG(LogTemp, Error, TEXT("[GenMapSky] TC_MapSky 저장 실패")); return 1;
		}
	}

	// 2) M_Sky(Unlit, TwoSided): SkyCube 샘플 × SkyTint → Emissive
	const FString MatName = TEXT("/Game/Maps/M_Sky");
	UPackage* MatPkg = CreatePackage(*MatName);
	MatPkg->FullyLoad();
	UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
	UMaterial* Sky = Cast<UMaterial>(Factory->FactoryCreateNew(UMaterial::StaticClass(), MatPkg, TEXT("M_Sky"), RF_Standalone | RF_Public, nullptr, GWarn));
	Sky->MaterialDomain = MD_Surface;
	Sky->SetShadingModel(MSM_Unlit);
	Sky->TwoSided = true;

	UMaterialExpressionTextureSampleParameterCube* CubeSample = Cast<UMaterialExpressionTextureSampleParameterCube>(
		UMaterialEditingLibrary::CreateMaterialExpression(Sky, UMaterialExpressionTextureSampleParameterCube::StaticClass(), -500, 0));
	CubeSample->ParameterName = TEXT("SkyCube");
	CubeSample->Texture = Cube;

	UMaterialExpressionVectorParameter* Tint = Cast<UMaterialExpressionVectorParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Sky, UMaterialExpressionVectorParameter::StaticClass(), -500, 250));
	Tint->ParameterName = TEXT("SkyTint");
	Tint->DefaultValue = FLinearColor::White;

	UMaterialExpressionMultiply* Mul = Cast<UMaterialExpressionMultiply>(
		UMaterialEditingLibrary::CreateMaterialExpression(Sky, UMaterialExpressionMultiply::StaticClass(), -250, 100));
	UMaterialEditingLibrary::ConnectMaterialExpressions(CubeSample, TEXT(""), Mul, TEXT("A"));
	UMaterialEditingLibrary::ConnectMaterialExpressions(Tint, TEXT(""), Mul, TEXT("B"));
	UMaterialEditingLibrary::ConnectMaterialProperty(Mul, TEXT(""), MP_EmissiveColor);

	UMaterialEditingLibrary::RecompileMaterial(Sky);
	Sky->PostEditChange();
	FAssetRegistryModule::AssetCreated(Sky);
	{
		const FString File = FPackageName::LongPackageNameToFilename(MatName, FPackageName::GetAssetPackageExtension());
		FSavePackageArgs Args; Args.TopLevelFlags = RF_Standalone | RF_Public;
		if (!UPackage::SavePackage(MatPkg, Sky, *File, Args))
		{
			UE_LOG(LogTemp, Error, TEXT("[GenMapSky] M_Sky 저장 실패")); return 1;
		}
	}
	UE_LOG(LogTemp, Display, TEXT("[GenMapSky] TC_MapSky + M_Sky 저장 완료"));
	return 0;
#else
	UE_LOG(LogTemp, Error, TEXT("[GenMapSky] 에디터 빌드 전용"));
	return 1;
#endif
}
```
> API 주의: `UTextureCube::Source.Init`/`SetShadingModel`/`MaterialDomain`/`TwoSided`/`TextureSampleParameterCube` 시그니처가 UE5.7과 다르면 엔진 헤더 확인 후 조정. `TSF_BGRA8`로 FColor(BGRA) 정합.

- [ ] **Step 3: 에디터 빌드 + 스카이 커맨드릿 실행**

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\game\idle game\repo\client\IdleProject.uproject" -run=GenerateMapSkyAssets -unattended -nopause -nosplash -stdout
```
Expected: 빌드 성공 + `[GenMapSky] TC_MapSky + M_Sky 저장 완료`. `client/Content/Maps/TC_MapSky.uasset`, `M_Sky.uasset` 생성.

- [ ] **Step 4: LFS 추적 확인 + 커밋**

```powershell
cd "C:\game\idle game\repo"
git check-attr filter -- client/Content/Maps/TC_MapSky.uasset client/Content/Maps/M_Sky.uasset
git add client/Source/IdleProject/Commandlets/GenerateMapSkyAssetsCommandlet.h client/Source/IdleProject/Commandlets/GenerateMapSkyAssetsCommandlet.cpp client/Content/Maps/TC_MapSky.uasset client/Content/Maps/M_Sky.uasset
git commit -m @'
맵 시각 심화(3) — 절차 스카이 에셋(TC_MapSky 큐브맵 + M_Sky 스피어 머티리얼)

GenerateMapSkyAssets 커맨드릿이 그라데이션 UTextureCube + Unlit TwoSided 스카이
머티리얼(SkyCube×SkyTint→Emissive) 생성. 에셋 LFS. 챕터색은 런타임 SkyTint MID.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
git lfs ls-files | Select-String "TC_MapSky|M_Sky"
```
Expected: 두 에셋 `filter: lfs`, `git lfs ls-files`에 등장.

---

### Task 4: GameMode — Fog/SkySphere spawn + ApplyMapTheme 확장

**Files:**
- Modify: `client/Source/IdleProject/IdleProjectGameModeBase.h`
- Modify: `client/Source/IdleProject/IdleProjectGameModeBase.cpp`

- [ ] **Step 1: 헤더 멤버 + 테스트 접근자**

`IdleProjectGameModeBase.h`의 `TObjectPtr<class UMaterialInterface> ThemeMaterial`(#105 추가) 인근에 추가:
```cpp
	UPROPERTY()
	TObjectPtr<class AExponentialHeightFog> ThemeFog = nullptr;

	UPROPERTY()
	TObjectPtr<class AStaticMeshActor> ThemeSkySphere = nullptr;

	UPROPERTY()
	TObjectPtr<class UMaterialInterface> SkyMaterial = nullptr;

	UPROPERTY()
	TObjectPtr<class UTextureCube> SkyCubemap = nullptr;

	bool bSkyAssetsLoadAttempted = false;
```
테스트 접근자(GetGroundColorForTest 인근)에 추가:
```cpp
	float GetFogDensityForTest() const;
	bool GetSkyTintForTest(FLinearColor& OutTint) const;
	bool GetPropScalarForTest(int32 Index, const FName& Param, float& OutValue) const;
```

- [ ] **Step 2: cpp include 추가**

`IdleProjectGameModeBase.cpp` include 블록에 추가:
```cpp
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/TextureCube.h"
```

- [ ] **Step 3: SpawnDefaultEnvironment에 Fog/SkySphere spawn 추가**

`SpawnDefaultEnvironment`의 프롭/바닥 spawn 이후, 함수 끝 직전(`}` 앞)에 추가:
```cpp
	// 안개(대기 무드). 색/밀도는 ApplyMapTheme에서 챕터별 갱신.
	ThemeFog = World->SpawnActor<AExponentialHeightFog>(FVector(0.0f, 0.0f, 0.0f), FRotator::ZeroRotator, Params);

	// 스카이 스피어(역방향 배경). M_Sky MID는 ApplyMapTheme에서 SkyTint 갱신.
	if (UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		ThemeSkySphere = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (ThemeSkySphere)
		{
			ThemeSkySphere->SetMobility(EComponentMobility::Movable);
			if (UStaticMeshComponent* SphereComp = ThemeSkySphere->GetStaticMeshComponent())
			{
				SphereComp->SetStaticMesh(SphereMesh);
				SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			ThemeSkySphere->SetActorScale3D(FVector(-100.0f, -100.0f, -100.0f)); // 음수=역방향(안쪽이 보임)
		}
	}
```

- [ ] **Step 4: ApplyMapTheme에 안개/스카이/프롭 질감 적용**

(a) ApplyMapTheme 내 스카이 에셋 lazy 로드(테마 머티리얼 lazy 로드 인근)에 추가:
```cpp
	if (!bSkyAssetsLoadAttempted)
	{
		bSkyAssetsLoadAttempted = true;
		SkyMaterial = Cast<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Maps/M_Sky.M_Sky")).TryLoad());
		SkyCubemap = Cast<UTextureCube>(FSoftObjectPath(TEXT("/Game/Maps/TC_MapSky.TC_MapSky")).TryLoad());
	}
```
(b) 안개 적용(조명 적용 블록 인근에 추가):
```cpp
	if (ThemeFog)
	{
		if (UExponentialHeightFogComponent* FogComp = ThemeFog->GetComponent())
		{
			FogComp->SetFogInscatteringColor(Theme.FogColor);
			FogComp->SetFogDensity(Theme.FogDensity);
		}
	}
```
(c) 스카이 스피어 틴트 + SkyLight 큐브맵(조명 블록 인근):
```cpp
	if (ThemeSkySphere && SkyMaterial)
	{
		if (UStaticMeshComponent* SphereComp = ThemeSkySphere->GetStaticMeshComponent())
		{
			SphereComp->SetMaterial(0, SkyMaterial);
			if (UMaterialInstanceDynamic* SkyMID = SphereComp->CreateAndSetMaterialInstanceDynamic(0))
			{
				SkyMID->SetVectorParameterValue(TEXT("SkyTint"), Theme.SkyTint);
			}
		}
	}
	if (ThemeSky && SkyCubemap)
	{
		if (USkyLightComponent* SkyComp = ThemeSky->GetLightComponent())
		{
			SkyComp->Cubemap = SkyCubemap;
			SkyComp->SourceType = SLS_SpecifiedCubemap;
			SkyComp->SetLightColor(Theme.SkyColor);
			SkyComp->RecaptureSky();
		}
	}
```
(d) 프롭 MID 스칼라(프롭 spawn 루프의 MID 블록, `SetVectorParameterValue("Color", Prop.Color)` 다음)에 추가:
```cpp
					MID->SetScalarParameterValue(TEXT("Metallic"), Prop.Metallic);
					MID->SetScalarParameterValue(TEXT("Roughness"), Prop.Roughness);
					MID->SetScalarParameterValue(TEXT("EmissiveStrength"), Prop.EmissiveStrength);
```
> API 주의: `AExponentialHeightFog::GetComponent()`, `SetFogInscatteringColor`/`SetFogDensity`, `USkyLightComponent::Cubemap`/`SourceType`/`RecaptureSky`가 UE5.7과 다르면 엔진 헤더 확인 후 조정.

- [ ] **Step 5: 테스트 접근자 구현**

`ApplyMapTheme` 정의 다음에 추가:
```cpp
float AIdleProjectGameModeBase::GetFogDensityForTest() const
{
	if (ThemeFog)
	{
		if (UExponentialHeightFogComponent* FogComp = ThemeFog->GetComponent())
		{
			return FogComp->FogDensity;
		}
	}
	return -1.0f;
}

bool AIdleProjectGameModeBase::GetSkyTintForTest(FLinearColor& OutTint) const
{
	if (ThemeSkySphere)
	{
		if (UStaticMeshComponent* SphereComp = ThemeSkySphere->GetStaticMeshComponent())
		{
			if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(SphereComp->GetMaterial(0)))
			{
				return MID->GetVectorParameterValue(FMaterialParameterInfo(TEXT("SkyTint")), OutTint);
			}
		}
	}
	return false;
}

bool AIdleProjectGameModeBase::GetPropScalarForTest(int32 Index, const FName& Param, float& OutValue) const
{
	if (ThemeProps.IsValidIndex(Index) && ThemeProps[Index])
	{
		if (UStaticMeshComponent* Comp = ThemeProps[Index]->GetStaticMeshComponent())
		{
			if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Comp->GetMaterial(0)))
			{
				return MID->GetScalarParameterValue(FMaterialParameterInfo(Param), OutValue);
			}
		}
	}
	return false;
}
```

- [ ] **Step 6: 에디터 빌드 확인**

Run:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
```
Expected: 빌드 성공(C4458 0).

- [ ] **Step 7: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/IdleProjectGameModeBase.h client/Source/IdleProject/IdleProjectGameModeBase.cpp
git commit -m @'
맵 시각 심화(4) — Fog/SkySphere spawn + ApplyMapTheme 안개·스카이·질감 적용

ExponentialHeightFog/역방향 스카이 스피어 spawn, ApplyMapTheme에서 안개 색/밀도,
스카이 SkyTint MID + SkyLight 큐브맵, 프롭 MID Metallic/Roughness/Emissive 적용.
에셋/액터 무효 시 요소별 폴백. 테스트 접근자 3종.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 5: MapThemeTests 회귀 확장

**Files:**
- Modify: `client/Source/IdleProject/Tests/MapThemeTests.cpp`

- [ ] **Step 1: 라이브러리 테스트에 안개/스카이 단언 추가**

`FMapThemeLibraryTest::RunTest`의 챕터 루프 내(`T.SunIntensity > 0` 단언 인근)에 추가:
```cpp
		TestTrue(FString::Printf(TEXT("chapter %d fog density"), C), T.FogDensity > 0.0f);
```

- [ ] **Step 2: 에셋 테스트에 M_Sky/TC_MapSky/M_MapTheme 스칼라 추가**

`FMapThemeMaterialAssetTest::RunTest`의 `return true;` 직전에 추가:
```cpp
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
```

- [ ] **Step 3: Apply 테스트에 안개/스카이/질감 단언 추가**

`FMapThemeApplyTest`의 `World->DestroyWorld(false);` 직전(#105 GroundColor 단언 다음)에 추가:
```cpp
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
```

- [ ] **Step 4: MapTheme 게이트 실행**

Run:
```powershell
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1 -Filter "IdleProject.MapTheme"
```
Expected: `[ue-automation] GREEN.` — Library/Apply/MaterialAsset 통과(안개·스카이·질감 단언 포함).

- [ ] **Step 5: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/Tests/MapThemeTests.cpp
git commit -m @'
맵 시각 심화(5) — 안개/스카이/프롭 질감 회귀

라이브러리 FogDensity>0, 에셋 M_MapTheme Metallic·M_Sky SkyTint·TC_MapSky 로드,
적용 후 Fog Density·SkyTint·프롭 Roughness 단언.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 6: 전체 게이트 + 무변경 확인 (PM)

**Files:** (검증)

- [ ] **Step 1: 전체 IdleProject Automation + jumbo 빌드**

Run:
```powershell
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1
```
Expected: `[ue-automation] GREEN.`

- [ ] **Step 2: 세이브/서버 무변경 확인**

Run:
```powershell
cd "C:\game\idle game\repo"
git diff origin/main --stat -- server/ client/Source/IdleProject/GameCore/IdleSaveGame.h
```
Expected: 출력 없음(SaveVer 29, 서버 무변경).

- [ ] **Step 3: PR 발행(finishing-a-development-branch)**

전체 GREEN 후 `superpowers:finishing-a-development-branch` 옵션 2(push + PR). PR 본문에 시각 전용·폴백·세이브 무변경·헤드리스 한계 명시.

---

## Self-Review

- **스펙 커버리지:** 안개(§2-1)=Task 1 필드+4 팔레트+Task 4 spawn/적용+Task 5 회귀 / 프롭 질감(§2-2)=Task 1 필드+Task 2 커맨드릿+Task 4 MID+Task 5 / 절차 스카이(§2-3)=Task 3 커맨드릿+Task 4 spawn/적용+Task 5. 폴백(§9)=Task 4 `if(에셋)` 분기. 전부 매핑.
- **플레이스홀더:** 없음(8 팔레트 값 구체 표, 모든 코드 실체). 단 UTextureCube/Fog/SkyLight API는 UE5.7 검증 주석(빌드 게이트 적발).
- **타입 일관성:** `ThemeFog`(AExponentialHeightFog)/`ThemeSkySphere`(AStaticMeshActor, ThemeSky=SkyLight와 구분)/`SkyMaterial`/`SkyCubemap` 멤버가 헤더(Task4-1)·spawn(Task4-3)·적용(Task4-4)·접근자(Task4-5)·테스트(Task5)에서 일치. MID 파라미터명 `Metallic`/`Roughness`/`EmissiveStrength`/`SkyTint`/`SkyCube`가 커맨드릿(Task2/3)↔런타임(Task4)↔테스트(Task5) 동일.
- **네이밍 주의:** 기존 `ThemeSky`는 SkyLight(ASkyLight)이며 신규 스카이 스피어는 `ThemeSkySphere` — 혼동 금지.
