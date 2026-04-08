#include "GA/HGA_Attack.h"
#include "Unit/HBaseCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"

UHGA_Attack::UHGA_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UHGA_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AHBaseCharacter* Character = Cast<AHBaseCharacter>(ActorInfo->AvatarActor.Get());
	if (Character == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 캐릭터가 소유한 공격 몽타주 가져오기
	UAnimMontage* AttackMontage = Character->GetAttackMontage();
	if (AttackMontage == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. PlayMontageAndWait 태스크 생성 및 실행
	// 공격 속도 배율도 캐릭터 스탯(CurrentStat)에서 가져오도록 설정
	float AttackSpeedRate = Character->GetCurrentStat().AttackSpeedRate;
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("Attack"),
		AttackMontage,
		AttackSpeedRate,
		NAME_None,
		false,
		1.0f
	);

	if (PlayMontageTask)
	{
		// 몽타주가 완료되거나 취소되었을 때 EndAbility를 호출하도록 바인딩
		PlayMontageTask->OnCompleted.AddDynamic(this, &UHGA_Attack::OnMontageEnded);
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UHGA_Attack::OnMontageEnded);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UHGA_Attack::OnMontageEnded);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UHGA_Attack::OnMontageEnded);

		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UHGA_Attack::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHGA_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
