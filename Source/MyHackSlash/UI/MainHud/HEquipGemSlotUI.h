#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HEquipGemSlotUI.generated.h"

class UImage;
class UListView;
class UHMainGem;

/**
 * 하나의 메인 젬 슬롯과 연결된 보조 젬들을 표시하는 UI입니다.
 */
UCLASS()
class MYHACKSLASH_API UHEquipGemSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	/** @brief 특정 메인 젬의 데이터를 UI에 반영합니다. */
	UFUNCTION(BlueprintCallable, Category = "Gem|UI")
	void SetMainGem(UHMainGem* InMainGem);

	/** @brief 현재 설정된 메인 젬 정보를 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category = "Gem|UI")
	void ClearSlot();

protected:
	virtual void NativeConstruct() override;

	/** @brief 컴포넌트의 데이터를 기반으로 슬롯 UI를 최신화합니다. */
	UFUNCTION()
	void Refresh();

	// 드래그 앤 드롭 관련 오버라이드
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:
	/** @brief 이 UI가 담당하는 장착 슬롯 인덱스 (0~3) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|UI")
	int32 SlotIndex = 0;

	/** @brief 메인 젬의 아이콘을 표시할 이미지 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MainGemIcon;

	/** @brief 연결된 보조 젬 리스트를 보여줄 리스트 뷰 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UListView> SubGemSlotListView;
};
