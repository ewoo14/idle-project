#include "GenerateMapThemeMaterialCommandlet.h"

#if WITH_EDITOR
#include "Factories/MaterialFactoryNew.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "MaterialEditingLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "SceneTypes.h"
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

	UMaterialExpressionVectorParameter* ColorParam = Cast<UMaterialExpressionVectorParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionVectorParameter::StaticClass(), -500, 0));
	ColorParam->ParameterName = TEXT("Color");
	ColorParam->DefaultValue = FLinearColor::White;
	UMaterialEditingLibrary::ConnectMaterialProperty(ColorParam, TEXT(""), MP_BaseColor);

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
