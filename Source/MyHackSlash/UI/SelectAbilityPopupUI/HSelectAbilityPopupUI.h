#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HSelectAbilityPopupUI.generated.h"

class UTextBlock;
class UListView;

/**
 * 능력 선택 팝업의 메인 UI 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHSelectAbilityPopupUI : public UUserWidget
{
	GENERATED_BODY()

public:
	/** @brief 보상 옵션을 새롭게 생성하여 UI를 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category = "Select Ability")
	void RefreshOptions();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// 블루프린트 위젯 바인딩
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> DescText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UListView> AbilityListView;
};
