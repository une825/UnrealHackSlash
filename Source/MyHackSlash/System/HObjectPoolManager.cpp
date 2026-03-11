// Fill out your copyright notice in the Description page of Project Settings.


#include "System/HObjectPoolManager.h"
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
		// 풀에 없으면 새로 생성
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

	// 1. 물리 및 렌더링 활성화
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);

	//// 2. GAS 리셋 (중요)
	//if (IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(Actor))
	//{
	//	UAbilitySystemComponent* ASC = ASCHolder->GetAbilitySystemComponent();
	//	if (ASC)
	//	{
	//		// 모든 이펙트 제거 및 태그 정리
	//		ASC->RemoveAllGameplayEffects();
	//		// 스탯(Attribute)은 별도의 초기화 함수를 호출하는 것이 좋습니다.
	//	}
	//}
}

void UHObjectPoolManager::DeactivateActor(AActor* Actor)
{
	// 1. 물리 및 렌더링 비활성화
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);

	// 2. 랙돌 해제 (필요 시)
	if (ACharacter* Char = Cast<ACharacter>(Actor))
	{
		Char->GetMesh()->SetSimulatePhysics(false);
		// 메쉬를 다시 캡슐에 붙여주는 작업 등이 필요할 수 있음
	}
}