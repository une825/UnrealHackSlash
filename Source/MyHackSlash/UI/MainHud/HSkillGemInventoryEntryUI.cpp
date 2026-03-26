#include "UI/MainHud/HSkillGemInventoryEntryUI.h"
#include "Components/Image.h"
#include "Skill/SkillGem/HGemBase.h"
#include "Skill/SkillGem/HMainGem.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "DataAsset/HGemDataAsset.h"
#include "UI/MainHud/HGemDragDropOp.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Skill/HEquipmentComponent.h"
#include "Skill/HGemInventoryComponent.h"

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
	DragOp->SourceSlotIndex = -1; // 인벤토리에서 시작함

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

bool UHSkillGemInventoryEntryUI::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UHGemDragDropOp* GemOp = Cast<UHGemDragDropOp>(InOperation);
	if (!GemOp || !GemOp->DraggedGem) return false;

	// 장착 슬롯에서 온 드래그인 경우에만 장착 해제 처리
	if (GemOp->SourceSlotIndex == -1) return false;

	AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwningPlayerPawn());
	if (!Player || !Player->GetEquipmentComponent()) return false;

	UHEquipmentComponent* EquipComp = Player->GetEquipmentComponent();

	// 1. 메인 젬인 경우 (슬롯 전체 해제)
	if (UHMainGem* MainGem = Cast<UHMainGem>(GemOp->DraggedGem))
	{
		EquipComp->UnequipGem(GemOp->SourceSlotIndex);
		return true;
	}
	// 2. 보조 젬인 경우 (해당 메인 젬에서 보조 젬만 제거)
	else if (UHSupportGem* SupportGem = Cast<UHSupportGem>(GemOp->DraggedGem))
	{
		UHMainGem* CurrentMainGem = EquipComp->GetEquippedGem(GemOp->SourceSlotIndex);
		if (CurrentMainGem)
		{
			CurrentMainGem->RemoveSupportGem(SupportGem);

			// 보조 젬을 인벤토리에 다시 추가
			if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
			{
				InvComp->AddGemInstance(SupportGem);
			}

			// UI 갱신 (EquipmentComponent의 델리게이트 브로드캐스트)
			EquipComp->OnEquipmentChanged.Broadcast();
			return true;
		}
	}

	return false;
}
