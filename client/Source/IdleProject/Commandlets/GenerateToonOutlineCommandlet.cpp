#include "GenerateToonOutlineCommandlet.h"

#if WITH_EDITOR
#include "Factories/MaterialFactoryNew.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionSceneTexture.h"
#include "Materials/MaterialExpressionDDX.h"
#include "Materials/MaterialExpressionDDY.h"
#include "Materials/MaterialExpressionAbs.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "MaterialEditingLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "SceneTypes.h"
#include "MaterialDomain.h"
#include "MaterialSceneTextureId.h"
#include "Engine/BlendableInterface.h"
#endif

int32 UGenerateToonOutlineCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	const FString PackageName = TEXT("/Game/Render/M_ToonOutline");
	const FString AssetName = TEXT("M_ToonOutline");

	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("[GenToonOutline] 패키지 생성 실패: %s"), *PackageName);
		return 1;
	}
	Package->FullyLoad();

	UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
	UMaterial* Material = Cast<UMaterial>(Factory->FactoryCreateNew(
		UMaterial::StaticClass(), Package, *AssetName,
		RF_Standalone | RF_Public, nullptr, GWarn));
	if (!Material)
	{
		UE_LOG(LogTemp, Error, TEXT("[GenToonOutline] 머티리얼 생성 실패"));
		return 1;
	}

	// 포스트프로세스 도메인. DOF 이후 씬 컬러 단계에 합성(=구 BL_BeforeTonemapping), 우선순위 0.
	Material->MaterialDomain = MD_PostProcess;
	Material->BlendableLocation = BL_SceneColorAfterDOF;
	Material->BlendablePriority = 0;

	// --- 깊이 에지 검출 그래프(DDX/DDY 기반) ---
	// SceneDepth(Color 출력=깊이) 의 화면공간 미분(DDX/DDY) 절대값 합 = 이웃 픽셀 간
	// 깊이 변화량. 깊이가 급격히 변하는 실루엣 경계에서 큰 값이 나온다.
	// 명시적 4-이웃 UV 오프셋 샘플 대신 미분을 쓰면 API 배선이 단순하고 안정적이며,
	// 결과적으로 동일한 "중심 vs 이웃 깊이 차" 에지를 만든다.

	// SceneDepth 샘플
	UMaterialExpressionSceneTexture* DepthTex = Cast<UMaterialExpressionSceneTexture>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionSceneTexture::StaticClass(), -900, -100));
	DepthTex->SceneTextureId = PPI_SceneDepth;

	// DDX(깊이)
	UMaterialExpressionDDX* Ddx = Cast<UMaterialExpressionDDX>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionDDX::StaticClass(), -650, -200));
	UMaterialEditingLibrary::ConnectMaterialExpressions(DepthTex, TEXT("Color"), Ddx, TEXT("Value"));

	// DDY(깊이)
	UMaterialExpressionDDY* Ddy = Cast<UMaterialExpressionDDY>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionDDY::StaticClass(), -650, 0));
	UMaterialEditingLibrary::ConnectMaterialExpressions(DepthTex, TEXT("Color"), Ddy, TEXT("Value"));

	// |DDX|, |DDY|
	UMaterialExpressionAbs* AbsX = Cast<UMaterialExpressionAbs>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionAbs::StaticClass(), -450, -200));
	UMaterialEditingLibrary::ConnectMaterialExpressions(Ddx, TEXT(""), AbsX, TEXT(""));

	UMaterialExpressionAbs* AbsY = Cast<UMaterialExpressionAbs>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionAbs::StaticClass(), -450, 0));
	UMaterialEditingLibrary::ConnectMaterialExpressions(Ddy, TEXT(""), AbsY, TEXT(""));

	// 에지 강도 = |DDX| + |DDY| (중심 vs 4-이웃 깊이 차의 미분 근사)
	UMaterialExpressionAdd* EdgeMag = Cast<UMaterialExpressionAdd>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionAdd::StaticClass(), -250, -100));
	UMaterialEditingLibrary::ConnectMaterialExpressions(AbsX, TEXT(""), EdgeMag, TEXT("A"));
	UMaterialEditingLibrary::ConnectMaterialExpressions(AbsY, TEXT(""), EdgeMag, TEXT("B"));

	// 임계값 상수(깊이 단위). 이보다 큰 변화량을 윤곽선으로 본다.
	UMaterialExpressionConstant* Threshold = Cast<UMaterialExpressionConstant>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -250, 150));
	Threshold->R = 2.0f;

	// 에지 마스크: If(EdgeMag > Threshold ? 1 : 0). A==B 도 0 으로 처리.
	UMaterialExpressionConstant* OneConst = Cast<UMaterialExpressionConstant>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -250, 300));
	OneConst->R = 1.0f;
	UMaterialExpressionConstant* ZeroConst = Cast<UMaterialExpressionConstant>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -250, 420));
	ZeroConst->R = 0.0f;

	UMaterialExpressionIf* EdgeIf = Cast<UMaterialExpressionIf>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionIf::StaticClass(), 0, 0));
	UMaterialEditingLibrary::ConnectMaterialExpressions(EdgeMag, TEXT(""), EdgeIf, TEXT("A"));            // A = 에지 강도
	UMaterialEditingLibrary::ConnectMaterialExpressions(Threshold, TEXT(""), EdgeIf, TEXT("B"));          // B = 임계값
	UMaterialEditingLibrary::ConnectMaterialExpressions(OneConst, TEXT(""), EdgeIf, TEXT("A > B"));       // 에지면 1
	UMaterialEditingLibrary::ConnectMaterialExpressions(ZeroConst, TEXT(""), EdgeIf, TEXT("A == B"));     // 동일 0
	UMaterialEditingLibrary::ConnectMaterialExpressions(ZeroConst, TEXT(""), EdgeIf, TEXT("A < B"));      // 미만 0

	// 씬 컬러(PostProcessInput0)
	UMaterialExpressionSceneTexture* SceneColor = Cast<UMaterialExpressionSceneTexture>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionSceneTexture::StaticClass(), 0, 300));
	SceneColor->SceneTextureId = PPI_PostProcessInput0;

	// 어두운 아웃라인 색(거의 검정)
	UMaterialExpressionConstant3Vector* OutlineColor = Cast<UMaterialExpressionConstant3Vector>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), 0, 480));
	OutlineColor->Constant = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);

	// Lerp(씬컬러, 아웃라인색, 에지마스크) → EmissiveColor
	UMaterialExpressionLinearInterpolate* Lerp = Cast<UMaterialExpressionLinearInterpolate>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 300, 200));
	UMaterialEditingLibrary::ConnectMaterialExpressions(SceneColor, TEXT("Color"), Lerp, TEXT("A"));
	UMaterialEditingLibrary::ConnectMaterialExpressions(OutlineColor, TEXT(""), Lerp, TEXT("B"));
	UMaterialEditingLibrary::ConnectMaterialExpressions(EdgeIf, TEXT(""), Lerp, TEXT("Alpha"));
	UMaterialEditingLibrary::ConnectMaterialProperty(Lerp, TEXT(""), MP_EmissiveColor);

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
		UE_LOG(LogTemp, Error, TEXT("[GenToonOutline] 저장 실패: %s"), *FileName);
		return 1;
	}
	UE_LOG(LogTemp, Display, TEXT("[GenToonOutline] 저장 완료: %s"), *FileName);
	return 0;
#else
	UE_LOG(LogTemp, Error, TEXT("[GenToonOutline] 에디터 빌드에서만 실행 가능"));
	return 1;
#endif
}
