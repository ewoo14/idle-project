#include "GenerateUiChromeAssetsCommandlet.h"

#if WITH_EDITOR
#include "Engine/Texture2D.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"

namespace
{
	// 둥근 사각형 알파 + 골드 더블 프레임 패널 픽셀(BGRA8) 생성.
	// S=한 변 픽셀, CornerPx=모서리 반지름, Fill=채움색(FColor(R,G,B) 순서), bGoldFrame=프레임 여부.
	// 저장은 BGRA 순서로 기록한다(P[0]=B, P[1]=G, P[2]=R, P[3]=A).
	TArray<uint8> MakePanelPixels(int32 S, float CornerPx, FColor Fill, bool bGoldFrame)
	{
		TArray<uint8> Px;
		Px.SetNumZeroed(S * S * 4);
		const float r = CornerPx;

		// 모서리 영역에서는 코너 원 내부 여부로, 그 외에는 항상 내부로 판정.
		auto InRounded = [&](float x, float y) -> bool
		{
			const float l = r, t = r, rt = S - r, bt = S - r;
			float dx = 0.f, dy = 0.f;
			if (x < l && y < t) { dx = l - x; dy = t - y; }
			else if (x > rt && y < t) { dx = x - rt; dy = t - y; }
			else if (x < l && y > bt) { dx = l - x; dy = y - bt; }
			else if (x > rt && y > bt) { dx = x - rt; dy = y - bt; }
			else { return true; }
			return (dx * dx + dy * dy) <= r * r;
		};

		// 가장 가까운 외곽 변까지의 거리(픽셀). 더블 프레임 배치에 사용.
		auto EdgeDist = [&](float x, float y) -> float
		{
			return FMath::Min(FMath::Min(x, (float)(S - 1) - x), FMath::Min(y, (float)(S - 1) - y));
		};

		for (int32 y = 0; y < S; ++y)
		for (int32 x = 0; x < S; ++x)
		{
			uint8* P = &Px[(y * S + x) * 4];
			const bool in = InRounded((float)x + 0.5f, (float)y + 0.5f);
			FColor c = Fill;
			const uint8 a = in ? 255 : 0;
			if (in && bGoldFrame)
			{
				const float e = EdgeDist((float)x, (float)y);
				if (e < 2.5f) { c = FColor(200, 170, 110); }            // 외곽 골드(짙은 금)
				else if (e >= 4.0f && e < 5.5f) { c = FColor(255, 246, 223); } // 내측 골드 하이라이트
			}
			P[0] = c.B; P[1] = c.G; P[2] = c.R; P[3] = a;
		}
		return Px;
	}

	// 중심에서 멀어질수록 알파가 0 으로 감소하는 소프트 라디얼 섀도우(검정) 픽셀.
	// alpha = clamp(1 - dist/half, 0, 1) * 0.8.
	TArray<uint8> MakeSoftShadowPixels(int32 S, FColor Fill)
	{
		TArray<uint8> Px;
		Px.SetNumZeroed(S * S * 4);
		const float cx = (S - 1) * 0.5f;
		const float cy = (S - 1) * 0.5f;
		const float half = S * 0.5f;
		for (int32 y = 0; y < S; ++y)
		for (int32 x = 0; x < S; ++x)
		{
			uint8* P = &Px[(y * S + x) * 4];
			const float dx = x - cx;
			const float dy = y - cy;
			const float dist = FMath::Sqrt(dx * dx + dy * dy);
			const float af = FMath::Clamp(1.f - dist / half, 0.f, 1.f) * 0.8f;
			const uint8 a = (uint8)FMath::RoundToInt(af * 255.f);
			P[0] = Fill.B; P[1] = Fill.G; P[2] = Fill.R; P[3] = a;
		}
		return Px;
	}

	// 5각 별 알파(채움). 중심을 기준으로 외/내 반지름을 번갈아 잇는 10정점 폴리곤 내부 판정.
	// Fill 색으로 채우고, 별 외부는 알파 0.
	TArray<uint8> MakeStarPixels(int32 S, FColor Fill)
	{
		TArray<uint8> Px;
		Px.SetNumZeroed(S * S * 4);
		const float cx = (S - 1) * 0.5f;
		const float cy = (S - 1) * 0.5f;
		const float outer = S * 0.46f;
		const float inner = outer * 0.40f;

		// 10개 정점(외/내 교대). 첫 꼭짓점이 위(-Y)를 향하도록 -PI/2 시작.
		TArray<FVector2D> Pts;
		Pts.Reserve(10);
		for (int32 i = 0; i < 10; ++i)
		{
			const float ang = -PI * 0.5f + (float)i * (PI / 5.f);
			const float rad = (i % 2 == 0) ? outer : inner;
			Pts.Add(FVector2D(cx + FMath::Cos(ang) * rad, cy + FMath::Sin(ang) * rad));
		}

		// 점-다각형 내부 판정(짝수-홀수 규칙).
		auto InStar = [&](float px, float py) -> bool
		{
			bool inside = false;
			const int32 n = Pts.Num();
			for (int32 i = 0, j = n - 1; i < n; j = i++)
			{
				const FVector2D& a = Pts[i];
				const FVector2D& b = Pts[j];
				if (((a.Y > py) != (b.Y > py)) &&
					(px < (b.X - a.X) * (py - a.Y) / (b.Y - a.Y) + a.X))
				{
					inside = !inside;
				}
			}
			return inside;
		};

		for (int32 y = 0; y < S; ++y)
		for (int32 x = 0; x < S; ++x)
		{
			uint8* P = &Px[(y * S + x) * 4];
			const bool in = InStar((float)x + 0.5f, (float)y + 0.5f);
			const uint8 a = in ? 255 : 0;
			P[0] = Fill.B; P[1] = Fill.G; P[2] = Fill.R; P[3] = a;
		}
		return Px;
	}

	// 단일 UTexture2D(BGRA8) 패키지 생성·저장. 실패 시 false.
	bool SaveChromeTexture(const FString& PackageName, int32 S, const TArray<uint8>& Pixels)
	{
		const FString AssetName = FPackageName::GetShortName(PackageName);
		UPackage* Pkg = CreatePackage(*PackageName);
		Pkg->FullyLoad();
		UTexture2D* Tex = NewObject<UTexture2D>(Pkg, *AssetName, RF_Standalone | RF_Public);

		Tex->Source.Init(S, S, 1, 1, TSF_BGRA8, Pixels.GetData());
		Tex->SRGB = true;
		Tex->CompressionSettings = TC_EditorIcon;
		Tex->MipGenSettings = TMGS_NoMipmaps;
		Tex->UpdateResource();
		Tex->PostEditChange();
		FAssetRegistryModule::AssetCreated(Tex);

		const FString File = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
		FSavePackageArgs Args; Args.TopLevelFlags = RF_Standalone | RF_Public;
		if (!UPackage::SavePackage(Pkg, Tex, *File, Args))
		{
			UE_LOG(LogTemp, Error, TEXT("[GenUiChrome] %s 저장 실패"), *PackageName);
			return false;
		}
		UE_LOG(LogTemp, Display, TEXT("[GenUiChrome] %s 저장(%dx%d)"), *PackageName, S, S);
		return true;
	}
}
#endif

int32 UGenerateUiChromeAssetsCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	int32 Saved = 0;

	// 패널 2종(둥근 사각형 + 골드 더블 프레임).
	if (!SaveChromeTexture(TEXT("/Game/UI/Chrome/T_GenshinPanel"), 256,
		MakePanelPixels(256, 28.f, FColor(243, 238, 226), true))) return 1; ++Saved;
	if (!SaveChromeTexture(TEXT("/Game/UI/Chrome/T_GenshinPanelSlate"), 256,
		MakePanelPixels(256, 28.f, FColor(59, 65, 82), true))) return 1; ++Saved;

	// 버튼 2종.
	if (!SaveChromeTexture(TEXT("/Game/UI/Chrome/T_GenshinButtonGold"), 128,
		MakePanelPixels(128, 18.f, FColor(240, 210, 120), true))) return 1; ++Saved;
	if (!SaveChromeTexture(TEXT("/Game/UI/Chrome/T_GenshinButtonSlate"), 128,
		MakePanelPixels(128, 18.f, FColor(75, 84, 104), true))) return 1; ++Saved;

	// 소프트 섀도우(라디얼 알파, 검정, 프레임 없음).
	if (!SaveChromeTexture(TEXT("/Game/UI/Chrome/T_SoftShadow"), 128,
		MakeSoftShadowPixels(128, FColor(0, 0, 0)))) return 1; ++Saved;

	// 골드 스타(5각 별 알파).
	if (!SaveChromeTexture(TEXT("/Game/UI/Chrome/T_GoldStar"), 64,
		MakeStarPixels(64, FColor(243, 169, 58)))) return 1; ++Saved;

	UE_LOG(LogTemp, Display, TEXT("[GenUiChrome] UI 크롬 텍스처 %d개 저장 완료"), Saved);
	return 0;
#else
	UE_LOG(LogTemp, Error, TEXT("[GenUiChrome] 에디터 빌드 전용"));
	return 1;
#endif
}
