#include "UI/MainHud/HEquipGemSlotUI.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Skill/SkillGem/HMainGem.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "DataAsset/HGemDataAsset.h"
#include "UI/MainHud/HEquipGemSlotEntryUI.h"
#include "UI/MainHud/HGemDragDropOp.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Skill/HEquipmentComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UHEquipGemSlotUI::SetMainGem(UHMainGem* InMainGem)
{
	if (nullptr == InMainGem)
	{
		ClearSlot();
		return;
	}

	// 1. 메인 젬 아이콘 설정
	if (MainGemIcon)
	{
		const FHGemData& GemData = InMainGem->GetGemData();
		if (GemData.Icon)
		{
			MainGemIcon->SetBrushFromTexture(GemData.Icon);
			MainGemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			MainGemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// 2. 보조 젬 리스트 갱신
	if (SubGemSlotListView)
	{
		SubGemSlotListView->ClearListItems();

		const TArray<UHSupportGem*>& SupportGems = InMainGem->GetSupportGems();
		for (UHSupportGem* SupportGem : SupportGems)
		{
			if (SupportGem)
			{
				UHEquipGemSlotEntryData* EntryData = NewObject<UHEquipGemSlotEntryData>(this);
				EntryData->SupportGem = SupportGem;
				EntryData->SourceSlotIndex = SlotIndex;
				SubGemSlotListView->AddItem(EntryData);
			}
		}
	}
}

void UHEquipGemSlotUI::ClearSlot()
{
	if (MainGemIcon)
	{
		MainGemIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (SubGemSlotListView)
	{
		SubGemSlotListView->ClearListItems();
	}
}

void UHEquipGemSlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (AHPlayerCharacter* PlayerCharacter = Cast<AHPlayerCharacter>(GetOwningPlayerPawn()))
	{
		if (UHEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
		{
			// 장착 상태 변경 델리게이트 바인딩
			EquipComp->OnEquipmentChanged.AddDynamic(this, &UHEquipGemSlotUI::Refresh);
			
			// 초기 데이터 반영
			Refresh();
		}
	}
}

void UHEquipGemSlotUI::Refresh()
{
	if (AHPlayerCharacter* PlayerCharacter = Cast<AHPlayerCharacter>(GetOwningPlayerPawn()))
	{
		if (UHEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
		{
			// 현재 슬롯 인덱스에 맞는 메인 젬을 가져와서 UI 갱신
			UHMainGem* EquippedGem = EquipComp->GetEquippedGem(SlotIndex);
			SetMainGem(EquippedGem);
		}
	}
}

FReply UHEquipGemSlotUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return FReply::Unhandled();
}

void UHEquipGemSlotUI::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwningPlayerPawn());
	if (!Player || !Player->GetEquipmentComponent()) return;

	UHMainGem* CurrentGem = Player->GetEquipmentComponent()->GetEquippedGem(SlotIndex);
	if (!CurrentGem) return;

	// 1. DragDropOperation 생성
	UHGemDragDropOp* DragOp = NewObject<UHGemDragDropOp>();
	DragOp->DraggedGem = CurrentGem;
	DragOp->SourceWidget = this;
	DragOp->SourceSlotIndex = SlotIndex;

	// 2. 비주얼 설정 (아이콘 잔상)
	const FHGemData& GemData = CurrentGem->GetGemData();
	if (GemData.Icon)
	{
		UImage* DragVisual = NewObject<UImage>(this);
		if (DragVisual)
		{
			DragVisual->SetBrushFromTexture(GemData.Icon);

			FVector2D DragIconSize = FVector2D(128, 128);
			if (MainGemIcon)
			{
				DragIconSize = MainGemIcon->GetDesiredSize();
			}
			DragVisual->SetDesiredSizeOverride(DragIconSize);

			DragOp->DefaultDragVisual = DragVisual;
			DragOp->Pivot = EDragPivot::MouseDown;
		}
	}

	OutOperation = DragOp;
}

bool UHEquipGemSlotUI::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UHGemDragDropOp* GemOp = Cast<UHGemDragDropOp>(InOperation);
	if (!GemOp || !GemOp->DraggedGem) return false;

	AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwningPlayerPawn());
	if (!Player || !Player->GetEquipmentComponent()) return false;

	UHEquipmentComponent* EquipComp = Player->GetEquipmentComponent();

	// 1. 드래그된 젬이 메인 젬인 경우 (슬롯 교체)
	if (UHMainGem* MainGem = Cast<UHMainGem>(GemOp->DraggedGem))
	{
		if (EquipComp->EquipGem(SlotIndex, MainGem))
		{
			return true;
		}
	}
	// 2. 드래그된 젬이 보조 젬인 경우 (현재 장착된 메인 젬에 연결)
	else if (UHSupportGem* SupportGem = Cast<UHSupportGem>(GemOp->DraggedGem))
	{
		if (EquipComp->EquipSupportGem(SlotIndex, SupportGem))
		{
			return true;
		}
	}

	return false;
}
