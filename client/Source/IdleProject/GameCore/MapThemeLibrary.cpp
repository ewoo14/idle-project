#include "GameCore/MapThemeLibrary.h"

namespace
{
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
		T.FogColor = FLinearColor(0.70f,0.82f,0.95f); T.FogDensity = 0.012f; T.SkyTint = FLinearColor(0.70f,0.85f,1.0f);
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
		T.FogColor = FLinearColor(0.45f,0.60f,0.50f); T.FogDensity = 0.020f; T.SkyTint = FLinearColor(0.55f,0.70f,0.60f);
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
		T.FogColor = FLinearColor(0.30f,0.25f,0.42f); T.FogDensity = 0.030f; T.SkyTint = FLinearColor(0.30f,0.25f,0.45f);
		T.Props = {
			MakeProp(Cube, FVector(-680, 240, 90),  FVector(1.0f, 1.0f, 2.4f), FLinearColor(0.18f, 0.14f, 0.28f), FRotator(0, 20, 8), 0.0f, 0.5f, 0.2f),
			MakeProp(Cube, FVector(780, -220, 70),  FVector(1.2f, 1.2f, 1.8f), FLinearColor(0.16f, 0.12f, 0.26f), FRotator(0, -15, -6), 0.0f, 0.5f, 0.2f),
			MakeProp(Cube, FVector(120, 560, 120),  FVector(0.8f, 0.8f, 3.0f), FLinearColor(0.2f, 0.15f, 0.3f), FRotator(0, 40, 10), 0.0f, 0.5f, 0.2f),
			MakeProp(Cube, FVector(-300, 480, 60),  FVector(1.0f, 1.0f, 1.4f), FLinearColor(0.15f, 0.11f, 0.24f), FRotator(0, -30, 5), 0.0f, 0.5f, 0.2f),
		};
		break;
	case 4: // 폭풍/번개 — 차가운 청회, 바위 첨탑
		T.SunColor = FLinearColor(0.7f, 0.75f, 0.95f); T.SunIntensity = 4.0f; T.SunRotation = FRotator(-40, 50, 0);
		T.SkyIntensity = 0.8f; T.SkyColor = FLinearColor(0.4f, 0.45f, 0.6f); T.GroundColor = FLinearColor(0.18f, 0.18f, 0.22f);
		T.FogColor = FLinearColor(0.40f,0.45f,0.58f); T.FogDensity = 0.025f; T.SkyTint = FLinearColor(0.40f,0.45f,0.60f);
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
		T.FogColor = FLinearColor(0.12f,0.16f,0.32f); T.FogDensity = 0.040f; T.SkyTint = FLinearColor(0.15f,0.20f,0.40f);
		T.Props = {
			MakeProp(Cube, FVector(-700, 280, 140), FVector(0.9f, 0.9f, 4.0f), FLinearColor(0.12f, 0.14f, 0.22f), FRotator::ZeroRotator, 0.6f, 0.4f, 0.1f),
			MakeProp(Cube, FVector(700, -280, 140), FVector(0.9f, 0.9f, 4.0f), FLinearColor(0.12f, 0.14f, 0.22f), FRotator::ZeroRotator, 0.6f, 0.4f, 0.1f),
			MakeProp(Cube, FVector(-100, 620, 170), FVector(1.0f, 1.0f, 4.8f), FLinearColor(0.14f, 0.16f, 0.26f), FRotator::ZeroRotator, 0.6f, 0.4f, 0.1f),
			MakeProp(Cube, FVector(360, 520, 120),  FVector(0.8f, 0.8f, 3.4f), FLinearColor(0.1f, 0.12f, 0.2f), FRotator::ZeroRotator, 0.6f, 0.4f, 0.1f),
		};
		break;
	case 6: // 무너지는 근원 — 회보라, 부서진 큐브
		T.SunColor = FLinearColor(0.6f, 0.5f, 0.7f); T.SunIntensity = 3.0f; T.SunRotation = FRotator(-55, 20, 0);
		T.SkyIntensity = 0.55f; T.SkyColor = FLinearColor(0.35f, 0.3f, 0.4f); T.GroundColor = FLinearColor(0.16f, 0.13f, 0.16f);
		T.FogColor = FLinearColor(0.32f,0.27f,0.36f); T.FogDensity = 0.035f; T.SkyTint = FLinearColor(0.35f,0.30f,0.40f);
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
		T.FogColor = FLinearColor(0.55f,0.20f,0.15f); T.FogDensity = 0.030f; T.SkyTint = FLinearColor(0.60f,0.25f,0.20f);
		T.Props = {
			MakeProp(Cube, FVector(-650, 280, 320), FVector(0.4f, 0.4f, 2.2f), FLinearColor(0.5f, 0.18f, 0.12f), FRotator(35, 20, 18), 0.0f, 0.25f, 0.7f),
			MakeProp(Cube, FVector(720, -220, 360), FVector(0.35f, 0.35f, 2.6f), FLinearColor(0.55f, 0.2f, 0.12f), FRotator(-30, -15, 22), 0.0f, 0.25f, 0.7f),
			MakeProp(Cube, FVector(60, 600, 420),   FVector(0.45f, 0.45f, 3.0f), FLinearColor(0.6f, 0.22f, 0.14f), FRotator(45, 40, 10), 0.0f, 0.25f, 0.7f),
			MakeProp(Cube, FVector(-280, 460, 280), FVector(0.3f, 0.3f, 1.8f), FLinearColor(0.5f, 0.18f, 0.1f), FRotator(-25, -30, 30), 0.0f, 0.25f, 0.7f),
		};
		break;
	case 8: // 후존계 — 차가운 청백 정적, 부서진 기둥
	default:
		T.SunColor = FLinearColor(0.6f, 0.8f, 0.95f); T.SunIntensity = 4.0f; T.SunRotation = FRotator(-60, 35, 0);
		T.SkyIntensity = 1.0f; T.SkyColor = FLinearColor(0.5f, 0.65f, 0.8f); T.GroundColor = FLinearColor(0.18f, 0.22f, 0.26f);
		T.FogColor = FLinearColor(0.50f,0.62f,0.75f); T.FogDensity = 0.020f; T.SkyTint = FLinearColor(0.50f,0.65f,0.80f);
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
