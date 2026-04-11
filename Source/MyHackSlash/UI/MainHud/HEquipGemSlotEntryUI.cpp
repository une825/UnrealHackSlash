#include "UI/MainHud/HEquipGemSlotEntryUI.h"
#include "Components/Image.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "DataAsset/HGemDataAsset.h"
#include "UI/MainHud/HGemDragDropOp.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UHEquipGemSlotEntryUI::NativeOnListItemObjectSet(UObject* InListItemObject)
{
	CurrentEntryData = Cast<UHEquipGemSlotEntryData>(InListItemObject);
	if (!CurrentEntryData || !CurrentEntryData->SupportGem)
	{
		if (IconImage)
		{
			IconImage->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	// 보조 젬 데이터에서 아이콘을 가져와 설정
	const FHGemData& GemData = CurrentEntryData->SupportGem->GetGemData();
	if (IconImage && GemData.Icon)
	{
		IconImage->SetBrushFromTexture(GemData.Icon);
		IconImage->SetVisibility(ESlateVisibility::Visible);
	}
	else if (IconImage)
	{
		IconImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

FReply UHEquipGemSlotEntryUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return FReply::Unhandled();
}

void UHEquipGemSlotEntryUI::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (nullptr == CurrentEntryData || nullptr == CurrentEntryData->SupportGem)
	{
		return;
	}

	// 1. DragDropOperation 생성
	UHGemDragDropOp* DragOp = NewObject<UHGemDragDropOp>();
	DragOp->DraggedGem = CurrentEntryData->SupportGem;
	DragOp->SourceWidget = this;
	DragOp->SourceSlotIndex = CurrentEntryData->SourceSlotIndex;

	// 2. 비주얼 설정 (아이콘 잔상)
	const FHGemData& GemData = CurrentEntryData->SupportGem->GetGemData();
	if (GemData.Icon)
	{
		UImage* DragVisual = NewObject<UImage>(this);
		if (DragVisual)
		{
			DragVisual->SetBrushFromTexture(GemData.Icon);

			FVector2D DragIconSize = FVector2D(64, 64);
			if (IconImage)
			{
				DragIconSize = IconImage->GetDesiredSize();
			}
			DragVisual->SetDesiredSizeOverride(DragIconSize);

			DragOp->DefaultDragVisual = DragVisual;
			DragOp->Pivot = EDragPivot::MouseDown;
		}
	}

	OutOperation = DragOp;
}
