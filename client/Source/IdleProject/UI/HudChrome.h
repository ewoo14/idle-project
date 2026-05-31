#pragma once
#include "CoreMinimal.h"

namespace IdleProject::UI
{
    enum class EPanelStyle : uint8 { Cream, Slate };
    enum class EBtnStyle   : uint8 { Gold, Slate };

    struct FUiRect { float X = 0, Y = 0, W = 0, H = 0; };
    struct FNineSliceCell { FUiRect Dst; FUiRect UV; }; // UV: 0..1

    // dst를 코너 CornerDst(px) 기준 9칸으로. TexSize=텍스처 한 변(px), CornerUV=텍스처 코너 px.
    // OutCells[0..8] 행우선(TL,TC,TR, ML,MC,MR, BL,BC,BR).
    void ComputeNineSlice(const FUiRect& Dst, float CornerDst, float TexSize, float CornerUV, FNineSliceCell OutCells[9]);

    FString PanelTextureName(EPanelStyle Style);
    FString ButtonTextureName(EBtnStyle Style);

    // (StartX,StarY)부터 가로로 Count개, 각 StarSize, 간격 Gap.
    TArray<FUiRect> ComputeStarRects(float StartX, float StarY, int32 Count, float StarSize, float Gap);
}
