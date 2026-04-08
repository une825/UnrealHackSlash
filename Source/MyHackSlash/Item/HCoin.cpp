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
	SphereComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->SetEnableGravity(true);
	
	// Pawn과는 겹치도록 설정
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 물리 회전 고정
	SphereComponent->BodyInstance.bLockXRotation = true;
	SphereComponent->BodyInstance.bLockYRotation = true;
	SphereComponent->BodyInstance.bLockZRotation = true;

	// 메쉬 설정
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 회전 설정
	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovementComponent->SetUpdatedComponent(MeshComponent);
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
	
	// 1. 물리 시뮬레이션 일시 중단 (상태 변경을 위해)
	SphereComponent->SetSimulatePhysics(false);

	// 2. 충돌 설정 및 상태 초기화
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	SphereComponent->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	SphereComponent->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

	// 3. 물리 시뮬레이션 다시 활성화
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->WakeRigidBody();

	// 4. 팝콘처럼 튀어오르게 하기 (랜덤 방향)
	FVector PopDirection = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 1.5f);
	PopDirection.Normalize();
	float PopForce = FMath::FRandRange(MinPopForce, MaxPopForce);
	
	// 물리 엔진의 안정성을 위해 Velocity를 직접 수정
	SphereComponent->SetPhysicsLinearVelocity(PopDirection * PopForce);
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
