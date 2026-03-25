#include "UI/MainHud/HSkillGemInventoryEntryUI.h"
#include "Components/Image.h"
#include "Skill/SkillGem/HGemBase.h"
#include "DataAsset/HGemDataAsset.h"
#include "UI/MainHud/HGemDragDropOp.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UHSkillGemInventoryEntryUI::NativeOnListItemObjectSet(UObject* InListItemObject)
{
	CurrentEntryData = Cast<UHSkillGemEntryData>(InListItemObject);
	if (!CurrentEntryData || CurrentEntryData->bIsEmpty || !CurrentEntryData->GemBase)
	{
		if (AbilityIconImage)
		{
			AbilityIconImage->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	// 젬 아이콘 표시
	const FHGemData& GemData = CurrentEntryData->GemBase->GetGemData();
	if (AbilityIconImage && GemData.GemIcon)
	{
		AbilityIconImage->SetBrushFromTexture(GemData.GemIcon);
		AbilityIconImage->SetVisibility(ESlateVisibility::Visible);
	}
}

FReply UHSkillGemInventoryEntryUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 드래그 감지 시작
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return FReply::Unhandled();
}

void UHSkillGemInventoryEntryUI::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (nullptr == CurrentEntryData || CurrentEntryData->bIsEmpty || nullptr == CurrentEntryData->GemBase)
	{
		return;
	}

	// 1. DragDropOperation 생성
	UHGemDragDropOp* DragOp = NewObject<UHGemDragDropOp>();
	DragOp->DraggedGem = CurrentEntryData->GemBase;
	DragOp->SourceWidget = this;

	// 2. 드래그 시 마우스를 따라다닐 비주얼 설정 (아이콘 잔상)
	const FHGemData& GemData = CurrentEntryData->GemBase->GetGemData();
	if (GemData.GemIcon)
	{
		// UUserWidget 대신 UImage를 직접 생성하여 비주얼로 사용
		UImage* DragVisual = NewObject<UImage>(this);
		if (DragVisual)
		{
			DragVisual->SetBrushFromTexture(GemData.GemIcon);
			
			// 이미지 크기 조절
			FVector2D DragIconSize = FVector2D(128, 128);
			if (AbilityIconImage)
			{
				DragIconSize = AbilityIconImage->GetDesiredSize();
			}
			DragVisual->SetDesiredSizeOverride(DragIconSize);
			
			DragOp->DefaultDragVisual = DragVisual;
			DragOp->Pivot = EDragPivot::MouseDown; // 마우스 클릭 지점을 기준으로 드래그
		}
	}
	
	OutOperation = DragOp;
}
