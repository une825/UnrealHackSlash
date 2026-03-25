#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HExpBarUI.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 플레이어의 경험치 바를 담당하는 UI 클래스입니다.
 * 블루프린트 위젯의 요소 이름과 변수명이 일치해야 자동으로 바인딩됩니다.
 */
UCLASS()
class MYHACKSLASH_API UHExpBarUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// 경험치 바의 퍼센트와 텍스트를 업데이트합니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateExpBar(const int InLevel, const float InCurrentExp, const float InMaxExp);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ExpProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ExpText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelText;

private:
	float TargetExpPercent = 0.0f;
	float CurrentExpPercent = 0.0f;
	const float InterpSpeed = 5.0f;
	
};
