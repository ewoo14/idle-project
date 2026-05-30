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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	float Metallic = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	float Roughness = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	float EmissiveStrength = 0.15f;
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
	FLinearColor FogColor = FLinearColor(0.5f, 0.55f, 0.6f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	float FogDensity = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	FLinearColor SkyTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|MapTheme")
	TArray<FMapProp> Props;
};
