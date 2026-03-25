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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	struct FGameplayTag MontageTag;

	/** @brief 발사할 투사체 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AHProjectile> ProjectileClass;

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
	UPROPERTY()
	TObjectPtr<class UHUnitProfileData> CurrentUnitProfileData;
};
