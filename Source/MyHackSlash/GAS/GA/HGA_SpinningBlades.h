#pragma once

#include "GAS/GA/HGA_ProjectileBase.h"
#include "HGA_SpinningBlades.generated.h"

/**
 * @brief 소환자 주변을 도는 칼날들을 생성하는 스킬 어빌리티
 */
UCLASS()
class MYHACKSLASH_API UHGA_SpinningBlades : public UHGA_ProjectileBase
{
	GENERATED_BODY()

public:
	UHGA_SpinningBlades();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** @brief 투사체 개별 소환 로직 */
	void SpawnSpinningBlade(int32 Index, float AngleStep, const FGameplayEffectSpecHandle& DamageSpecHandle);

	UFUNCTION()
	void OnDelayFinish();

protected:
	/** 공전 반지름 */
	UPROPERTY(EditAnywhere, Category = "H|SpinningBlades")
	float OrbitRadius;

	/** 회전 속도 (도/초) */
	UPROPERTY(EditAnywhere, Category = "H|SpinningBlades")
	float RotationSpeed;

	/** 스킬 지속 시간 */
	UPROPERTY(EditAnywhere, Category = "H|SpinningBlades")
	float Duration;
};
