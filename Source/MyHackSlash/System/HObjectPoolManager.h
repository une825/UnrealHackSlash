// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HObjectPoolManager.generated.h"

/**
 * 
 */

USTRUCT()
struct FObjectPoolArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AActor*> InactiveActors;
};

UCLASS()
class MYHACKSLASH_API UHObjectPoolManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 풀에서 액터를 가져오거나 새로 생성합니다.
	AActor* SpawnFromPool(UClass* ActorClass, FVector Location, FRotator Rotation);

	// 사용이 끝난 액터를 풀로 반납합니다.
	void ReturnToPool(AActor* Actor);

private:
	UPROPERTY()
	TMap<UClass*, FObjectPoolArray> ObjectPools;

	void DeactivateActor(AActor* Actor);
	void ActivateActor(AActor* Actor, FVector Location, FRotator Rotation);
};
