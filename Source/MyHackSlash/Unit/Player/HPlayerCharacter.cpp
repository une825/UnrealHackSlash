// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Player/HPlayerCharacter.h"
#include "Unit/Player/HPlayerState.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "DataAsset/HUnitProfileData.h"
#include "DataAsset/HPlayerStatRow.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/GameplayStatics.h>

AHPlayerCharacter::AHPlayerCharacter()
{
	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); 
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; 

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; 
}

void AHPlayerCharacter::InitializeStat(int32 InNewLevel)
{
	Level = InNewLevel;

	if (UnitProfileData && UnitProfileData->UnitType == EUnitType::Player)
	{
		if (FPlayerStatRow* StatRow = UnitProfileData->GetPlayerStatRowByLevel(Level))
		{
			// 공통 스탯 복사
			CurrentStat = *StatRow;
			CurrentPlayerStat = *StatRow;

			// 체력 초기화
			MaxHP = StatRow->MaxHP;
			CurrentHP = MaxHP;

			// 플레이어 특화 스탯 (경험치)
			MaxExp = StatRow->MaxExp;

			GetCharacterMovement()->MaxWalkSpeed = CurrentStat.MovementSpeed;

			OnExpChanged.Broadcast(Level, CurrentExp, MaxExp);
			OnHPChanged.Broadcast(CurrentHP, MaxHP);

			UE_LOG(LogTemp, Warning, TEXT("Player Initialized Level %d (HP: %f, MaxExp: %f)"), Level, MaxHP, MaxExp);
		}
	}
}

void AHPlayerCharacter::AddExp(float InExp)
{
	if (IsDead) return;

	CurrentExp += InExp;
	UE_LOG(LogTemp, Log, TEXT("Player gained %f EXP. (Total: %f / %f)"), InExp, CurrentExp, MaxExp);

	while (CurrentExp >= MaxExp && MaxExp > 0)
	{
		CurrentExp -= MaxExp;
		Level++;
		OnLevelUp();
	}

	OnExpChanged.Broadcast(Level, CurrentExp, MaxExp);
}

void AHPlayerCharacter::OnLevelUp()
{
	InitializeStat(Level);

	// 레벨업 시 체력을 충전한다.
	CurrentHP = MaxHP;
	OnHPChanged.Broadcast(CurrentHP, MaxHP);

	UE_LOG(LogTemp, Warning, TEXT("Player LEVELED UP! Now Level %d"), Level);
}

void AHPlayerCharacter::SetDead()
{
	Super::SetDead();

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, false);
	CameraBoom->AttachToComponent(GetMesh(), AttachmentRules, TEXT("Pelvis"));
	
	// 1. 아주 짧은 정지 (강한 타격감)
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.01f);

	// 2. 잠시 후 슬로우 모션으로 전환 (Timer 사용)
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
}

void AHPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AHPlayerState* GASPlayerState = GetPlayerState<AHPlayerState>();
	if (GASPlayerState)
	{
		AbilitySystemComponent = GASPlayerState->GetAbilitySystemComponent();
		AbilitySystemComponent->InitAbilityActorInfo(GASPlayerState, this);

		for (TSubclassOf<UGameplayAbility> StartAbility : StartAbilities)
		{
			FGameplayAbilitySpec StartSpec(StartAbility);
			AbilitySystemComponent->GiveAbility(StartSpec);
		}

		for (TPair<int32, TSubclassOf<UGameplayAbility>> Pair : StartInputAbilities)
		{
			FGameplayAbilitySpec StartSpec(Pair.Value);
			StartSpec.InputID = Pair.Key;
			AbilitySystemComponent->GiveAbility(StartSpec);
		}

		SetupGASInputComponent();
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(NewController))
	{
		//PlayerController->ConsoleCommand(TEXT("showdebug abilitysystem"));
	}
}

void AHPlayerCharacter::SetupPlayerInputComponent(UInputComponent* InPlayerInputComponent)
{
	Super::SetupPlayerInputComponent(InPlayerInputComponent);
	SetupGASInputComponent();
}

void AHPlayerCharacter::SetupGASInputComponent()
{
	if (IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AHPlayerCharacter::GASInputPressed, 0);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHPlayerCharacter::GASInputReleased, 0);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AHPlayerCharacter::GASInputPressed, 1);
	}
}

void AHPlayerCharacter::GASInputPressed(const int32 InInputID)
{
	if (!AbilitySystemComponent) return;
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromInputID(InInputID);
	if (Spec)
	{
		Spec->InputPressed = true;
		if (Spec->IsActive())
		{
			AbilitySystemComponent->AbilitySpecInputPressed(*Spec);
		}
		else
		{
			AbilitySystemComponent->TryActivateAbility(Spec->Handle);
		}
	}
}

void AHPlayerCharacter::GASInputReleased(const int32 InInputID)
{
	if (!AbilitySystemComponent) return;
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromInputID(InInputID);
	if (Spec)
	{
		Spec->InputPressed = false;
		if (Spec->IsActive())
		{
			AbilitySystemComponent->AbilitySpecInputReleased(*Spec);
		}
	}
}
