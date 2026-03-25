#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HSkillGemInventory.generated.h"

class UListView;

/**
 * 스킬 젬 인벤토리를 관리하는 UI 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHSkillGemInventory : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** 인벤토리 목록을 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
	void RefreshInventory();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UListView> InventoryListView;

private:
	/** 항상 유지할 10개의 슬롯 데이터 객체들 */
	UPROPERTY()
	TArray<TObjectPtr<class UHSkillGemEntryData>> InventorySlotDatas;
};
