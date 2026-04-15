#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HMagnetItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UGameplayEffect;

/**
 * 획득 시 플레이어에게 자력을 부여하는 아이템입니다.
 */
UCLASS()
class MYHACKSLASH_API AHMagnetItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AHMagnetItem();

	/** @brief 풀에서 활성화될 때 초기화하는 함수입니다. */
	void PrepareFromPool();

	/** @brief 풀로 반납합니다. */
	void ReturnToPool();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class URotatingMovementComponent> RotatingMovementComponent;

	/** @brief 플레이어에게 적용할 자력 Gameplay Effect */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	TSubclassOf<UGameplayEffect> MagnetEffectClass;

	/** @brief 획득 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	TObjectPtr<USoundBase> PickupSound;

	/** @brief 최소 팝콘 힘 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float MinPopForce = 300.0f;

	/** @brief 최대 팝콘 힘 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float MaxPopForce = 600.0f;

	/** @brief 메시의 시각적 높이 오프셋 (땅에 파묻히는 것 방지) */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	FVector MeshVisualOffset = FVector(0.0f, 0.0f, 0.0f);
};
