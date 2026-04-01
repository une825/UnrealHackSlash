// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/HCoin.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Unit/Player/HPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "System/HObjectPoolManager.h"
#include "Engine/EngineTypes.h"

AHCoin::AHCoin()
{
	PrimaryActorTick.bCanEverTick = false;

	// 충돌 설정
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetSphereRadius(25.0f);
	
	// 물리 및 충돌 프리셋 설정
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->SetEnableGravity(true);
	SphereComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	// Pawn과는 겹치도록 설정
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 물리 회전 고정 (모든 축의 회전을 물리 엔진이 계산하지 않도록 잠금)
	SphereComponent->BodyInstance.bLockXRotation = true;
	SphereComponent->BodyInstance.bLockYRotation = true;
	SphereComponent->BodyInstance.bLockZRotation = true;

	// 메쉬 설정
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 회전 설정
	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	// 회전 대상을 액터 루트가 아닌 메쉬 컴포넌트로 지정
	RotatingMovementComponent->SetUpdatedComponent(MeshComponent);
	// Z축(Yaw)으로 초당 180도 회전하도록 설정 (팽이처럼 도는 효과)
	RotatingMovementComponent->RotationRate = FRotator(0.0f, 180.0f, 0.0f); 

	// 델리게이트 바인딩
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AHCoin::OnOverlapBegin);
}

void AHCoin::BeginPlay()
{
	Super::BeginPlay();
}

void AHCoin::PrepareFromPool(int32 InGoldAmount)
{
	GoldAmount = InGoldAmount;
	
	SetActorHiddenInGame(false);
	
	// 충돌 설정 강제 적용 (SetActorEnableCollision은 액터 수준이므로 컴포넌트 수준에서 다시 보장)
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 물리 시뮬레이션 활성화 보장
	if (!SphereComponent->IsSimulatingPhysics())
	{
		SphereComponent->SetSimulatePhysics(true);
	}

	// 물리 상태 초기화
	SphereComponent->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	SphereComponent->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	
	// 물리 엔진 강제 깨우기
	SphereComponent->WakeRigidBody();

	// 팝콘처럼 튀어오르게 하기 (랜덤 방향)
	FVector PopDirection = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 1.5f);
	PopDirection.Normalize();
	float PopForce = FMath::FRandRange(MinPopForce, MaxPopForce);
	
	// AddImpulse 대신 직접 속도(Velocity)를 설정하여 'CollisionEnabled' 체크 우회 및 안정성 확보
	SphereComponent->SetAllPhysicsLinearVelocity(PopDirection * PopForce);
}

void AHCoin::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(OtherActor))
	{
		if (AHPlayerState* PS = Cast<AHPlayerState>(Player->GetPlayerState()))
		{
			PS->AddGold(GoldAmount);

			// 시각적/청각적 피드백
			if (PickupEffect)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PickupEffect, GetActorLocation());
			}

			if (PickupSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
			}

			ReturnToPool();
		}
	}
}

void AHCoin::ReturnToPool()
{
	if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
	{
		PoolManager->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}
