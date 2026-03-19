// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HObjectPoolManager.generated.h"

/**
 * 
 */

class UNiagaraSystem;

USTRUCT()
struct FObjectPoolArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> InactiveActors;
};

UCLASS()
class MYHACKSLASH_API UHObjectPoolManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 풀에서 액터를 가져오거나 생성하여 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	AActor* SpawnFromPool(UClass* InActorClass, FVector InLocation, FRotator InRotation);

	// Niagara 이펙트를 풀에서 가져와 재생합니다.
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	class UNiagaraComponent* SpawnNiagaraFromPool(UNiagaraSystem* InNiagaraSystem, FVector InLocation, FRotator InRotation);
// 사용이 끝난 액터를 풀에 반납합니다.
UFUNCTION(BlueprintCallable, Category = "ObjectPool")
void ReturnToPool(AActor* InActor);

private:
UFUNCTION()
void OnNiagaraFinished(class UNiagaraComponent* InPSystem);

private:
UPROPERTY()

	TMap<UClass*, FObjectPoolArray> ObjectPools;

	// Niagara 전용 풀 (System별로 관리)
	UPROPERTY()
	TMap<TObjectPtr<UNiagaraSystem>, FObjectPoolArray> NiagaraPools;

	void DeactivateActor(AActor* InActor);
	void ActivateActor(AActor* InActor, FVector InLocation, FRotator InRotation);
};

