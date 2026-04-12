#pragma once

#include "GAS/GA/HGA_ProjectileBase.h"
#include "HGA_FireBall.generated.h"

class AHProjectile;

/**
 * 파이어볼을 발사하는 액티브 스킬 어빌리티입니다.
 */
UCLASS()
class MYHACKSLASH_API UHGA_FireBall : public UHGA_ProjectileBase
{
	GENERATED_BODY()

public:
	UHGA_FireBall();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, const FGameplayEventData* InTriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, bool bInReplicateEndAbility, bool bInWasCancelled) override;

protected:
	UFUNCTION()
	void OnCompleteCallback();

	UFUNCTION()
	void OnInterruptedCallback();

protected:
	UPROPERTY()
	TObjectPtr<class UHUnitProfileData> CurrentUnitProfileData;
};
