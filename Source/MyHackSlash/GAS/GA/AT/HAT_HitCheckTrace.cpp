// Source/MyHackSlash/GA/AT/HAT_HitCheckTrace.cpp

#include "GAS/GA/AT/HAT_HitCheckTrace.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include <GAS/GA/TA/HTA_HitCheckTrace.h>

UHAT_HitCheckTrace::UHAT_HitCheckTrace()
{
}

UHAT_HitCheckTrace* UHAT_HitCheckTrace::CreateTask(UGameplayAbility* OwningAbility, TSubclassOf<AHTA_HitCheckTrace> InTargetActorClass)
{
	UHAT_HitCheckTrace* MyObj = NewAbilityTask<UHAT_HitCheckTrace>(OwningAbility);
	MyObj->TargetActorClass = InTargetActorClass;
	return MyObj;
}

void UHAT_HitCheckTrace::Activate()
{
	Super::Activate();

	SpawnAndInitializeTargetActor();
	FinalizeTargetActor();

	SetWaitingOnAvatar();
}

bool UHAT_HitCheckTrace::SpawnAndInitializeTargetActor()
{
	SpawnedTargetActor = Cast<AHTA_HitCheckTrace>(Ability->GetWorld()->SpawnActorDeferred<AGameplayAbilityTargetActor>(TargetActorClass, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	if (SpawnedTargetActor)
	{
		SpawnedTargetActor->TargetDataReadyDelegate.AddUObject(this, &UHAT_HitCheckTrace::OnTargetDataReadyCallback);
	}

	//SpawnedTargetActor = GetWorld()->SpawnActor<AGameplayAbilityTargetActor>(TargetActorClass);
	//if (SpawnedTargetActor)
	//{
	//	SpawnedTargetActor->SetOwner(GetAvatarActor());
	//	
	//	// 데이터 준비 완료 델리게이트 바인딩
	//	SpawnedTargetActor->TargetDataReadyDelegate.AddUObject(this, &UHAT_HitCheckTrace::OnTargetDataReadyCallback);
	//	
	//	// 타겟팅 시작
	//	SpawnedTargetActor->StartTargeting(Ability);
	//	return true;
	//}

	return false;
}

void UHAT_HitCheckTrace::FinalizeTargetActor()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC)
	{
		const FTransform SpawnTransform = ASC->GetAvatarActor()->GetTransform();
		SpawnedTargetActor->FinishSpawning(SpawnTransform);

		ASC->SpawnedTargetActors.Push(SpawnedTargetActor);
		SpawnedTargetActor->StartTargeting(Ability);
		SpawnedTargetActor->ConfirmTargeting();
	}

	//if (SpawnedTargetActor && SpawnedTargetActor->ShouldProduceTargetData())
	//{
	//	SpawnedTargetActor->ConfirmTargetingAndContinue();
	//}
}

void UHAT_HitCheckTrace::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& DataHandle)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTargetDataReady.Broadcast(DataHandle);
	}
	
	EndTask();
}

void UHAT_HitCheckTrace::OnDestroy(bool bInOwnerFinished)
{
	if (SpawnedTargetActor)
	{
		SpawnedTargetActor->Destroy();
	}

	Super::OnDestroy(bInOwnerFinished);
}
