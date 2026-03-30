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
#include <NiagaraFunctionLibrary.h>
#include "NiagaraComponent.h"
#include <System/HObjectPoolManager.h>


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

void AHBaseCharacter::InitializeStat(int32 InNewLevel)
{
	// 기본 클래스에서는 레벨 변수만 갱신합니다.
	// 하위 클래스(Player, Monster)에서 상세 로직을 구현합니다.
	Level = InNewLevel;
}

void AHBaseCharacter::ResetCharacter()
{
	IsDead = false;
	Attackable = true;
	LastDamageCauser = nullptr;

	// 체력 초기화 (중요: 오브젝트 풀링 재활용 대응)
	InitializeStat(Level);

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

#include "System/HUIManager.h"
#include "UI/HDamageTextActor.h"

float AHBaseCharacter::TakeDamage(float InDamageAmount, FDamageEvent const& InDamageEvent, AController* InEventInstigator, AActor* InDamageCauser)
{
	float ActualDamage = InDamageAmount; // 기본적으로 전달받은 데미지부터 시작
	
	if (IsDead) return ActualDamage;

	// --- 치명타 로직 시작 ---
	bool bIsCritical = false;
	AHBaseCharacter* Attacker = nullptr;

	// 1. 공격자(Attacker) 찾기
	if (InEventInstigator)
	{
		Attacker = Cast<AHBaseCharacter>(InEventInstigator->GetPawn());
	}
	else if (InDamageCauser)
	{
		Attacker = Cast<AHBaseCharacter>(InDamageCauser->GetOwner());
		if (!Attacker) Attacker = Cast<AHBaseCharacter>(InDamageCauser);
	}

	// 2. 공격자 스탯에 기반하여 치명타 확률 계산
	if (Attacker)
	{
		float CritChance = Attacker->GetCurrentStat().CriticalRate;
		if (FMath::FRandRange(0.0f, 100.0f) <= CritChance)
		{
			bIsCritical = true;
			ActualDamage *= Attacker->GetCurrentStat().CriticalMultiplier;
		}
	}
	// --- 치명타 로직 끝 ---

	// 실제 엔진 로직은 최종 계산된 데미지로 수행
	ActualDamage = Super::TakeDamage(ActualDamage, InDamageEvent, InEventInstigator, InDamageCauser);

	CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.0f, MaxHP);
	UE_LOG(LogTemp, Log, TEXT("[%s] Received Damage: %.2f (Crit: %s), Current HP: %.2f / %.2f"), *GetName(), ActualDamage, bIsCritical ? TEXT("Yes") : TEXT("No"), CurrentHP, MaxHP);

	// 데미지 텍스트 팝업
	if (ActualDamage > 0.0f)
	{
		bool bShowDamageText = false;
		if (Attacker && Attacker->GetUnitProfileData() && GetUnitProfileData())
		{
			if (Attacker->GetUnitProfileData()->UnitType == EHUnitType::Player &&
				GetUnitProfileData()->UnitType == EHUnitType::Monster)
			{
				bShowDamageText = true;
			}
		}

		if (bShowDamageText)
		{
			if (UHUIManager* UIManager = GetGameInstance()->GetSubsystem<UHUIManager>())
			{
				if (UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>())
				{
					TSubclassOf<AHDamageTextActor> DamageTextClass = UIManager->GetDamageTextActorClass();
					if (DamageTextClass)
					{
						FVector SpawnLocation = GetActorLocation() + FVector(FMath::RandRange(-20.f, 20.f), FMath::RandRange(-20.f, 20.f), 100.f);
						if (AHDamageTextActor* DamageText = Cast<AHDamageTextActor>(Pool->SpawnFromPool(DamageTextClass, SpawnLocation, FRotator::ZeroRotator)))
						{
							DamageText->InitializeDamageText(ActualDamage, bIsCritical);
						}
					}
				}
			}
		}
		LastDamageCauser = Attacker ? Attacker : InDamageCauser;
	}
	else
	{
		// 데미지가 0인 경우에도 마지막 Causer 정보는 남겨둘 수 있음
		LastDamageCauser = InDamageCauser;
	}

	OnHPChanged.Broadcast(CurrentHP, MaxHP);

	PlayHittedEffect();

	// 카메라 쉐이크 재생 (플레이어 캐릭터인 경우)
	if (HitCameraShakeClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			// 데미지에 비례하여 흔들림 강도 계산 (최소 0.1 이상 보장)
			const float ShakeScale = FMath::Max(0.1f, InDamageAmount * DamageToShakeScale);
			PC->ClientStartCameraShake(HitCameraShakeClass, ShakeScale);
		}
	}

	if (CurrentHP <= 0.0f)
	{
		SetDead();
		UE_LOG(LogTemp, Log, TEXT("[%s] Character is Dead: %s"), *GetName(), IsDead ? TEXT("True") : TEXT("False"));
	}

	return ActualDamage;
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

	GetCharacterMovement()->DisableMovement();

	// 래그돌 활성화
	EnableRagdoll();
	SetDeadImpulse();
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
			AHBaseCharacter* TargetCharacter = Cast<AHBaseCharacter>(Hit.GetActor());
			if (TargetCharacter && !TargetCharacter->IsDead && !WasAlreadyHit(TargetCharacter))
			{
				// 동일한 진영(플레이어 vs 플레이어, 몬스터 vs 몬스터)은 공격하지 않음
				if (GetUnitProfileData() && TargetCharacter->GetUnitProfileData())
				{
					if (GetUnitProfileData()->UnitType == TargetCharacter->GetUnitProfileData()->UnitType)
					{
						continue;
					}
				}

				AddHitActor(TargetCharacter);
	
				FDamageEvent DamageEvent;
				TargetCharacter->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
			}
		}
	}

#if ENABLE_DRAW_DEBUG
	//if (UnitProfileData && UnitProfileData->UnitType == EHUnitType::Player)
	//{
	//	FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
	//	float CapsuleHalfHeight = AttackRange * 0.5f;
	//	FColor DrawColor = bHit ? FColor::Green : FColor::Red;
	//	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, HitRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 0.1f);
	//}
#endif
}

void AHBaseCharacter::PlayHittedEffect()
{
	if (HittedBodyEffect)
	{
		if (UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>())
		{
			UNiagaraComponent* NiagaraComp = Pool->SpawnNiagaraFromPool(HittedBodyEffect, FVector::ZeroVector, FRotator::ZeroRotator);
			if (NiagaraComp)
			{
				NiagaraComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("pelvis"));
			}
		}
	}

	if (HittedEffect)
	{
		if (UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>())
		{
			Pool->SpawnNiagaraFromPool(HittedEffect, GetActorLocation(), GetActorRotation());
		}
	}
}

void AHBaseCharacter::EnableRagdoll()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), true, true);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
	}
}

void AHBaseCharacter::SetDeadImpulse()
{
	// 공격자(가해자) 정보를 기반으로 임펄스 적용
	float FinalImpulseForce = 5000.0f;
	FVector ImpulseDir = -GetActorForwardVector();

	if (LastDamageCauser.IsValid())
	{
		if (AHBaseCharacter* Attacker = Cast<AHBaseCharacter>(LastDamageCauser.Get()))
		{
			if (const UHUnitProfileData* AttackerProfile = Attacker->GetUnitProfileData())
			{
				FinalImpulseForce = AttackerProfile->DeathImpulseForce;
			}
		}

		ImpulseDir = GetActorLocation() - LastDamageCauser->GetActorLocation();
		ImpulseDir.Normalize();
	}

	ImpulseDir.Z = 0.5f;
	ImpulseDir.Normalize();
	GetMesh()->AddImpulse(ImpulseDir * FinalImpulseForce, NAME_None, true);
}
