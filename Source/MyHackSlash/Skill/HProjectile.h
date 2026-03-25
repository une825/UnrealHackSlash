#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "System/HGlobalTypes.h"
#include "DataAsset/HUnitProfileData.h"
#include "HProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class MYHACKSLASH_API AHProjectile : public AActor
{
	GENERATED_BODY()

public:
	AHProjectile();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	void SetDamage(float InDamageAmount) { DamageAmount = InDamageAmount; }
	void SetElement(HEElement InElement) { Element = InElement; }
	void SetOwningUnitType(EHUnitType InUnitType) { OwningUnitType = InUnitType; }

protected:
	void Explode();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem> FlightEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float DamageAmount = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	HEElement Element = HEElement::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float LifeSpan = 5.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "Stat")
	EHUnitType OwningUnitType = EHUnitType::Monster;
};
