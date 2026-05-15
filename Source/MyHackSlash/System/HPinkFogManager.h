#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "System/HGlobalTypes.h"
#include "HPinkFogManager.generated.h"

class UHPinkFogConfigDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPinkFogStateChanged, EHPinkFogState, NewState);

/**
 * 핑크 안개 이벤트를 총괄 관리하는 월드 서브시스템입니다.
 */
UCLASS()
class MYHACKSLASH_API UHPinkFogManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** @brief 핑크 안개 이벤트를 강제로 시작합니다. */
	UFUNCTION(BlueprintCallable, Category = "PinkFog")
	void StartPinkFog();

	/** @brief 핑크 안개 이벤트를 강제로 종료합니다. */
	UFUNCTION(BlueprintCallable, Category = "PinkFog")
	void StopPinkFog();

	/** @brief 현재 안개 상태를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "PinkFog")
	EHPinkFogState GetCurrentState() const { return CurrentState; }

	/** @brief 현재 보상 배율을 반환합니다. (안개 활성 시 적용) */
	UFUNCTION(BlueprintPure, Category = "PinkFog")
	float GetRewardMultiplier() const;

	/** @brief 핑크 안개 설정 데이터 에셋을 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "PinkFog")
	UHPinkFogConfigDataAsset* GetConfig() const { return Config; }

	/** @brief 웨이브 시작 시 확률적으로 안개 발생 체크 */
	UFUNCTION()
	void OnWaveStarted(int32 Index, EHWaveType Type, EHWaveClearType ClearType);

	/** @brief 웨이브 종료 시 체크 타이머 중지 */
	UFUNCTION()
	void OnWaveCompleted(int32 Index);

	/** @brief 안개 중 스폰되는 몬스터에게 버프 적용 */
	UFUNCTION()
	void OnMonsterSpawned(class AHBaseMonster* Monster);

public:
	/** @brief 안개 상태 변경 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "PinkFog")
	FOnPinkFogStateChanged OnPinkFogStateChanged;

protected:
	void SetState(EHPinkFogState NewState);
	
	void UpdateFading(float InDeltaTime);
	void UpdateActive(float InDeltaTime);

	/** @brief 현재 안개의 강도 (0~1, 보간용) */
	float CurrentFogIntensity = 0.0f;

	void UpdateVisuals(float InIntensity);

	void CacheVisualActors();

	/** @brief 주기적으로 안개 발생 여부를 체크합니다. */
	void CheckPinkFogOccurrence();

private:
	UPROPERTY()
	TObjectPtr<UHPinkFogConfigDataAsset> Config;

	UPROPERTY()
	TObjectPtr<class AExponentialHeightFog> CachedFogActor;

	UPROPERTY()
	TObjectPtr<class APostProcessVolume> CachedPPVolume;

	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> SpawnedFogNiagara;

	EHPinkFogState CurrentState = EHPinkFogState::Inactive;

	float StateTimer = 0.0f;

	FTimerHandle TickTimerHandle;
	FTimerHandle OccurrenceCheckTimerHandle;

	void TickPinkFog();
};
