// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/AnimNotifyState/HAnimNotifyState_Attack.h"
#include "HAnimNotifyState_Attack.h"
#include "Skill/HCombatInterface.h"

void UHAnimNotifyState_Attack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	if (MeshComp && MeshComp->GetOwner())
	{
		// 인터페이스를 상속받은 캐릭터인지 확인 후 공격 로직 호출
		if (IHCombatInterface* CombatInterface = Cast<IHCombatInterface>(MeshComp->GetOwner()))
		{
            CombatInterface->ClearAlreadyHitActors();
			CombatInterface->AttackHitCheck();
		}
	}
}

void UHAnimNotifyState_Attack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

    if (bExecuteOnce && bHasExecuted) return;

    if (MeshComp && MeshComp->GetOwner())
    {
        if (IHCombatInterface* CombatInterface = Cast<IHCombatInterface>(MeshComp->GetOwner()))
        {
            //switch (AttackType)
            //{
            //case EAttackType::Melee:
            //    CombatInterface->ExecuteMeleeCheck(AttackData);
            //    break;
            //case EAttackType::Projectile:
            //    CombatInterface->SpawnProjectile(AttackData);
            //    break;
            //case EAttackType::Hitscan:
            //    CombatInterface->ExecuteHitscanCheck(AttackData);
            //    break;
            //}
            bHasExecuted = true;
        }
    }
}

void UHAnimNotifyState_Attack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
}
