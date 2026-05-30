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
