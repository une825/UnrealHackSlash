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
#include "System/HSoundManager.h"
#include "Engine/EngineTypes.h"
#include "Mode/MyHackSlashPlayerController.h"
#include "Net/UnrealNetwork.h"

AHCoin::AHCoin()
{
	bReplicates = true;
	SetReplicateMovement(true);

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

	PickupSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphereComponent"));
	PickupSphereComponent->SetupAttachment(RootComponent);
	PickupSphereComponent->SetSphereRadius(60.0f);
	PickupSphereComponent->SetCanEverAffectNavigation(false);
	PickupSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	PickupSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphereComponent->SetGenerateOverlapEvents(true);

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
	PickupSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AHCoin::OnOverlapBegin);
}

void AHCoin::BeginPlay()
{
	Super::BeginPlay();
	ApplyPickupActiveState();
}

void AHCoin::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHCoin, bPickupActive);
}

void AHCoin::PrepareFromPool(int32 InGoldAmount)
{
	if (!HasAuthority()) return;

	GoldAmount = InGoldAmount;
	bPickupActive = true;
	
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	
	// 1. 상태 초기화를 위해 물리/충돌 잠시 끔
	SphereComponent->SetSimulatePhysics(false);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupSphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
	PickupSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ApplyPickupActiveState();

	// 5. 팝콘 효과 적용 (SetPhysicsLinearVelocity는 내부적으로 힘을 가하므로 충돌이 켜져있어야 함)
	FVector PopDirection = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 1.5f);
	PopDirection.Normalize();
	float PopForce = FMath::FRandRange(MinPopForce, MaxPopForce);
	
	SphereComponent->SetPhysicsLinearVelocity(PopDirection * PopForce);
	MulticastSetPickupActive(true);
	ForceNetUpdate();
}

void AHCoin::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(OtherActor))
	{
		if (HasAuthority())
		{
			TryPickup(Player);
			return;
		}

		if (AMyHackSlashPlayerController* PC = Cast<AMyHackSlashPlayerController>(Player->GetController()))
		{
			if (PC->IsLocalController())
			{
				ApplyPickupVisualState(false);
				PC->RequestPickupCoin(this);
			}
		}
	}
}

bool AHCoin::TryPickup(AHPlayerCharacter* InPlayer)
{
	if (!HasAuthority() || !bPickupActive || !InPlayer) return false;

	const float MaxDistanceSq = FMath::Square(PickupValidationDistance);
	if (FVector::DistSquared(InPlayer->GetActorLocation(), GetActorLocation()) > MaxDistanceSq)
	{
		return false;
	}

	AHPlayerState* PS = Cast<AHPlayerState>(InPlayer->GetPlayerState());
	if (!PS) return false;

	bPickupActive = false;
	ApplyPickupActiveState();
	MulticastSetPickupActive(false);

	PS->AddGold(GoldAmount);

	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PickupEffect, GetActorLocation());
	}

	if (PickupSound)
	{
		if (UHSoundManager* SoundManager = GetWorld()->GetSubsystem<UHSoundManager>())
		{
			SoundManager->PlaySoundAtLocationThrottled(PickupSound, GetActorLocation());
		}
	}

	ReturnToPool();
	return true;
}

void AHCoin::ReturnToPool()
{
	if (!HasAuthority()) return;

	// 풀에 들어가기 전에 물리와 충돌을 확실히 끔
	bPickupActive = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (SphereComponent)
	{
		SphereComponent->SetSimulatePhysics(false);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	}
	if (PickupSphereComponent)
	{
		PickupSphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	ApplyPickupActiveState();
	MulticastSetPickupActive(false);
	ForceNetUpdate();

	if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
	{
		PoolManager->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}

void AHCoin::ApplyPickupActiveState()
{
	ApplyPickupVisualState(bPickupActive);
}

void AHCoin::ApplyPickupVisualState(bool bInPickupActive)
{
	SetActorHiddenInGame(!bInPickupActive);
	SetActorEnableCollision(bInPickupActive);

	if (PickupSphereComponent)
	{
		PickupSphereComponent->SetCollisionEnabled(bInPickupActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (SphereComponent && !bInPickupActive)
	{
		SphereComponent->SetSimulatePhysics(false);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	}

	if (MeshComponent)
	{
		MeshComponent->SetVisibility(bInPickupActive, true);
	}
}

void AHCoin::OnRep_PickupActive()
{
	ApplyPickupActiveState();
}

void AHCoin::MulticastSetPickupActive_Implementation(bool bInPickupActive)
{
	bPickupActive = bInPickupActive;
	ApplyPickupActiveState();
}
