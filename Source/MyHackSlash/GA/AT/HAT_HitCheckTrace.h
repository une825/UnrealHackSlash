// Source/MyHackSlash/GA/AT/HAT_HitCheckTrace.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "HAT_HitCheckTrace.generated.h"

class AHTA_HitCheckTrace;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHitCheckTraceDelegate, const FGameplayAbilityTargetDataHandle&, TargetData);

/**
 * 타겟 액터를 활용하여 트레이스 판정을 수행하고 결과를 전달하는 AbilityTask입니다.
 */
UCLASS()
class MYHACKSLASH_API UHAT_HitCheckTrace : public UAbilityTask
{
	GENERATED_BODY()

public:
	UHAT_HitCheckTrace();

	/** @brief 타겟 액터를 활용한 히트 체크 태스크를 생성합니다. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UHAT_HitCheckTrace* CreateTask(UGameplayAbility* OwningAbility, TSubclassOf<AHTA_HitCheckTrace> TargetActorClass);

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

	UPROPERTY(BlueprintAssignable)
	FHitCheckTraceDelegate OnTargetDataReady;

protected:
	/** 타겟 액터로부터 데이터가 준비되었을 때 호출되는 콜백 */
	UFUNCTION()
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& DataHandle);

	/** @brief 타겟 액터를 스폰하고 초기 설정을 수행합니다. */
	bool SpawnAndInitializeTargetActor();

	/** @brief 타겟 액터의 타겟팅을 확정하고 데이터를 요청합니다. */
	void FinalizeTargetActor();

private:
	UPROPERTY()
	TSubclassOf<AGameplayAbilityTargetActor> TargetActorClass;

	UPROPERTY()
	TObjectPtr<AGameplayAbilityTargetActor> SpawnedTargetActor;
};
