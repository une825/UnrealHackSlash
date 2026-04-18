#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HGemIconUI.generated.h"

class UImage;

/**
 * 젬 아이콘과 티어(성급)를 표시하는 공용 위젯 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHGemIconUI : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 젬 정보를 기반으로 UI를 설정합니다. */
	UFUNCTION(BlueprintCallable, Category = "Gem|UI")
	void SetGemInfo(FName GemID, int32 Tier);

protected:
	/** 젬 아이콘 이미지 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> GemIconImage;

	/** 티어 표시용 별 이미지들 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> TierStarImage_0;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> TierStarImage_1;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> TierStarImage_2;
};
