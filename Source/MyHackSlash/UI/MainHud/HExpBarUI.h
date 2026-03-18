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

public:
	// 경험치 바의 퍼센트와 텍스트를 업데이트합니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateExpBar(float CurrentExp, float MaxExp);

protected:
	// 블루프린트의 ProgressBar 이름이 "ExpProgressBar"여야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ExpProgressBar;

	// 블루프린트의 TextBlock 이름이 "ExpText"여야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ExpText;
};
