#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HDamageTextWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class MYHACKSLASH_API UHDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 블루프린트의 'DamageText'라는 이름의 TextBlock과 자동 바인딩됩니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageText;

	// 블루프린트의 'ShowAnimation'이라는 이름의 애니메이션과 자동 바인딩됩니다. (선택 사항)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> ShowAnimation;

	// 데미지 수치 및 스타일 설정
	void SetDamage(float InDamage, bool bInIsCritical);

protected:
	// 일반 데미지 색상
	UPROPERTY(EditAnywhere, Category = "DamageText")
	FLinearColor NormalColor = FLinearColor::White;

	// 일반 데미지 윤곽선 색상
	UPROPERTY(EditAnywhere, Category = "DamageText")
	FLinearColor NormalOutlineColor = FLinearColor::Black;

	// 크리티컬 데미지 색상
	UPROPERTY(EditAnywhere, Category = "DamageText")
	FLinearColor CriticalColor = FLinearColor::Yellow;

	// 크리티컬 데미지 윤곽선 색상
	UPROPERTY(EditAnywhere, Category = "DamageText")
	FLinearColor CriticalOutlineColor = FLinearColor::Black;
};
