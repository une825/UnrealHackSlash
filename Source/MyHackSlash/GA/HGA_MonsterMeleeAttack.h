// Source/MyHackSlash/GA/HGA_MonsterMeleeAttack.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HGA_MonsterMeleeAttack.generated.h"

/**
 * 몬스터의 근접 공격을 담당하는 어빌리티 클래스입니다.
 * 캐릭터가 소유한 공격 몽타주를 재생하고, 공격 속도를 반영합니다.
 */
UCLASS()
class MYHACKSLASH_API UHGA_MonsterMeleeAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHGA_MonsterMeleeAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 몽타주 재생 종료 또는 취소 시 호출될 콜백 함수입니다. */
	UFUNCTION()
	void OnMontageEnded();
};
