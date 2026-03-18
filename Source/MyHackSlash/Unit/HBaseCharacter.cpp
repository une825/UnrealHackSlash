// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/HBaseCharacter.h"

#include "AbilitySystemComponent.h"
#include "DataAsset/HUnitProfileData.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"


// Sets default values
AHBaseCharacter::AHBaseCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("HCapsule"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = nullptr;
}

void AHBaseCharacter::Attack()
{
	ProcessAttack();
}

void AHBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitializeStat(Level);
}

void AHBaseCharacter::InitializeStat(int32 NewLevel)
{
	// 기본 클래스에서는 레벨 변수만 갱신합니다.
	// 하위 클래스(Player, Monster)에서 상세 로직을 구현합니다.
	Level = NewLevel;
}

void AHBaseCharacter::ResetCharacter()
{
	IsDead = false;
	Attackable = true;
	LastDamageCauser = nullptr;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCharacterMovement()->SetComponentTickEnabled(true);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("HCapsule"));
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);

	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
}

UAbilitySystemComponent* AHBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAnimMontage* AHBaseCharacter::GetAttackMontage() const
{
	return UnitProfileData ? UnitProfileData->AttackMontage : nullptr;
}

float AHBaseCharacter::GetAIPatrolRadius() const
{
	return UnitProfileData ? UnitProfileData->PatrolRadius : 800.0f;
}

float AHBaseCharacter::GetAIDetectRadius() const
{
	return UnitProfileData ? UnitProfileData->DetectRadius : 600.0f;
}

void AHBaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

bool AHBaseCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation();
}

float AHBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	LastDamageCauser = DamageCauser;
	SetDead();
	return DamageAmount;
}

void AHBaseCharacter::ProcessAttack()
{
	if (Attackable && !IsDead)
	{
		Attackable = false;
		AttackBegin();
	}
}

void AHBaseCharacter::AttackBegin()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && UnitProfileData && UnitProfileData->AttackMontage)
	{
		const float AttackSpeedRate = CurrentStat.AttackSpeedRate;
		AnimInstance->Montage_Play(UnitProfileData->AttackMontage, AttackSpeedRate);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AHBaseCharacter::AttackEnd);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, UnitProfileData->AttackMontage);
	}
	else
	{
		AttackEnd(nullptr, false);
	}
}

void AHBaseCharacter::AttackEnd(UAnimMontage* InAnimMontage, bool bInInterrupted)
{
	Attackable = true;
	NotifyAttackEnd();
}

void AHBaseCharacter::NotifyAttackEnd()
{
}

void AHBaseCharacter::UpdateWalkSpeed(const float InNewWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = InNewWalkSpeed;
}

void AHBaseCharacter::SetDead()
{
	if (IsDead) return;

	IsDead = true;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
		if (UnitProfileData && UnitProfileData->DeadMontage)
		{
			AnimInstance->Montage_Play(UnitProfileData->DeadMontage, 1.0f);
		}
	}
}

void AHBaseCharacter::AttackHitCheck()
{
	if (IsDead) return;

	const float AttackRange = CurrentStat.AttackRange;
	const float AttackDamage = CurrentStat.AttackDamage;

	const FVector Start = GetMesh()->GetSocketLocation(WeaponSocketName);
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack));
	Params.AddIgnoredActor(this); 
	
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start, End,
		FQuat::Identity,
		ECC_GameTraceChannel1, 
		FCollisionShape::MakeSphere(HitRadius),
		Params
	);
	
	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* TargetActor = Hit.GetActor();
			if (TargetActor && !WasAlreadyHit(TargetActor))
			{
				AddHitActor(TargetActor);
	
				FDamageEvent DamageEvent;
				TargetActor->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
			}
		}
	}

#if ENABLE_DRAW_DEBUG
	FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;
	FColor DrawColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, HitRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 1.0f);
#endif
}
