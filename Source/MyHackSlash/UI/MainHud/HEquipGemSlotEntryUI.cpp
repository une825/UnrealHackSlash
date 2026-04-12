#include "UI/MainHud/HEquipGemSlotEntryUI.h"
#include "Components/Image.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "Skill/SkillGem/HMainGem.h"
#include "DataAsset/HGemDataAsset.h"
#include "UI/MainHud/HGemDragDropOp.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Skill/HEquipmentComponent.h"

void UHEquipGemSlotEntryUI::NativeOnListItemObjectSet(UObject* InListItemObject)
{
	CurrentEntryData = Cast<UHEquipGemSlotEntryData>(InListItemObject);
	
	if (!CurrentEntryData)
	{
		UE_LOG(LogTemp, Error, TEXT("[UHEquipGemSlotEntryUI] NativeOnListItemObjectSet: InListItemObject is NULL or not UHEquipGemSlotEntryData"));
		if (IconImage) IconImage->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	if (CurrentEntryData->bIsEmpty || !CurrentEntryData->SupportGem)
	{
		if (IconImage)
		{
			// 빈 슬롯: 아이콘을 숨기거나 완전히 투명하게 설정
			IconImage->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	// 보조 젬 데이터에서 아이콘을 가져와 설정
	const FHGemData& GemData = CurrentEntryData->SupportGem->GetGemData();
	if (IconImage)
	{
		if (GemData.Icon)
		{
			IconImage->SetBrushFromTexture(GemData.Icon);
			IconImage->SetVisibility(ESlateVisibility::Visible);
			IconImage->SetRenderOpacity(1.0f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[UHEquipGemSlotEntryUI] Gem %s has NO Icon!"), *GemData.GemID.ToString());
			IconImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UHEquipGemSlotEntryUI] IconImage widget is NOT bound (nullptr)! Check Widget Blueprint."));
	}
}

FReply UHEquipGemSlotEntryUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 빈 슬롯이 아닐 때만 드래그 감지
		if (CurrentEntryData && !CurrentEntryData->bIsEmpty)
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
		}
	}

	return FReply::Unhandled();
}

void UHEquipGemSlotEntryUI::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (nullptr == CurrentEntryData || CurrentEntryData->bIsEmpty || nullptr == CurrentEntryData->SupportGem)
	{
		return;
	}

	// 1. DragDropOperation 생성
	UHGemDragDropOp* DragOp = NewObject<UHGemDragDropOp>();
	DragOp->DraggedGem = CurrentEntryData->SupportGem;
	DragOp->SourceWidget = this;
	DragOp->SourceSlotIndex = CurrentEntryData->SourceSlotIndex;

	// 2. 비주얼 설정
	const FHGemData& GemData = CurrentEntryData->SupportGem->GetGemData();
	if (GemData.Icon)
	{
		UImage* DragVisual = NewObject<UImage>(this);
		DragVisual->SetBrushFromTexture(GemData.Icon);
		DragVisual->SetDesiredSizeOverride(FVector2D(64, 64)); // 보조 젬은 조금 작게
		DragOp->DefaultDragVisual = DragVisual;
	}

	// 3. 드래그 시작 시 해당 메인 젬에서 보조 젬 일시 제거 (선택 사항: 이동 효과를 위함)
	// 여기서는 단순히 드래그 데이터를 넘기고, 최종 Drop 위치에 따라 처리를 맡깁니다.
	
	OutOperation = DragOp;
}

bool UHEquipGemSlotEntryUI::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UHGemDragDropOp* GemOp = Cast<UHGemDragDropOp>(InOperation);
	if (!GemOp || !GemOp->DraggedGem || !CurrentEntryData) return false;

	// 보조 젬만 이 슬롯에 드롭 가능
	UHSupportGem* SupportGem = Cast<UHSupportGem>(GemOp->DraggedGem);
	if (!SupportGem) return false;

	AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwningPlayerPawn());
	if (!Player || !Player->GetEquipmentComponent()) return false;

	UHEquipmentComponent* EquipComp = Player->GetEquipmentComponent();

	// 드래그 소스가 인벤토리이거나 다른 슬롯일 경우, 현재 슬롯(SourceSlotIndex)의 메인 젬에 장착 시도
	if (EquipComp->EquipSupportGem(CurrentEntryData->SourceSlotIndex, SupportGem))
	{
		return true;
	}

	return false;
}
