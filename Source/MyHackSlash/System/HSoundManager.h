#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HSoundManager.generated.h"

class UAudioComponent;
class UHSoundDataAsset;

/**
 * @brief 사운드 재생을 중앙에서 관리하고, 중첩 재생(Audio Stacking)을 방지하는 매니저 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHSoundManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** @brief 데이터 에셋을 설정하여 매니저를 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	void InitializeManager(UHSoundDataAsset* InConfig);

	/** @brief 지정된 위치에서 사운드를 재생하되, 동일 사운드의 과도한 중첩을 방지합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	void PlaySoundAtLocationThrottled(USoundBase* InSound, FVector InLocation, float InVolumeMultiplier = 1.0f, float InPitchMultiplier = 1.0f, float InStartTime = 0.0f);

	/** @brief 키값을 이용하여 효과음(SFX)을 재생합니다. (레거시) */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	void PlaySFXByKey(FName InKey, FVector InLocation, float InVolumeMultiplier = 1.0f, bool bIsUI = false);

	/** @brief GameplayTag를 이용하여 효과음(SFX)을 재생합니다. (권장) */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	void PlaySFX(FGameplayTag InTag, FVector InLocation, float InVolumeMultiplier = 1.0f, bool bIsUI = false);

	/** @brief BGM을 재생합니다. 이미 재생 중인 BGM이 있다면 페이드 아웃 후 교체합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	void PlayBGM(USoundBase* InBGM, float InFadeInTime = 1.0f, float InFadeOutTime = 1.0f, bool bIsUI = false);

	/** @brief 키값을 이용하여 BGM을 재생합니다. (레거시) */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	void PlayBGMByKey(FName InKey, float InFadeInTime = 1.0f, float InFadeOutTime = 1.0f, bool bIsUI = false);

	/** @brief GameplayTag를 이용하여 BGM을 재생합니다. (권장) */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	void PlayBGMByTag(FGameplayTag InTag, float InFadeInTime = 1.0f, float InFadeOutTime = 1.0f, bool bIsUI = false);

	/** @brief 현재 재생 중인 BGM을 정지합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	void StopBGM(float InFadeOutTime = 1.0f);

	/** @brief 재생 중인 BGM의 볼륨을 조절합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	void SetBGMVolume(float InVolumeMultiplier);

	/** @brief 재생 중인 BGM의 피치(재생 속도)를 조절합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Sound")
	void SetBGMPitch(float InPitchMultiplier);

protected:
	/** @brief 사운드 데이터 에셋 */
	UPROPERTY()
	TObjectPtr<UHSoundDataAsset> SoundConfig;

	/** @brief 사운드별 마지막 재생 시간을 기록하여 Throttling을 수행합니다. (초 단위) */
	UPROPERTY()
	TMap<TObjectPtr<USoundBase>, float> LastPlayedTimes;

	/** @brief 동일 프레임/매우 짧은 시간에 사운드가 몰리는 것을 방지하기 위한 최소 간격 */
	float MinPlaybackInterval = 0.05f;

	/** @brief 현재 재생 중인 BGM 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UAudioComponent> CurrentBGMComponent;

	/** @brief 현재 재생 중인 BGM 에셋 */
	UPROPERTY()
	TObjectPtr<USoundBase> CurrentBGMAsset;
};
