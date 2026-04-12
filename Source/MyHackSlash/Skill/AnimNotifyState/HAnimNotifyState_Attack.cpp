// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/AnimNotifyState/HAnimNotifyState_Attack.h"
#include "HAnimNotifyState_Attack.h"
#include "Skill/HCombatInterface.h"

void UHAnimNotifyState_Attack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

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

void UHAnimNotifyState_Attack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    if (bExecuteOnce && bHasExecuted) return;

    if (MeshComp && MeshComp->GetOwner())
    {
        if (IHCombatInterface* CombatInterface = Cast<IHCombatInterface>(MeshComp->GetOwner()))
        {
            bHasExecuted = true;
        }
    }
}

void UHAnimNotifyState_Attack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
