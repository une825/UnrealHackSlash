#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HGA_SpinningBlades.generated.h"

/**
 * @brief 소환자 주변을 도는 칼날들을 생성하는 스킬 어빌리티
 */
UCLASS()
class MYHACKSLASH_API UHGA_SpinningBlades : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHGA_SpinningBlades();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 소환할 투사체 클래스 (BP로 지정) */
	UPROPERTY(EditAnywhere, Category = "H|SpinningBlades")
	TSubclassOf<class AHProjectile_SpinningBlade> ProjectileClass;

	/** 적용할 데미지 GE 클래스 */
	UPROPERTY(EditAnywhere, Category = "H|SpinningBlades")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	/** 칼날 개수 */
	UPROPERTY(EditAnywhere, Category = "H|SpinningBlades")
	int32 BladeCount;

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
