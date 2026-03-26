#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "HGemInventoryEntryUI.generated.h"

class UImage;

/**
 * GemInventory 리스트 뷰의 항목 데이터 객체입니다.
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHGemInventoryEntryData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Gem")
	bool bIsEmpty = true;

	UPROPERTY(BlueprintReadOnly, Category = "Gem")
	TObjectPtr<class UHGemBase> GemBase;
};

/**
 * GemInventory의 개별 항목을 표시하는 UI 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHGemInventoryEntryUI : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	// IUserObjectListEntry 인터페이스 구현
	virtual void NativeOnListItemObjectSet(UObject* InListItemObject) override;

	// 드래그 앤 드롭 관련 오버라이드
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> AbilityIconImage;

	/** @brief 현재 이 슬롯에 할당된 데이터 (드래그 시 페이로드로 사용) */
	UPROPERTY()
	TObjectPtr<UHGemInventoryEntryData> CurrentEntryData;
};
