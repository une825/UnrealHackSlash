// Fill out your copyright notice in the Description page of Project Settings.


#include "System/HObjectPoolManager.h"
#include "Unit/HBaseCharacter.h"
#include "System/HMonsterAIController.h"
#include "GameFramework/Character.h"

AActor* UHObjectPoolManager::SpawnFromPool(UClass* ActorClass, FVector Location, FRotator Rotation)
{
	if (!ActorClass) return nullptr;

	AActor* PooledActor = nullptr;
	if (ObjectPools.Contains(ActorClass) && ObjectPools[ActorClass].InactiveActors.Num() > 0)
	{
		PooledActor = ObjectPools[ActorClass].InactiveActors.Pop();
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PooledActor = GetWorld()->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
	}

	if (PooledActor)
	{
		ActivateActor(PooledActor, Location, Rotation);
	}

	return PooledActor;
}

void UHObjectPoolManager::ReturnToPool(AActor* Actor)
{
	if (!Actor) return;

	UClass* ActorClass = Actor->GetClass();
	DeactivateActor(Actor);
	ObjectPools.FindOrAdd(ActorClass).InactiveActors.Add(Actor);
}

void UHObjectPoolManager::ActivateActor(AActor* Actor, FVector Location, FRotator Rotation)
{
	Actor->SetActorLocationAndRotation(Location, Rotation);

	// 1. 기본 액터 활성화
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);

	// 2. 캐릭터 특화 초기화 (HBaseCharacter인 경우)
	if (AHBaseCharacter* BaseChar = Cast<AHBaseCharacter>(Actor))
	{
		BaseChar->ResetCharacter();
		
		// AI 다시 시작
		if (AHMonsterAIController* AICon = Cast<AHMonsterAIController>(BaseChar->GetController()))
		{
			AICon->RunAI();
		}
	}
}

void UHObjectPoolManager::DeactivateActor(AActor* Actor)
{
	// 1. 캐릭터 특화 비활성화
	if (AHBaseCharacter* BaseChar = Cast<AHBaseCharacter>(Actor))
	{
		// AI 중지
		if (AHMonsterAIController* AICon = Cast<AHMonsterAIController>(BaseChar->GetController()))
		{
			AICon->StopAI();
		}
	}

	// 2. 기본 액터 비활성화
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);
}
