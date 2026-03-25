#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HHPBarUI.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 캐릭터의 HP 바를 담당하는 UI 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHHPBarUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// HP 바의 퍼센트와 텍스트를 업데이트합니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHPBar(float InCurrentHP, float InMaxHP);

protected:
	// 블루프린트의 ProgressBar 이름이 "HPProgressBar"여야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HPProgressBar;

	// 블루프린트의 TextBlock 이름이 "HPText"여야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HPText;

private:
	float TargetHPPercent = 0.0f;
	float CurrentHPPercent = 0.0f;
	const float InterpSpeed = 5.0f;
};
