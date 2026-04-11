// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit/HBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "DataAsset/HUnitProfileData.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "System/HObjectPoolManager.h"
#include "System/HUIManager.h"
#include "System/HSoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "UI/HDamageTextActor.h"
#include "UI/MainHud/HMainHudUI.h"
#include "Item/HBreakableActor.h"
#include "Attribute/HCharacterAttributeSet.h"

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

	AttackAbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.FistAttack"));
	DeadTag = FGameplayTag::RequestGameplayTag(TEXT("Character.State.IsDead"));
}

void AHBaseCharacter::Attack()
{
	if (IsDead) return;

	if (AbilitySystemComponent && AttackAbilityTag.IsValid())
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(AttackAbilityTag));
	}
}

void AHBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 레벨로 스탯 초기화 (Level 변수가 제거되었으므로 기본값 1 또는 외부 설정값 사용)
	InitializeStat(GetLevel() > 0 ? GetLevel() : 1);

	if (AbilitySystemComponent && DeadTag.IsValid())
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(DeadTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AHBaseCharacter::OnDeadTagChanged);
	}
}

void AHBaseCharacter::OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0 && !IsDead)
	{
		SetDead();
	}
}

int32 AHBaseCharacter::GetLevel() const
{
	if (AttributeSet)
	{
		return static_cast<int32>(AttributeSet->GetLevel());
	}
	return 1;
}

float AHBaseCharacter::GetCurrentHP() const
{
	if (AttributeSet)
	{
		return AttributeSet->GetHealth();
	}
	return 0.0f;
}

float AHBaseCharacter::GetMaxHP() const
{
	if (AttributeSet)
	{
		return AttributeSet->GetMaxHealth();
	}
	return 0.0f;
}

float AHBaseCharacter::GetAttackDamage() const
{
	return AttributeSet ? AttributeSet->GetAttackDamage() : 0.0f;
}

float AHBaseCharacter::GetAttackSpeedRate() const
{
	return AttributeSet ? AttributeSet->GetAttackSpeedRate() : 1.0f;
}

float AHBaseCharacter::GetAttackRange() const
{
	return AttributeSet ? AttributeSet->GetAttackRange() : 0.0f;
}

float AHBaseCharacter::GetCriticalRate() const
{
	return AttributeSet ? AttributeSet->GetCriticalRate() : 0.0f;
}

float AHBaseCharacter::GetCriticalMultiplier() const
{
	return AttributeSet ? AttributeSet->GetCriticalMultiplier() : 1.5f;
}

float AHBaseCharacter::GetMovementSpeed() const
{
	return AttributeSet ? AttributeSet->GetMovementSpeed() : 0.0f;
}

void AHBaseCharacter::InitializeStat(int32 InNewLevel)
{
	if (AbilitySystemComponent && InitStatEffect)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddInstigator(this, this);

		// GE의 Level 파라미터로 InNewLevel을 넘깁니다. 
		// GE 내부에서 이 Level 값을 기반으로 DataTable의 스탯을 참조하게 됩니다.
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitStatEffect, static_cast<float>(InNewLevel), EffectContext);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void AHBaseCharacter::ResetCharacter()
{
	IsDead = false;
	Attackable = true;
	LastDamageCauser = nullptr;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(DeadTag);
		FGameplayTagContainer AllCharacterTags;
		AllCharacterTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Character")));
		AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(AllCharacterTags);
	}

	InitializeStat(GetLevel());

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCharacterMovement()->SetComponentTickEnabled(true);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("HCapsule"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);

	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 충돌 명시적으로 복구
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
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

	if (AbilitySystemComponent)
	{
		BindAttributeCallbacks();
	}
}

void AHBaseCharacter::BindAttributeCallbacks()
{
	if (AbilitySystemComponent)
	{
		// Health Attribute 변경 시 OnHealthAttributeChanged 호출되도록 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHCharacterAttributeSet::GetHealthAttribute())
			.AddUObject(this, &AHBaseCharacter::OnHealthAttributeChanged);

		// MaxHealth도 동일하게 감지하여 델리게이트 호출
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHCharacterAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &AHBaseCharacter::OnHealthAttributeChanged);

		// MovementSpeed 변경 감지
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHCharacterAttributeSet::GetMovementSpeedAttribute())
			.AddUObject(this, &AHBaseCharacter::OnMovementSpeedAttributeChanged);
	}
}

void AHBaseCharacter::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	// HP 변경 델리게이트 브로드캐스트 (UI 등에서 사용)
	OnHPChanged.Broadcast(GetCurrentHP(), GetMaxHP());
}

void AHBaseCharacter::OnMovementSpeedAttributeChanged(const FOnAttributeChangeData& Data)
{
	UpdateWalkSpeed(Data.NewValue);
}

bool AHBaseCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation();
}

float AHBaseCharacter::TakeDamage(float InDamageAmount, FDamageEvent const& InDamageEvent, AController* InEventInstigator, AActor* InDamageCauser)
{
	if (IsDead) return 0.0f;
	return Super::TakeDamage(InDamageAmount, InDamageEvent, InEventInstigator, InDamageCauser);
}

void AHBaseCharacter::ShowDamageText(float InActualDamage, bool bInIsCritical, AActor* InDamageCauser)
{
	if (InActualDamage <= 0.0f) return;

	AActor* ActualCauser = InDamageCauser;
	if (APlayerState* PS = Cast<APlayerState>(InDamageCauser))
	{
		ActualCauser = PS->GetPawn();
	}

	AHBaseCharacter* Attacker = Cast<AHBaseCharacter>(ActualCauser);
	if (Attacker && Attacker->GetUnitProfileData() && GetUnitProfileData())
	{
		if (Attacker->GetUnitProfileData()->UnitType == EHUnitType::Player &&
			GetUnitProfileData()->UnitType == EHUnitType::Monster)
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
							DamageText->InitializeDamageText(InActualDamage, bInIsCritical);
						}
					}
				}
			}
		}
	}
}

void AHBaseCharacter::HandleHUDDamageEffect()
{
	if (UnitProfileData && UnitProfileData->UnitType == EHUnitType::Player)
	{
		if (UHUIManager* UIManager = GetGameInstance()->GetSubsystem<UHUIManager>())
		{
			if (UHMainHudUI* MainHud = UIManager->GetWidget<UHMainHudUI>())
			{
				MainHud->PlayDamageEffectAnim();
			}
		}
	}
}

void AHBaseCharacter::HandleCameraShake(float InDamageAmount)
{
	if (HitCameraShakeClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			const float ShakeScale = FMath::Max(0.1f, InDamageAmount * DamageToShakeScale);
			PC->ClientStartCameraShake(HitCameraShakeClass, ShakeScale);
		}
	}
}

void AHBaseCharacter::HandleHitSound()
{
	if (UnitProfileData && UnitProfileData->HitSounds.Num() > 0)
	{
		const int32 RandomIndex = FMath::RandRange(0, UnitProfileData->HitSounds.Num() - 1);
		if (USoundBase* SelectedSound = UnitProfileData->HitSounds[RandomIndex])
		{
			if (UHSoundManager* SoundManager = GetWorld()->GetSubsystem<UHSoundManager>())
			{
				SoundManager->PlaySoundAtLocationThrottled(SelectedSound, GetActorLocation());
			}
			else
			{
				UGameplayStatics::PlaySoundAtLocation(this, SelectedSound, GetActorLocation());
			}
		}
	}
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
		const float AttackSpeedRate = GetAttackSpeedRate();
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
	EnableRagdoll();
	SetDeadImpulse();
}

void AHBaseCharacter::AttackHitCheck()
{
	if (IsDead) return;

	if (AbilitySystemComponent)
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;
		Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Action.AttackHitCheck"));
		AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
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
	// 캡슐 컴포넌트 내비게이션 및 충돌 완전 제거
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCanEverAffectNavigation(false); // 내비게이션 영향 제거

	// 무브먼트 컴포넌트 정지 및 회피 비활성화
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetAvoidanceEnabled(false); // RVO Avoidance 비활성화
	GetCharacterMovement()->SetCanEverAffectNavigation(false); // 무브먼트 내비 영향 제거

	// 메쉬 설정
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	// 프로필 설정 후에 명시적으로 Pawn 채널을 Ignore 해야 다른 AI들이 통과함
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCanEverAffectNavigation(false); // 메쉬 내비 영향 제거
	
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
	float FinalImpulseForce = 5000.0f;
	FVector ImpulseDir = -GetActorForwardVector();

	if (LastDamageCauser.IsValid())
	{
		AActor* ActualCauser = LastDamageCauser.Get();
		if (APlayerState* PS = Cast<APlayerState>(LastDamageCauser))
		{
			ActualCauser = PS->GetPawn();
		}

		if (AHBaseCharacter* Attacker = Cast<AHBaseCharacter>(ActualCauser))
		{
			if (const UHUnitProfileData* AttackerProfile = Attacker->GetUnitProfileData())
			{
				FinalImpulseForce = AttackerProfile->DeathImpulseForce;
			}
		}

		ImpulseDir = GetActorLocation() - ActualCauser->GetActorLocation();
		ImpulseDir.Normalize();
	}

	ImpulseDir.Z = 0.5f;
	ImpulseDir.Normalize();
	GetMesh()->AddImpulse(ImpulseDir * FinalImpulseForce, NAME_None, true);
}
