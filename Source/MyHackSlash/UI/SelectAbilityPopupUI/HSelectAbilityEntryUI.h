#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "DataAsset/HSelectAbilityData.h"
#include "HSelectAbilityEntryUI.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UOverlay;

/**
 * 능력 선택 리스트 항목의 데이터를 담는 객체입니다. (UObject로 래핑하여 ListView에서 사용)
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHSelectAbilityData : public UObject
{
	GENERATED_BODY()

public:
	/** @brief 원본 보상 데이터 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FHRewardOptionData RewardOptionData;

	/** @brief 보상 데이터 설정을 위한 헬퍼 함수 */
	void SetRewardOptionData(const FHRewardOptionData& InData) { RewardOptionData = InData; }
};

/**
 * 능력 선택 팝업 내의 개별 항목을 담당하는 UI 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHSelectAbilityEntryUI : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* InListItemObject) override;

	/** @brief 선택 버튼 클릭 시 호출됩니다. */
	UFUNCTION()
	void OnSelectButtonClicked();

protected:
	// 블루프린트 위젯 바인딩
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DescText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SelectButton;

	// 등급별 배경/효과 오버레이
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UOverlay> SilverGrade;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UOverlay> GoldGrade;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UOverlay> DiaGrade;

private:
	/** @brief 현재 할당된 데이터 객체 */
	UPROPERTY()
	TWeakObjectPtr<UHSelectAbilityData> CurrentData;
};
