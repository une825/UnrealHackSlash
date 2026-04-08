#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HMainHudUI.generated.h"

class UTextBlock;
class UProgressBar;
class UImage;
class UWidgetAnimation;

/**
 * 게임의 메인 HUD를 담당하는 기본 C++ 클래스입니다.
 * 블루프린트(bp_MainHud)의 부모 클래스로 설정하여 사용할 수 있습니다.
 */
UCLASS()
class MYHACKSLASH_API UHMainHudUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 초기화 로직이 필요할 경우 여기에 추가합니다.
	virtual void NativeConstruct() override;

	/** @brief 현재 골드 텍스트를 최신 정보로 갱신합니다. */
	void RefreshGold(int32 InNewGold);

	/** @brief 피격 시 블러드 이펙트 애니메이션을 재생합니다. */
	void PlayDamageEffectAnim();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RemainTimeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KillProgressText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> WaveProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BloodEffectImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> PlayDamageEffect;

protected:
	UFUNCTION()
	void UpdateWaveInfo(int32 InWaveIndex, EHWaveType InWaveType, EHWaveClearType InClearType);

	UFUNCTION()
	void UpdateWaveProgress(float InProgressPercent, float InCurrentValue, float InTargetValue);

	UFUNCTION()
	void OnWaveCompleted(int32 InWaveIndex);
};
