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
#include "System/HInfiniteMapManager.h"

#include "NavigationInvokerComponent.h"
#include <System/HSoundManager.h>

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

	// 네비게이션 인보커 컴포넌트 생성 및 설정
	NavInvokerComponent = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvokerComponent"));
	// 플레이어 주변 3000유닛(타일 1.5개 분량) 정도를 굽고, 5000유닛 밖은 제거
	NavInvokerComponent->SetGenerationRadii(3000.0f, 5000.0f);

	PrimaryActorTick.bCanEverTick = true;
}

void AHPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AHPlayerCharacter::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);

	// 무한 맵 업데이트
	if (UHInfiniteMapManager* MapManager = GetWorld()->GetSubsystem<UHInfiniteMapManager>())
	{
		MapManager->UpdateMap(GetActorLocation());
	}
}

void AHPlayerCharacter::InitializeStat(int32 InNewLevel)
{
	Super::InitializeStat(InNewLevel);

	if (UnitProfileData && UnitProfileData->UnitType == EHUnitType::Player && AttributeSet)
	{
		if (FPlayerStatRow* StatRow = UnitProfileData->GetPlayerStatRowByLevel(Level))
		{
			// 플레이어 특화 스탯 복사
			CurrentPlayerStat = *StatRow;

			// 다음 레벨에 필요한 경험치 설정 (AttributeSet 사용)
			AttributeSet->SetMaxExperience(StatRow->MaxExp);

			GetCharacterMovement()->MaxWalkSpeed = CurrentPlayerStat.MovementSpeed;

			OnExpChanged.Broadcast(Level, AttributeSet->GetExperience(), AttributeSet->GetMaxExperience());
			OnHPChanged.Broadcast(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());

			UE_LOG(LogTemp, Warning, TEXT("Player Initialized Level %d (HP: %f, MaxExp: %f)"), 
				Level, AttributeSet->GetMaxHealth(), AttributeSet->GetMaxExperience());
		}
	}
}

void AHPlayerCharacter::AddExp(float InExp)
{
	if (IsDead || !AbilitySystemComponent || !AttributeSet) return;

	// GAS 방식으로 경험치 증가
	float NewExp = AttributeSet->GetExperience() + InExp;
	AttributeSet->SetExperience(NewExp);

	// 레벨업 로직은 AttributeSet::PostGameplayEffectExecute에서 처리됨
	
	// UI 업데이트를 위한 델리게이트 호출
	OnExpChanged.Broadcast(GetLevel(), AttributeSet->GetExperience(), AttributeSet->GetMaxExperience());

	UE_LOG(LogTemp, Log, TEXT("Player gained %f EXP via GAS. (Total: %f / %f)"), 
		InExp, AttributeSet->GetExperience(), AttributeSet->GetMaxExperience());
}

float AHPlayerCharacter::GetCurrentExp() const
{
	if (AttributeSet)
	{
		return AttributeSet->GetExperience();
	}
	return 0.0f;
}

float AHPlayerCharacter::GetMaxExp() const
{
	if (AttributeSet)
	{
		return AttributeSet->GetMaxExperience();
	}
	return 0.0f;
}

void AHPlayerCharacter::OnLevelUp()
{
	if (AttributeSet)
	{
		InitializeStat(AttributeSet->GetLevel());
		
		// 레벨업 시 체력을 충전한다.
		AttributeSet->SetHealth(AttributeSet->GetMaxHealth());
		OnHPChanged.Broadcast(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
	}

	UE_LOG(LogTemp, Warning, TEXT("Player LEVELED UP! Now Level %d"), Level);

	if (UHSoundManager* SoundManager = GetWorld()->GetSubsystem<UHSoundManager>())
	{
		SoundManager->PlaySFXByKey(TEXT("LevelUpSound"), Owner->GetActorLocation(), 1.0, true);
	}

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

	// BGM 속도도 함께 느리게 조절
	if (UHSoundManager* SoundManager = GetWorld()->GetSubsystem<UHSoundManager>())
	{
		SoundManager->SetBGMPitch(0.4f); // 0.2보다는 조금 덜 낮춰서 너무 늘어지지 않게 함
	}
}

void AHPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AHPlayerState* GASPlayerState = GetPlayerState<AHPlayerState>();
	if (GASPlayerState)
	{
		AbilitySystemComponent = GASPlayerState->GetAbilitySystemComponent();
		AbilitySystemComponent->InitAbilityActorInfo(GASPlayerState, this);

		// AttributeSet 설정 (부모 클래스의 멤버 변수)
		// PlayerState에서 직접 가져오기 위한 Getter 함수를 추가하거나, 
		// HPlayerState.h에서 AttributeSet이 TObjectPtr임을 고려하여 가져옴
		// 여기서는 IAbilitySystemInterface를 통해 ASC가 관리하는 AttributeSet을 찾는 방식 사용 가능
		AttributeSet = const_cast<UHCharacterAttributeSet*>(AbilitySystemComponent->GetSet<UHCharacterAttributeSet>());

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
