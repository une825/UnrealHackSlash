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
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("HCapsule"));

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Activate ticking in order to update the cursor every frame.
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

	// 시작 시 설정된 레벨로 스탯 초기화
	InitializeStat(Level);
}

void AHBaseCharacter::InitializeStat(int32 NewLevel)
{
	Level = NewLevel;

	if (UnitProfileData)
	{
		if (FMonsterStatRow* StatRow = UnitProfileData->GetStatRowByLevel(Level))
		{
			CurrentStat = *StatRow;

			// 이동 속도 적용
			GetCharacterMovement()->MaxWalkSpeed = CurrentStat.MovementSpeed;

			UE_LOG(LogTemp, Log, TEXT("%s Level %d Initialized (HP: %f, Attack: %f)"), *GetName(), Level, CurrentStat.MaxHP, CurrentStat.AttackDamage);
		}
	}
}

void AHBaseCharacter::ResetCharacter()
{
	IsDead = false;
	Attackable = true;
	LastDamageCauser = nullptr;

	// 이동 및 물리 상태 복구
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCharacterMovement()->SetComponentTickEnabled(true);
	
	// 충돌 설정 복구
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("HCapsule"));
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block); // 기본적으로 블록

	// 메시 물리 복구
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

void AHBaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

bool AHBaseCharacter::CanJumpInternal_Implementation() const
{
	bool bCanJump = Super::CanJumpInternal_Implementation();
	return bCanJump;
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
		// 애니메이션이 없거나 재생할 수 없는 경우 즉시 공격 종료 처리하여 AI가 멈추지 않게 함
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
	// 상속받은 클래스(Monster 등)에서 AI 태스크 종료 알림 등을 처리
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
