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

protected:
	/** @brief 보조 젬의 아이콘을 표시할 이미지 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;
};
