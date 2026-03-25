#include "UI/MainHud/HEquipGemSlot.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Skill/SkillGem/HMainGem.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "DataAsset/HGemDataAsset.h"
#include "UI/MainHud/HEquipGemSlotEntryUI.h"
#include "UI/MainHud/HGemDragDropOp.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Skill/HEquipmentComponent.h"
#include "Skill/HGemInventoryComponent.h"

void UHEquipGemSlot::SetMainGem(UHMainGem* InMainGem)
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
		if (GemData.GemIcon)
		{
			MainGemIcon->SetBrushFromTexture(GemData.GemIcon);
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
				SubGemSlotListView->AddItem(EntryData);
			}
		}
	}
}

void UHEquipGemSlot::ClearSlot()
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

bool UHEquipGemSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UHGemDragDropOp* GemOp = Cast<UHGemDragDropOp>(InOperation);
	if (!GemOp || !GemOp->DraggedGem) return false;

	AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwningPlayerPawn());
	if (!Player || !Player->GetEquipmentComponent()) return false;

	UHEquipmentComponent* EquipComp = Player->GetEquipmentComponent();

	// 1. 드래그된 젬이 메인 젬인 경우 (슬롯 교체)
	if (UHMainGem* MainGem = Cast<UHMainGem>(GemOp->DraggedGem))
	{
		// 인벤토리에서 먼저 제거 시도
		if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
		{
			InvComp->RemoveGemInstance(MainGem);
		}

		if (EquipComp->EquipGem(SlotIndex, MainGem))
		{
			SetMainGem(MainGem);
			return true;
		}
	}
	// 2. 드래그된 젬이 보조 젬인 경우 (현재 장착된 메인 젬에 연결)
	else if (UHSupportGem* SupportGem = Cast<UHSupportGem>(GemOp->DraggedGem))
	{
		UHMainGem* CurrentMainGem = EquipComp->GetEquippedGem(SlotIndex);
		if (CurrentMainGem)
		{
			if (CurrentMainGem->AddSupportGem(SupportGem))
			{
				// 인벤토리에서 제거
				if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
				{
					InvComp->RemoveGemInstance(SupportGem);
				}
				
				SetMainGem(CurrentMainGem);
				return true;
			}
		}
	}

	return false;
}
