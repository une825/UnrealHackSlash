#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HMainHudUI.generated.h"

class UTextBlock;
class UProgressBar;
class UImage;
class UOverlay;
class UButton;
class UWidgetAnimation;
class UHEquipGemSlotUI;

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

	/** @brief 알림 텍스트를 화면에 표시합니다. */
	void ShowNotifyText(const FText& InText, const FLinearColor& InColor = FLinearColor::White, bool bPlayAnim = true);

	/** @brief 알림 텍스트를 숨깁니다. */
	void HideNotifyText();

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

	UPROPERTY(meta = (BindWidget, OptionalWidget))
	TObjectPtr<UButton> SettingButton;

	// 젬 장착 슬롯 위젯 (BindWidget)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHEquipGemSlotUI> EquipGemSlot_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHEquipGemSlotUI> EquipGemSlot_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHEquipGemSlotUI> EquipGemSlot_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHEquipGemSlotUI> EquipGemSlot_3;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> PlayDamageEffect;

	// --- Notification UI ---
	UPROPERTY(meta = (BindWidget, OptionalWidget))
	TObjectPtr<UOverlay> NotifyTextOverlay;

	UPROPERTY(meta = (BindWidget, OptionalWidget))
	TObjectPtr<UTextBlock> NotifyText;

	UPROPERTY(meta = (BindWidgetAnim, OptionalWidget), Transient)
	TObjectPtr<UWidgetAnimation> NotifyAnim;

protected:
	UFUNCTION()
	void UpdateWaveInfo(int32 InWaveIndex, EHWaveType InWaveType, EHWaveClearType InClearType);

	UFUNCTION()
	void UpdateWaveProgress(float InProgressPercent, float InCurrentValue, float InTargetValue);

	UFUNCTION()
	void OnWaveCompleted(int32 InWaveIndex);

	/** @brief 핑크 안개 상태 변경 시 UI 연출 처리 */
	UFUNCTION()
	void HandlePinkFogStateChanged(EHPinkFogState NewState);

	UFUNCTION()
	void OnSettingButtonClicked();

	FText GetGlobalText(FName InTextKey) const;
};
