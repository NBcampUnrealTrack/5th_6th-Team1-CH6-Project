#include "UI/UW_Scope.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Components/Border.h"

void UUW_Scope::InitScope(UTextureRenderTarget2D* InRT)
{
    if (!ScopeMaterial || !Render || !InRT) return;

    CachedMID = UMaterialInstanceDynamic::Create(ScopeMaterial, this);

    CachedMID->SetTextureParameterValue(TEXT("ScopeTexture"), InRT);

    FSlateBrush Brush;
    Brush.SetResourceObject(CachedMID);
    Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
    Render->SetBrush(Brush);
}

