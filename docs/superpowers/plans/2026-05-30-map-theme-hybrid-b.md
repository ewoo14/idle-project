# 맵 테마 하이브리드 B — 정밀 색 구현 플랜

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 헤드리스 에디터 커맨드릿으로 `M_MapTheme`(Color 벡터 파라미터) 머티리얼 에셋을 생성·커밋하고, 런타임이 바닥·프롭에 적용해 챕터/프롭 정밀 색이 실제로 반영되게 한다.

**Architecture:** (1) `WITH_EDITOR` 커맨드릿이 `UMaterialEditingLibrary`로 `/Game/Maps/M_MapTheme` 절차 생성→LFS 커밋. (2) `ApplyMapTheme`이 에셋 lazy 로드해 바닥·프롭 컴포넌트 머티리얼을 교체 후 기존 MID `Color` 틴트 적용(에셋 부재 시 #104 폴백). (3) `MapThemeTests`가 에셋 로드·파라미터·적용 후 MID 값 회귀.

**Tech Stack:** UE5.7 C++ (IdleProject 모듈), 에디터 모듈(UnrealEd/MaterialEditor/AssetTools, 에디터 타깃 한정), Automation 테스트, Git LFS(.uasset).

---

## 파일 구조

- `client/Source/IdleProject/IdleProject.Build.cs` — 수정: 에디터 타깃 조건부 에디터 모듈 추가.
- `client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.h` — 신규: 커맨드릿 선언.
- `client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.cpp` — 신규: 머티리얼 절차 생성+저장(`WITH_EDITOR`).
- `client/Content/Maps/M_MapTheme.uasset` — 신규(커맨드릿 산출, LFS).
- `client/Source/IdleProject/IdleProjectGameModeBase.h` — 수정: `ThemeMaterial` 멤버 + 테스트 접근자.
- `client/Source/IdleProject/IdleProjectGameModeBase.cpp` — 수정: `ApplyMapTheme` 색 적용 분기 보강.
- `client/Source/IdleProject/Tests/MapThemeTests.cpp` — 수정: 에셋/머티리얼/MID 회귀 추가.

---

### Task 1: Build.cs 에디터 모듈 + 커맨드릿 스켈레톤

**Files:**
- Modify: `client/Source/IdleProject/IdleProject.Build.cs`
- Create: `client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.h`
- Create: `client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.cpp`

- [ ] **Step 1: Build.cs에 에디터 타깃 조건부 모듈 추가**

`IdleProject.Build.cs`의 생성자 끝(`DynamicallyLoadedModuleNames.AddRange(...)` 블록 다음)에 추가:

```csharp
		// 에디터 타깃에서만: 맵 테마 머티리얼 절차 생성 커맨드릿용 에디터 모듈.
		// 런타임/패키지 빌드에는 포함되지 않아 시각 전용·무영향.
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"MaterialEditor",
				"AssetTools"
			});
		}
```

- [ ] **Step 2: 커맨드릿 헤더 작성**

`Commandlets/GenerateMapThemeMaterialCommandlet.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "GenerateMapThemeMaterialCommandlet.generated.h"

/**
 * 맵 테마용 파라미터 머티리얼(/Game/Maps/M_MapTheme)을 절차적으로 생성·저장하는
 * 에디터 전용 커맨드릿. "Color" 벡터 파라미터 → BaseColor(+약한 Emissive), Roughness 상수.
 * 실행: UnrealEditor-Cmd.exe "<IdleProject.uproject>" -run=GenerateMapThemeMaterial
 *       -unattended -nopause -nosplash
 */
UCLASS()
class UGenerateMapThemeMaterialCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};
```

- [ ] **Step 3: 커맨드릿 본문 작성(빈 스텁, 컴파일 통과 목적)**

`Commandlets/GenerateMapThemeMaterialCommandlet.cpp` (이 Task에서는 컴파일만 — 본문은 Task 2에서 채움):

```cpp
#include "GenerateMapThemeMaterialCommandlet.h"

int32 UGenerateMapThemeMaterialCommandlet::Main(const FString& Params)
{
	// Task 2에서 머티리얼 생성 로직 구현.
	return 0;
}
```

- [ ] **Step 4: 에디터 빌드로 컴파일 확인**

Run:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
```
Expected: 빌드 성공(exit 0). 커맨드릿 클래스 등록·에디터 모듈 링크 확인.

- [ ] **Step 5: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/IdleProject.Build.cs client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.h client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.cpp
git commit -m @'
맵 테마 하이브리드 B(1) — 에디터 모듈 + 머티리얼 생성 커맨드릿 스켈레톤

IdleProject.Build.cs에 에디터 타깃 조건부 UnrealEd/MaterialEditor/AssetTools
추가, UGenerateMapThemeMaterialCommandlet 선언/스텁. 런타임 빌드 무영향.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 2: 커맨드릿 머티리얼 절차 생성 본문

**Files:**
- Modify: `client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.cpp`

- [ ] **Step 1: Main 본문 구현(머티리얼 그래프 + 저장)**

`GenerateMapThemeMaterialCommandlet.cpp` 전체를 아래로 교체. `UMaterialEditingLibrary`(MaterialEditor 모듈)의 정적 헬퍼로 버전 안정적으로 노드 연결한다. UE5.7에서 시그니처가 다르면 엔진 헤더(`MaterialEditingLibrary.h`)에 맞춰 조정:

```cpp
#include "GenerateMapThemeMaterialCommandlet.h"

#if WITH_EDITOR
#include "Factories/MaterialFactoryNew.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "MaterialEditingLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "SceneTypes.h" // EMaterialProperty(MP_BaseColor 등)
#endif

int32 UGenerateMapThemeMaterialCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	const FString PackageName = TEXT("/Game/Maps/M_MapTheme");
	const FString AssetName = TEXT("M_MapTheme");

	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("[GenMapThemeMat] 패키지 생성 실패: %s"), *PackageName);
		return 1;
	}
	Package->FullyLoad();

	UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
	UMaterial* Material = Cast<UMaterial>(Factory->FactoryCreateNew(
		UMaterial::StaticClass(), Package, *AssetName,
		RF_Standalone | RF_Public, nullptr, GWarn));
	if (!Material)
	{
		UE_LOG(LogTemp, Error, TEXT("[GenMapThemeMat] 머티리얼 생성 실패"));
		return 1;
	}

	// Color 벡터 파라미터 → BaseColor
	UMaterialExpressionVectorParameter* ColorParam = Cast<UMaterialExpressionVectorParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionVectorParameter::StaticClass(), -500, 0));
	ColorParam->ParameterName = TEXT("Color");
	ColorParam->DefaultValue = FLinearColor::White;
	UMaterialEditingLibrary::ConnectMaterialProperty(ColorParam, TEXT(""), MP_BaseColor);

	// Color * 0.15 → Emissive(어두운 챕터 식별성)
	UMaterialExpressionConstant* EmiScale = Cast<UMaterialExpressionConstant>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -500, 250));
	EmiScale->R = 0.15f;
	UMaterialExpressionMultiply* Mul = Cast<UMaterialExpressionMultiply>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionMultiply::StaticClass(), -250, 150));
	UMaterialEditingLibrary::ConnectMaterialExpressions(ColorParam, TEXT(""), Mul, TEXT("A"));
	UMaterialEditingLibrary::ConnectMaterialExpressions(EmiScale, TEXT(""), Mul, TEXT("B"));
	UMaterialEditingLibrary::ConnectMaterialProperty(Mul, TEXT(""), MP_EmissiveColor);

	// Roughness 0.85(무광)
	UMaterialExpressionConstant* Rough = Cast<UMaterialExpressionConstant>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -500, 450));
	Rough->R = 0.85f;
	UMaterialEditingLibrary::ConnectMaterialProperty(Rough, TEXT(""), MP_Roughness);

	UMaterialEditingLibrary::RecompileMaterial(Material);
	Material->PostEditChange();
	Material->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Material);

	const FString FileName = FPackageName::LongPackageNameToFilename(
		PackageName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Standalone | RF_Public;
	const bool bSaved = UPackage::SavePackage(Package, Material, *FileName, SaveArgs);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Error, TEXT("[GenMapThemeMat] 저장 실패: %s"), *FileName);
		return 1;
	}
	UE_LOG(LogTemp, Display, TEXT("[GenMapThemeMat] 저장 완료: %s"), *FileName);
	return 0;
#else
	UE_LOG(LogTemp, Error, TEXT("[GenMapThemeMat] 에디터 빌드에서만 실행 가능"));
	return 1;
#endif
}
```

- [ ] **Step 2: 에디터 빌드 확인**

Run:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
```
Expected: 빌드 성공. (API 시그니처 불일치 시 `MaterialEditingLibrary.h`/`SceneTypes.h` 참조해 조정 후 재빌드.)

- [ ] **Step 3: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/Commandlets/GenerateMapThemeMaterialCommandlet.cpp
git commit -m @'
맵 테마 하이브리드 B(2) — 커맨드릿 머티리얼 절차 생성 본문

UMaterialEditingLibrary로 Color 벡터 파라미터→BaseColor, Color*0.15→Emissive,
Roughness 0.85 그래프 구성 후 /Game/Maps/M_MapTheme 저장. WITH_EDITOR 가드.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 3: 헤드리스 커맨드릿 실행 → 에셋 생성 + LFS 커밋

**Files:**
- Create: `client/Content/Maps/M_MapTheme.uasset` (커맨드릿 산출)

- [ ] **Step 1: 커맨드릿 헤드리스 실행**

Run:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\game\idle game\repo\client\IdleProject.uproject" -run=GenerateMapThemeMaterial -unattended -nopause -nosplash -stdout
```
Expected: 로그에 `[GenMapThemeMat] 저장 완료: .../Content/Maps/M_MapTheme.uasset`. exit 0.

- [ ] **Step 2: 산출물 + LFS 추적 확인**

Run:
```powershell
cd "C:\game\idle game\repo"
git status --short client/Content/Maps/
git check-attr filter -- client/Content/Maps/M_MapTheme.uasset
```
Expected: `M_MapTheme.uasset`가 신규(`??`)로 보이고 `filter: lfs`. (`.gitattributes`의 `*.uasset filter=lfs` 적용.)

- [ ] **Step 3: 커밋(LFS)**

```powershell
cd "C:\game\idle game\repo"
git add client/Content/Maps/M_MapTheme.uasset
git commit -m @'
맵 테마 하이브리드 B(3) — M_MapTheme 파라미터 머티리얼 에셋 생성

GenerateMapThemeMaterial 커맨드릿 헤드리스 실행 산출물(LFS). 런타임이 바닥·프롭에
적용해 Color 틴트가 실제 반영되는 베이스 머티리얼.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
git lfs ls-files | Select-String M_MapTheme
```
Expected: 커밋 성공. `git lfs ls-files`에 `M_MapTheme.uasset` 등장(LFS 포인터로 저장됨).

---

### Task 4: 런타임 ApplyMapTheme 색 적용 보강

**Files:**
- Modify: `client/Source/IdleProject/IdleProjectGameModeBase.h`
- Modify: `client/Source/IdleProject/IdleProjectGameModeBase.cpp:137-190`

- [ ] **Step 1: 헤더에 ThemeMaterial 멤버 + 테스트 접근자 추가**

`IdleProjectGameModeBase.h`에서 `int32 AppliedThemeChapter = -1;`(70행) 다음, 그리고 테스트 접근자(`GetThemePropCountForTest` 인근 32행)에 추가:

private 멤버 영역(`AppliedThemeChapter` 아래)에:
```cpp
	// 맵 테마 파라미터 머티리얼(/Game/Maps/M_MapTheme). lazy 로드 캐시.
	// 무효(에셋 부재/로드 실패) 시 기존 베이스 머티리얼 폴백.
	UPROPERTY()
	TObjectPtr<class UMaterialInterface> ThemeMaterial = nullptr;

	bool bThemeMaterialLoadAttempted = false;
```

public 테스트 접근자 영역(`GetThemePropCountForTest` 다음)에:
```cpp
	// 테스트: 적용된 바닥 컴포넌트의 현재 머티리얼(0) 반환(MID 또는 베이스).
	class UMaterialInterface* GetGroundMaterialForTest() const;
	// 테스트: 바닥 MID의 "Color" 벡터 파라미터 값 조회(MID 아니면 false).
	bool GetGroundColorForTest(FLinearColor& OutColor) const;
```

- [ ] **Step 2: cpp 상단 include 추가**

`IdleProjectGameModeBase.cpp` include 블록에 (없으면) 추가:
```cpp
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
```

- [ ] **Step 3: ThemeMaterial lazy 로드 + 바닥 적용 보강**

`ApplyMapTheme`의 바닥 색 블록(`IdleProjectGameModeBase.cpp:137-151`)을 아래로 교체:

```cpp
	// 테마 머티리얼 1회 lazy 로드(에셋 부재 시 폴백).
	if (!bThemeMaterialLoadAttempted)
	{
		bThemeMaterialLoadAttempted = true;
		static const FSoftObjectPath ThemeMatPath(TEXT("/Game/Maps/M_MapTheme.M_MapTheme"));
		ThemeMaterial = Cast<UMaterialInterface>(ThemeMatPath.TryLoad());
	}

	// 바닥 색(파라미터 머티리얼 있으면 실제 틴트, 없으면 기존 베이스 best-effort).
	if (ThemeGround)
	{
		if (UStaticMeshComponent* GroundMesh = ThemeGround->GetStaticMeshComponent())
		{
			if (ThemeMaterial)
			{
				GroundMesh->SetMaterial(0, ThemeMaterial);
			}
			if (GroundMesh->GetMaterial(0))
			{
				UMaterialInstanceDynamic* MID = GroundMesh->CreateAndSetMaterialInstanceDynamic(0);
				if (MID)
				{
					MID->SetVectorParameterValue(TEXT("Color"), Theme.GroundColor);
				}
			}
		}
	}
```

- [ ] **Step 4: 프롭 색 적용 보강**

프롭 spawn 루프의 머티리얼 블록(`IdleProjectGameModeBase.cpp:182-189`)을 아래로 교체:

```cpp
			if (ThemeMaterial)
			{
				Comp->SetMaterial(0, ThemeMaterial);
			}
			if (Comp->GetMaterial(0))
			{
				UMaterialInstanceDynamic* MID = Comp->CreateAndSetMaterialInstanceDynamic(0);
				if (MID)
				{
					MID->SetVectorParameterValue(TEXT("Color"), Prop.Color);
				}
			}
```

- [ ] **Step 5: 테스트 접근자 구현**

`ApplyMapTheme` 함수 정의 다음(또는 `HandleStageChangedForTheme` 인근)에 추가:

```cpp
UMaterialInterface* AIdleProjectGameModeBase::GetGroundMaterialForTest() const
{
	if (ThemeGround)
	{
		if (UStaticMeshComponent* GroundMesh = ThemeGround->GetStaticMeshComponent())
		{
			return GroundMesh->GetMaterial(0);
		}
	}
	return nullptr;
}

bool AIdleProjectGameModeBase::GetGroundColorForTest(FLinearColor& OutColor) const
{
	if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(GetGroundMaterialForTest()))
	{
		return MID->GetVectorParameterValue(FMaterialParameterInfo(TEXT("Color")), OutColor);
	}
	return false;
}
```

- [ ] **Step 6: 에디터 빌드 확인**

Run:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
```
Expected: 빌드 성공(C4458 멤버 셰도잉 등 경고-에러 0).

- [ ] **Step 7: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/IdleProjectGameModeBase.h client/Source/IdleProject/IdleProjectGameModeBase.cpp
git commit -m @'
맵 테마 하이브리드 B(4) — 런타임 바닥·프롭에 M_MapTheme 적용

ThemeMaterial lazy 로드 후 바닥·프롭 컴포넌트 머티리얼 교체 → 기존 Color MID
틴트 실제 반영. 에셋 무효 시 #104 베이스 폴백. 테스트 접근자 2종 추가.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 5: MapThemeTests 회귀 확장

**Files:**
- Modify: `client/Source/IdleProject/Tests/MapThemeTests.cpp`

- [ ] **Step 1: 에셋 로드 + Color 파라미터 테스트 추가**

`MapThemeTests.cpp`의 `#include` 블록에 추가:
```cpp
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
```

`FMapThemeApplyTest` 정의 위에 신규 테스트 추가:
```cpp
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
	// "Color" 벡터 파라미터 보유.
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Mat, nullptr);
	FLinearColor Out;
	const bool bHas = MID && MID->GetVectorParameterValue(FMaterialParameterInfo(TEXT("Color")), Out);
	TestTrue(TEXT("Color 벡터 파라미터 존재"), bHas);
	return true;
}
```

- [ ] **Step 2: 적용 후 MID Color 값 단언을 FMapThemeApplyTest에 추가**

`FMapThemeApplyTest`의 `World->DestroyWorld(false);` 직전에 추가:
```cpp
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
```

> 주: 헤드리스 Automation 월드에서 `M_MapTheme` 로드 성공 시 MID 경로 단언, 실패 시 폴백이라 단언 skip(크래시 0) — 게이트는 `FMapThemeMaterialAssetTest`가 에셋 존재를 강제.

- [ ] **Step 3: 표준 게이트 실행(빌드 + Automation)**

Run:
```powershell
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1 -Filter "IdleProject.MapTheme"
```
Expected: `[ue-automation] GREEN.` — `MapTheme.Library` / `MapTheme.Apply` / `MapTheme.MaterialAsset` 통과.

- [ ] **Step 4: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/Tests/MapThemeTests.cpp
git commit -m @'
맵 테마 하이브리드 B(5) — M_MapTheme 로드/Color 파라미터/적용 MID 회귀

MaterialAsset 테스트(에셋 로드 + Color 벡터 파라미터 강제), Apply 테스트에
바닥 MID Color == 테마 GroundColor 단언 추가.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 6: 전체 게이트 + 세이브/서버 무변경 확인 (PM)

**Files:** (코드 변경 없음 — 검증 단계)

- [ ] **Step 1: 전체 IdleProject Automation + 표준 jumbo 빌드**

Run:
```powershell
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1
```
Expected: `[ue-automation] GREEN.` (전체 IdleProject — SaveSystem 포함 회귀 0).

- [ ] **Step 2: 세이브/서버 무변경 확인**

Run:
```powershell
cd "C:\game\idle game\repo"
git diff origin/main --stat -- server/ client/Source/IdleProject/GameCore/IdleSaveGame.h
```
Expected: 출력 없음(서버·SaveGame 헤더 무변경 — SaveVer 29 유지, 시각 전용).

- [ ] **Step 3: 최종 리뷰 후 finishing-a-development-branch로 PR 생성**

전체 GREEN 확인 후 `superpowers:finishing-a-development-branch`로 PR 발행(옵션 2). PR 본문에 시각 전용·폴백·세이브 무변경 명시.

---

## Self-Review

- **스펙 커버리지:** 커맨드릿(스펙 §3)=Task 1·2·3 / 런타임 적용(§4)=Task 4 / 테스트(§6)=Task 5 / 게이트·무변경(§6·§7)=Task 6. 폴백(§5·§7)=Task 4 Step 3·4의 `if (ThemeMaterial)` 분기 + GetMaterial 가드. 전부 매핑됨.
- **플레이스홀더:** 없음(모든 코드 블록 실체).
- **타입 일관성:** `ThemeMaterial`(TObjectPtr<UMaterialInterface>), `GetGroundColorForTest`/`GetGroundMaterialForTest` 시그니처가 헤더(Task 4 Step 1)와 구현(Step 5)·테스트(Task 5 Step 2) 사용처에서 일치. `FMaterialParameterInfo(TEXT("Color"))` 일관.
- **버전 주의:** `UMaterialEditingLibrary`/`SceneTypes.h` API는 UE5.7 기준 — 빌드 게이트가 시그니처 불일치 적발(Task 2 Step 2 주석).
