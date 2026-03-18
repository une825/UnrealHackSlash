// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/HGA_Attack.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

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

	AHBaseCharacter* BaseCharacter = Cast<AHBaseCharacter>(ActorInfo->AvatarActor.Get());
	if (nullptr == BaseCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UHUnitProfileData* Profile = BaseCharacter->GetUnitProfileData();
	if (Profile && Profile->AttackMontage)
	{
		// 유닛 프로필에 설정된 공격 애니메이션 재생
		UAbilityTask_PlayMontageAndWait* PlayAttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, 
			TEXT("PlayAttack"), 
			Profile->AttackMontage, 
			BaseCharacter->GetCurrentStat().AttackSpeedRate
		);

		if (PlayAttackTask)
		{
			PlayAttackTask->OnCompleted.AddDynamic(this, &UHGA_Attack::OnCompleteCallback);
			PlayAttackTask->OnInterrupted.AddDynamic(this, &UHGA_Attack::OnInterruptedCallback);
			PlayAttackTask->ReadyForActivation();
		}
	}
	else
	{
		// 몽타주가 없는 경우 캐릭터의 기본 공격 로직 실행 시도
		BaseCharacter->Attack();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UHGA_Attack::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
}

void UHGA_Attack::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UHGA_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHGA_Attack::OnCompleteCallback()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHGA_Attack::OnInterruptedCallback()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
