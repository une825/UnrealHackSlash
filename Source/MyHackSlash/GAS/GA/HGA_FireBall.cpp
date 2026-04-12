#include "GAS/GA/HGA_FireBall.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UHGA_FireBall::UHGA_FireBall()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	BaseDamage = 20.0f;
	Element = HEElement::Fire;
}

void UHGA_FireBall::ActivateAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, const FGameplayEventData* InTriggerEventData)
{
	Super::ActivateAbility(InHandle, InActorInfo, InActivationInfo, InTriggerEventData);

	if (InActorInfo == nullptr || !InActorInfo->AvatarActor.IsValid())
	{
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, true);
		return;
	}

	AHBaseCharacter* BaseCharacter = Cast<AHBaseCharacter>(InActorInfo->AvatarActor.Get());
	if (nullptr == BaseCharacter)
	{
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, true);
		return;
	}

	// 베이스 클래스의 다중 발사 로직 호출
	SpawnProjectiles();

	const UHUnitProfileData* Profile = BaseCharacter->GetUnitProfileData();
	UAnimMontage* MontageToPlay = (Profile) ? Profile->GetActionMontage(MontageTag) : nullptr;

	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* PlayAttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("PlayFireBall"),
			MontageToPlay,
			BaseCharacter->GetAttackSpeedRate()
		);
		if (PlayAttackTask)
		{
			PlayAttackTask->OnCompleted.AddDynamic(this, &UHGA_FireBall::OnCompleteCallback);
			PlayAttackTask->OnInterrupted.AddDynamic(this, &UHGA_FireBall::OnInterruptedCallback);
			PlayAttackTask->ReadyForActivation();
		}
	}
	else
	{
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, false);
	}
}

void UHGA_FireBall::EndAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, bool bInReplicateEndAbility, bool bInWasCancelled)
{
	Super::EndAbility(InHandle, InActorInfo, InActivationInfo, bInReplicateEndAbility, bInWasCancelled);
}

void UHGA_FireBall::OnCompleteCallback()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHGA_FireBall::OnInterruptedCallback()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
