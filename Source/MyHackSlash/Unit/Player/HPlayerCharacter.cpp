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
#include "System/HUIManager.h"
#include "Skill/HGemInventoryComponent.h"
#include "Skill/HEquipmentComponent.h"
#include "DataAsset/HGemDataAsset.h"
#include "Skill/SkillGem/HMainGem.h"
#include "Mode/MyHackSlashGameMode.h"

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

	// 젬 인벤토리 컴포넌트 생성
	GemInventoryComponent = CreateDefaultSubobject<UHGemInventoryComponent>(TEXT("GemInventoryComponent"));

	// 젬 장착 컴포넌트 생성
	EquipmentComponent = CreateDefaultSubobject<UHEquipmentComponent>(TEXT("EquipmentComponent"));
}

void AHPlayerCharacter::InitializeStat(int32 InNewLevel)
{
	Level = InNewLevel;

	if (UnitProfileData && UnitProfileData->UnitType == EHUnitType::Player)
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

	// 팝업 띄우기
	if (UHUIManager* UIManager = GetGameInstance()->GetSubsystem<UHUIManager>())
	{
		UIManager->ShowWidgetByName(TEXT("SelectAbilityPopupUI"));
	}
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

		// 기본 젬 지급 및 장착 (FistAttack)
		AMyHackSlashGameMode* GameMode = Cast<AMyHackSlashGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode && GameMode->GetGemCollectionDataAsset() && GemInventoryComponent && EquipmentComponent)
		{
			FHGemData FistAttackData;
			//if (GameMode->GetGemCollectionDataAsset()->FindGemData(TEXT("FireBall_T1"), FistAttackData))
			if (GameMode->GetGemCollectionDataAsset()->FindGemData(TEXT("FistAttack_T1"), FistAttackData))
			{
				UHGemBase* NewGem = GemInventoryComponent->AddGem(FistAttackData);
				if (UHMainGem* MainGem = Cast<UHMainGem>(NewGem))
				{
					EquipmentComponent->EquipGem(0, MainGem);
				}
			}
		}
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

		// Jump (InputID 0)
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AHPlayerCharacter::GASInputPressed, 0);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHPlayerCharacter::GASInputReleased, 0);

		// Basic Attack (InputID 1 - Slot 0)
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AHPlayerCharacter::GASInputPressed, 1);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AHPlayerCharacter::GASInputReleased, 1);

		// Additional Skills (InputID 2~ - Slot 1~)
		for (int32 i = 0; i < SkillActions.Num(); ++i)
		{
			if (SkillActions[i])
			{
				int32 InputID = i + 2; // Slot 1 -> InputID 2
				EnhancedInputComponent->BindAction(SkillActions[i], ETriggerEvent::Triggered, this, &AHPlayerCharacter::GASInputPressed, InputID);
				EnhancedInputComponent->BindAction(SkillActions[i], ETriggerEvent::Completed, this, &AHPlayerCharacter::GASInputReleased, InputID);
			}
		}
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
