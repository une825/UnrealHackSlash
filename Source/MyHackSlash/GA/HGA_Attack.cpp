// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/HGA_Attack.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UHGA_Attack::UHGA_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UHGA_Attack::ActivateAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, const FGameplayEventData* InTriggerEventData)
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

	// 공격 이펙트 재생
	if (AttackEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			AttackEffect, 
			BaseCharacter->GetMesh(), 
			TEXT("hand_r"), 
			FVector::ZeroVector, 
			FRotator::ZeroRotator, 
			EAttachLocation::SnapToTarget, 
			true
		);
	}

	const UHUnitProfileData* Profile = BaseCharacter->GetUnitProfileData();
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
		EndAbility(InHandle, InActorInfo, InActivationInfo, true, false);
	}
}

void UHGA_Attack::InputPressed(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo)
{
	Super::InputPressed(InHandle, InActorInfo, InActivationInfo);
}

void UHGA_Attack::CancelAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, bool bInReplicateCancelAbility)
{
	Super::CancelAbility(InHandle, InActorInfo, InActivationInfo, bInReplicateCancelAbility);
}

void UHGA_Attack::EndAbility(const FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilityActivationInfo InActivationInfo, bool bInReplicateEndAbility, bool bInWasCancelled)
{
	Super::EndAbility(InHandle, InActorInfo, InActivationInfo, bInReplicateEndAbility, bInWasCancelled);
}

void UHGA_Attack::OnCompleteCallback()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHGA_Attack::OnInterruptedCallback()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
