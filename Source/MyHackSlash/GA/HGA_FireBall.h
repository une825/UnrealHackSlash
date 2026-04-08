#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "HGA_FireBall.generated.h"

class AHProjectile;
class UNiagaraSystem;

/**
 * 파이어볼을 발사하는 액티브 스킬 어빌리티입니다.
 */
UCLASS()
class MYHACKSLASH_API UHGA_FireBall : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHGA_FireBall();

protected:
	/** @brief 투사체의 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float ProjectileDamage = 20.0f;

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, const FGameplayEventData* InTriggerEventData) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, bool bInReplicateCancelAbility) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, bool bInReplicateEndAbility, bool bInWasCancelled) override;

protected:
	/** @brief 투사체 발사 로직 */
	void SpawnProjectile();

	UFUNCTION()
	void OnCompleteCallback();
	
	UFUNCTION()
	void OnInterruptedCallback();

protected:
	/** 투사체가 충돌 시 적용할 데미지 이펙트 */
	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<class UGameplayEffect> FireBallDamageEffect;

	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<class AHProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FGameplayTag MontageTag;

	UPROPERTY()
	TObjectPtr<class UHUnitProfileData> CurrentUnitProfileData;
};
