#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "HEquipGemSlotEntryUI.generated.h"

class UImage;
class UHSupportGem;

/**
 * EquipGemSlot의 ListView 항목으로 사용될 데이터 클래스입니다.
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHEquipGemSlotEntryData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Gem")
	TObjectPtr<UHSupportGem> SupportGem;

	/** @brief 이 보조 젬이 속한 메인 젬의 슬롯 인덱스 */
	UPROPERTY(BlueprintReadOnly, Category = "Gem")
	int32 SourceSlotIndex = -1;
};

/**
 * 보조 젬 슬롯 리스트의 개별 항목을 표시하는 UI 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHEquipGemSlotEntryUI : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	// IUserObjectListEntry Interface
	virtual void NativeOnListItemObjectSet(UObject* InListItemObject) override;

	// 드래그 앤 드롭 관련 오버라이드
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

protected:
	/** @brief 보조 젬의 아이콘을 표시할 이미지 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	/** @brief 현재 이 슬롯에 할당된 데이터 */
	UPROPERTY()
	TObjectPtr<UHEquipGemSlotEntryData> CurrentEntryData;
};
