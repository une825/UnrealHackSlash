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

	// 자석 감지를 위한 태그 추가
	Tags.Add(TEXT("Item.Currency"));

	// 충돌 설정
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetSphereRadius(25.0f);
	SphereComponent->SetCanEverAffectNavigation(false);
	
	// 물리 및 충돌 프리셋 설정
	SphereComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 기본값 끔
	SphereComponent->SetSimulatePhysics(false); // 기본값 끔
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
	MeshComponent->SetCanEverAffectNavigation(false);

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
	
	// 1. 상태 초기화를 위해 물리/충돌 잠시 끔
	SphereComponent->SetSimulatePhysics(false);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 2. 충돌 설정 완료
	SphereComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	// 3. 속도 초기화
	SphereComponent->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	SphereComponent->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

	// 4. 물리 시뮬레이션 활성화 (충돌이 준비된 상태에서)
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->WakeRigidBody();

	// 5. 팝콘 효과 적용 (SetPhysicsLinearVelocity는 내부적으로 힘을 가하므로 충돌이 켜져있어야 함)
	FVector PopDirection = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 1.5f);
	PopDirection.Normalize();
	float PopForce = FMath::FRandRange(MinPopForce, MaxPopForce);
	
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
	// 풀에 들어가기 전에 물리와 충돌을 확실히 끔
	SetActorHiddenInGame(true);
	if (SphereComponent)
	{
		SphereComponent->SetSimulatePhysics(false);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	}

	if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
	{
		PoolManager->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}
