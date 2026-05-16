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

	/** @brief 서버에서 플레이어의 코인 획득을 검증하고 처리합니다. */
	bool TryPickup(class AHPlayerCharacter* InPlayer);

	/** @brief 골드 획득 시 호출되는 함수입니다. */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** @brief 풀로 반납합니다. */
	void ReturnToPool();

	void ApplyPickupActiveState();
	void ApplyPickupVisualState(bool bInPickupActive);

	UFUNCTION()
	void OnRep_PickupActive();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetPickupActive(bool bInPickupActive);

protected:
	/** @brief 충돌 영역 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "H|Item")
	TObjectPtr<class USphereComponent> SphereComponent;

	/** @brief 플레이어 획득 판정 전용 영역 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "H|Item")
	TObjectPtr<class USphereComponent> PickupSphereComponent;

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

	/** @brief 클라이언트 요청을 허용할 서버 거리 검증 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Item")
	float PickupValidationDistance = 180.0f;

	UPROPERTY(ReplicatedUsing = OnRep_PickupActive)
	bool bPickupActive = false;

	/** @brief 획득 이펙트 (선택 사항) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Item")
	TObjectPtr<class UNiagaraSystem> PickupEffect;

	/** @brief 획득 사운드 (선택 사항) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Item")
	TObjectPtr<class USoundBase> PickupSound;
};
