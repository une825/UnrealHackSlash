#include "Skill/HProjectile_SpinningBlade.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

AHProjectile_SpinningBlade::AHProjectile_SpinningBlade()
	: Radius(200.0f), RotationSpeed(180.0f), CurrentAngle(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;

	// AHProjectile에서 이미 생성된 컴포넌트들을 활용하거나 설정
	if (ProjectileMovement)
	{
		// 공전 투사체이므로 기본 투사체 이동 로직은 비활성화
		ProjectileMovement->Deactivate();
	}
}

void AHProjectile_SpinningBlade::BeginPlay()
{
	Super::BeginPlay();
}

void AHProjectile_SpinningBlade::Initialize(AActor* InOwner, float InRadius, float InSpeed, float InInitialAngle, const FGameplayEffectSpecHandle& InSpecHandle)
{
	CenterActor = InOwner;
	Radius = InRadius;
	RotationSpeed = InSpeed;
	CurrentAngle = InInitialAngle;
	DamageSpecHandle = InSpecHandle;
}

void AHProjectile_SpinningBlade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CenterActor) return;

	// 삼각함수를 활용한 원형 공전 로직
	CurrentAngle += RotationSpeed * DeltaTime;
	float RadAngle = FMath::DegreesToRadians(CurrentAngle);
	
	FVector CenterLoc = CenterActor->GetActorLocation();
	FVector TargetOffset = FVector(FMath::Cos(RadAngle) * Radius, FMath::Sin(RadAngle) * Radius, 0.0f);
	FVector NewLocation = CenterLoc + TargetOffset;

	SetActorLocation(NewLocation);

	// 진행 방향(접선)을 바라보도록 회전 설정
	FRotator NewRotation = TargetOffset.Rotation();
	NewRotation.Yaw += 90.0f; 
	SetActorRotation(NewRotation);
}

void AHProjectile_SpinningBlade::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 부모 클래스의 OnOverlap은 무시하고 GAS 기반의 로직 수행
	if (OtherActor && OtherActor != CenterActor && DamageSpecHandle.IsValid())
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
			
			// 추가적인 히트 처리(이펙트 등)는 블루프린트에서 OnOverlap을 이벤트로 받아 처리 가능
		}
	}
}
