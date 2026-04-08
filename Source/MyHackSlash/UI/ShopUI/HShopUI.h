// Source/MyHackSlash/UI/ShopUI/HShopUI.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "System/HWaveManager.h"
#include "HShopUI.generated.h"

class UTileView;
class UTextBlock;
class UImage;
class UButton;

/**
 * 상점 메인 화면을 담당하는 위젯 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHShopUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** @brief 현재 보유 중인 재화 수치를 갱신합니다. */
	void RefreshCurrency(int32 InNewCurrency);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTileView> ShopTileView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WaveTypeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CurrencyIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrencyText; // 보유 재화

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NextWaveButton;

protected:
	/** @brief 웨이브 정보를 기반으로 상단 텍스트를 초기화합니다. */
	void InitWaveInfo();

	/** @brief 다음 웨이브 진행 버튼 클릭 시 호출됩니다. */
	UFUNCTION()
	void OnClickNextWave();
};
