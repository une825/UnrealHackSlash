#pragma once

#include "CoreMinimal.h"
#include "Skill/HProjectile.h"
#include "GameplayEffectTypes.h"
#include "HProjectile_SpinningBlade.generated.h"

/**
 * @brief AHProjectile을 상속받아 캐릭터 주변을 공전하는 특수 투사체 클래스
 */
UCLASS()
class MYHACKSLASH_API AHProjectile_SpinningBlade : public AHProjectile
{
	GENERATED_BODY()
	
public:	
	AHProjectile_SpinningBlade();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/** 
	 * @brief 공전 투사체 초기화
	 * @param InOwner 공전의 중심이 될 액터
	 * @param InRadius 공전 반지름
	 * @param InSpeed 공전 속도 (도/초)
	 * @param InInitialAngle 시작 각도
	 * @param InSpecHandle 적용할 데미지 이펙트 스펙
	 /** 공전 투사체 초기화 */
	 void Initialize(AActor* InOwner, float InRadius, float InSpeed, float InInitialAngle, const FGameplayEffectSpecHandle& InSpecHandle);

	 /** @brief 풀에서 꺼낼 때 호출하여 상태를 초기화합니다. (공전 전용으로 오버라이드) */
	 void ResetProjectile(FVector InLocation, FRotator InRotation);

	 protected:
	/** 공전 중심 액터 */
	UPROPERTY()
	TObjectPtr<AActor> CenterActor;

	float Radius;
	float RotationSpeed;
	float CurrentAngle;

	/** 데미지 계산을 위한 SpecHandle (AHProjectile의 DamageAmount 대신 GAS 시스템 활용) */
	FGameplayEffectSpecHandle DamageSpecHandle;

	/** AHProjectile의 OnOverlap을 오버라이드하여 GAS 기반 데미지 적용 */
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
