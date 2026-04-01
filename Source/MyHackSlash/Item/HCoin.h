// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HCoin.generated.h"

/**
 * @brief 몬스터 처치 시 드랍되는 코인 액터입니다.
 */
UCLASS()
class MYHACKSLASH_API AHCoin : public AActor
{
	GENERATED_BODY()
	
public:	
	AHCoin();

protected:
	virtual void BeginPlay() override;

public:
	/** @brief 지급할 골드 양을 설정합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Item")
	void SetGoldAmount(int32 InAmount) { GoldAmount = InAmount; }

	/** @brief 풀에서 꺼내질 때 초기화하는 함수입니다. */
	void PrepareFromPool(int32 InGoldAmount);

	/** @brief 골드 획득 시 호출되는 함수입니다. */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	/** @brief 풀로 반납합니다. */
	void ReturnToPool();

protected:
	/** @brief 충돌 영역 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "H|Item")
	TObjectPtr<class USphereComponent> SphereComponent;

	/** @brief 시각적 메쉬 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "H|Item")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	/** @brief 회전 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "H|Item")
	TObjectPtr<class URotatingMovementComponent> RotatingMovementComponent;

	/** @brief 튀어오르는 최소/최대 힘 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Item")
	float MinPopForce = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Item")
	float MaxPopForce = 300.0f;

	/** @brief 이 코인이 지급할 골드 양 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Item")
	int32 GoldAmount = 10;

	/** @brief 획득 이펙트 (선택 사항) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Item")
	TObjectPtr<class UNiagaraSystem> PickupEffect;

	/** @brief 획득 사운드 (선택 사항) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Item")
	TObjectPtr<class USoundBase> PickupSound;
};
