// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "HBreakableActor.generated.h"

class UAbilitySystemComponent;
class UHCharacterAttributeSet;

/**
 * @brief 공격으로 부술 수 있고 보상을 드랍하는 액터 클래스입니다. (GAS 기반)
 */
UCLASS()
class MYHACKSLASH_API AHBreakableActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	AHBreakableActor();

	// IAbilitySystemInterface 구현
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;

	/** @brief 사망(파괴) 태그가 변경될 때 호출됩니다. */
	virtual void OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

public:
	/** @brief 풀에서 활성화될 때 초기화하는 함수입니다. */
	void PrepareFromPool();

protected:
	/** @brief 파괴 시 호출되는 로직입니다. */
	virtual void OnBreak();

	/** @brief 풀로 반납합니다. */
	void ReturnToPool();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UHCharacterAttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<class UGameplayEffect> InitStatEffect;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "H|Component")
	TObjectPtr<class UBoxComponent> BoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "H|Component")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	/** @brief 파괴 시 드랍할 코인 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Reward")
	TSubclassOf<class AHCoin> CoinClass;

	/** @brief 드랍할 골드 양 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Reward")
	int32 GoldAmount = 20;

	/** @brief 파괴 시 이펙트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Effect")
	TObjectPtr<class UNiagaraSystem> BreakEffect;

	/** @brief 파괴 시 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Effect")
	TObjectPtr<class USoundBase> BreakSound;

	bool bIsBroken = false;
};
