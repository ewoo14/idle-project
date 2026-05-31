#include "UI/HudChrome.h"

namespace IdleProject::UI
{
    void ComputeNineSlice(const FUiRect& D, float C, float TexSize, float CUV, FNineSliceCell Out[9])
    {
        const float xs[4] = { D.X, D.X + C, D.X + D.W - C, D.X + D.W };
        const float ys[4] = { D.Y, D.Y + C, D.Y + D.H - C, D.Y + D.H };
        const float u = CUV / TexSize;
        const float us[4] = { 0.0f, u, 1.0f - u, 1.0f };
        for (int32 r = 0; r < 3; ++r)
        for (int32 c = 0; c < 3; ++c)
        {
            FNineSliceCell& Cell = Out[r * 3 + c];
            Cell.Dst = FUiRect{ xs[c], ys[r], xs[c + 1] - xs[c], ys[r + 1] - ys[r] };
            Cell.UV  = FUiRect{ us[c], us[r], us[c + 1] - us[c], us[r + 1] - us[r] };
        }
    }

    FString PanelTextureName(EPanelStyle Style)
    {
        return Style == EPanelStyle::Slate ? TEXT("T_GenshinPanelSlate") : TEXT("T_GenshinPanel");
    }
    FString ButtonTextureName(EBtnStyle Style)
    {
        return Style == EBtnStyle::Slate ? TEXT("T_GenshinButtonSlate") : TEXT("T_GenshinButtonGold");
    }

    TArray<FUiRect> ComputeStarRects(float StartX, float StarY, int32 Count, float StarSize, float Gap)
    {
        TArray<FUiRect> R;
        for (int32 i = 0; i < Count; ++i)
        {
            R.Add(FUiRect{ StartX + i * (StarSize + Gap), StarY, StarSize, StarSize });
        }
        return R;
    }
}
