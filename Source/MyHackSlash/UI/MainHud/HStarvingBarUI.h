#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HStarvingBarUI.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 플레이어의 배고픔(Hunger) 바를 담당하는 UI 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHStarvingBarUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// 배고픔 바의 퍼센트와 텍스트를 업데이트합니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHungerBar(float InCurrentHunger, float InMaxHunger);

protected:
	// 블루프린트의 ProgressBar 이름이 "HungerProgressBar"여야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HungerProgressBar;

	// 블루프린트의 TextBlock 이름이 "HungerText"여야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HungerText;

private:
	float TargetHungerPercent = 0.0f;
	float CurrentHungerPercent = 0.0f;
	const float InterpSpeed = 5.0f;
};
