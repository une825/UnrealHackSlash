// Fill out your copyright notice in the Description page of Project Settings.


#include "System/HObjectPoolManager.h"
#include "Unit/HBaseCharacter.h"
#include "System/HMonsterAIController.h"
#include "GameFramework/Character.h"
#include "NiagaraActor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

AActor* UHObjectPoolManager::SpawnFromPool(UClass* InActorClass, FVector InLocation, FRotator InRotation)
{
	if (!InActorClass) return nullptr;

	AActor* PooledActor = nullptr;
	if (ObjectPools.Contains(InActorClass) && ObjectPools[InActorClass].InactiveActors.Num() > 0)
	{
		PooledActor = ObjectPools[InActorClass].InactiveActors.Pop();
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PooledActor = GetWorld()->SpawnActor<AActor>(InActorClass, InLocation, InRotation, SpawnParams);
	}

	if (PooledActor)
	{
		ActivateActor(PooledActor, InLocation, InRotation);
	}

	return PooledActor;
}

UNiagaraComponent* UHObjectPoolManager::SpawnNiagaraFromPool(UNiagaraSystem* InNiagaraSystem, FVector InLocation, FRotator InRotation)
{
	if (!InNiagaraSystem) return nullptr;

	FObjectPoolArray& Pool = NiagaraPools.FindOrAdd(InNiagaraSystem);
	ANiagaraActor* NiagaraActor = nullptr;

	if (Pool.InactiveActors.Num() > 0)
	{
		NiagaraActor = Cast<ANiagaraActor>(Pool.InactiveActors.Pop());
		ActivateActor(NiagaraActor, InLocation, InRotation);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		NiagaraActor = GetWorld()->SpawnActor<ANiagaraActor>(ANiagaraActor::StaticClass(), InLocation, InRotation, SpawnParams);
		NiagaraActor->GetNiagaraComponent()->SetAsset(InNiagaraSystem);
	}

	if (NiagaraActor)
	{
		UNiagaraComponent* NiagaraComp = NiagaraActor->GetNiagaraComponent();
		NiagaraComp->Activate(true);

		// 재생이 끝나면 자동으로 풀에 반납하도록 전용 함수 바인딩
		NiagaraComp->OnSystemFinished.Clear();
		NiagaraComp->OnSystemFinished.AddDynamic(this, &UHObjectPoolManager::OnNiagaraFinished);
		
		return NiagaraComp;
	}

	return nullptr;
}

void UHObjectPoolManager::OnNiagaraFinished(UNiagaraComponent* InPSystem)
{
	if (InPSystem)
	{
		// NiagaraComponent의 소유자인 ANiagaraActor를 풀에 반납
		ReturnToPool(InPSystem->GetOwner());
	}
}

void UHObjectPoolManager::ReturnToPool(AActor* InActor)
{
	if (!InActor) return;

	DeactivateActor(InActor);

	// NiagaraActor인 경우 시스템 에셋별로 관리
	if (ANiagaraActor* NiagaraActor = Cast<ANiagaraActor>(InActor))
	{
		UNiagaraSystem* SystemAsset = NiagaraActor->GetNiagaraComponent()->GetAsset();
		if (SystemAsset)
		{
			NiagaraPools.FindOrAdd(SystemAsset).InactiveActors.AddUnique(InActor);
			return;
		}
	}

	UClass* ActorClass = InActor->GetClass();
	ObjectPools.FindOrAdd(ActorClass).InactiveActors.AddUnique(InActor);
}

void UHObjectPoolManager::ActivateActor(AActor* InActor, FVector InLocation, FRotator InRotation)
{
	InActor->SetActorLocationAndRotation(InLocation, InRotation);

	// 1. 기본 액터 활성화
	InActor->SetActorHiddenInGame(false);
	InActor->SetActorEnableCollision(true);
	InActor->SetActorTickEnabled(true);

	// 2. 캐릭터 특화 초기화 (HBaseCharacter인 경우)
	if (AHBaseCharacter* BaseChar = Cast<AHBaseCharacter>(InActor))
	{
		BaseChar->ResetCharacter();
		
		// AI 다시 시작
		if (AHMonsterAIController* AICon = Cast<AHMonsterAIController>(BaseChar->GetController()))
		{
			AICon->RunAI();
		}
	}
}

void UHObjectPoolManager::DeactivateActor(AActor* InActor)
{
	// 1. 캐릭터 특화 비활성화
	if (AHBaseCharacter* BaseChar = Cast<AHBaseCharacter>(InActor))
	{
		// AI 중지
		if (AHMonsterAIController* AICon = Cast<AHMonsterAIController>(BaseChar->GetController()))
		{
			AICon->StopAI();
		}
	}

	// 2. 기본 액터 비활성화
	InActor->SetActorHiddenInGame(true);
	InActor->SetActorEnableCollision(false);
	InActor->SetActorTickEnabled(false);
}
