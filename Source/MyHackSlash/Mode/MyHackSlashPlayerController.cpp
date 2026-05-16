// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mode/MyHackSlashPlayerController.h"
#include "AbilitySystemComponent.h"
#include "Attribute/HCharacterAttributeSet.h"
#include "DataAsset/HGemDataAsset.h"
#include "DataAsset/HShopRow.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "System/HUIManager.h"
#include "Engine/LocalPlayer.h"
#include "Mode/MyHackSlashGameMode.h"
#include "Skill/HGemInventoryComponent.h"
#include "System/HSelectAbilityManager.h"
#include "System/HWaveManager.h"
#include "System/HUIManager.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Unit/Player/HPlayerState.h"
#include "UI/HWaveResultUI.h"
#include "Item/HCoin.h"
#include <Unit/HBaseCharacter.h>
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMyHackSlashPlayerController::AMyHackSlashPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}

void AMyHackSlashPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyHackSlashPlayerController, SelectAbilityOptions);
	DOREPLIFETIME(AMyHackSlashPlayerController, SelectAbilityRefreshCount);
}

void AMyHackSlashPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	// 인게임 입력 모드 설정 (인트로의 UIOnly 설정을 해제하기 위함)
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	// 마우스 커서 표시 보장
	bShowMouseCursor = true;

	// test
	if (UHUIManager* UIMgr = GetGameInstance()->GetSubsystem<UHUIManager>())
	{
		UIMgr->ShowWidgetByName(TEXT("MainHUD"));
	}
}

void AMyHackSlashPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(DestinationClickAction, ETriggerEvent::Started, this, &AMyHackSlashPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(DestinationClickAction, ETriggerEvent::Triggered, this, &AMyHackSlashPlayerController::OnDestinationTriggered);
		EnhancedInputComponent->BindAction(DestinationClickAction, ETriggerEvent::Completed, this, &AMyHackSlashPlayerController::OnDestinationReleased);
		EnhancedInputComponent->BindAction(DestinationClickAction, ETriggerEvent::Canceled, this, &AMyHackSlashPlayerController::OnDestinationReleased);

		// Setup WASD Move events
		EnhancedInputComponent->BindAction(WASDMoveAction, ETriggerEvent::Triggered, this, &AMyHackSlashPlayerController::OnWASDMove);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMyHackSlashPlayerController::OnInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down
void AMyHackSlashPlayerController::OnDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AMyHackSlashPlayerController::OnDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

void AMyHackSlashPlayerController::OnWASDMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn != nullptr)
	{
		// 1. �̵� ����
		const FVector ForwardDirection(1.0f, 0.0f, 0.0f);
		const FVector RightDirection(0.0f, 1.0f, 0.0f);

		ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);

		// 2. ���콺 ���� �ٶ󺸱� ���� �߰�
		FHitResult HitResult;
		// ECC_Visibility ä���� ����Ͽ� ���콺 �Ʒ��� ���� ��ǥ�� �����ɴϴ�.
		if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			FVector MouseLocation = HitResult.Location;
			FVector PawnLocation = ControlledPawn->GetActorLocation();

			// ���콺 ��ġ�� ĳ���� ��ġ ������ ȸ���� ���
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PawnLocation, MouseLocation);

			// ĳ���Ͱ� ���Ʒ��� ����� �ʵ��� Pitch�� Roll�� 0���� �����ϰ� Yaw(�¿� ȸ��)�� ����մϴ�.
			ControlledPawn->SetActorRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));
		}
	}
}

void AMyHackSlashPlayerController::OnAttackTriggered()
{
	AHBaseCharacter* ControlledCharacter = Cast<AHBaseCharacter>(GetPawn());
	if (ControlledCharacter != nullptr)
	{
		//ControlledCharacter->Attack();
	}
}

void AMyHackSlashPlayerController::RequestPurchaseShopItem(FName InShopRowName)
{
	if (InShopRowName.IsNone()) return;

	ServerPurchaseShopItem(InShopRowName);
}

void AMyHackSlashPlayerController::RequestSelectAbilityReward(FName InRewardRowName)
{
	if (InRewardRowName.IsNone()) return;

	ServerSelectAbilityReward(InRewardRowName);
}

void AMyHackSlashPlayerController::RequestRefreshSelectAbilityOptions()
{
	ServerRefreshSelectAbilityOptions();
}

void AMyHackSlashPlayerController::RequestContinueWaveFromResult()
{
	ServerContinueWaveFromResult();
}

void AMyHackSlashPlayerController::RequestPickupCoin(AHCoin* InCoin)
{
	if (!InCoin) return;

	ServerPickupCoin(InCoin);
}

void AMyHackSlashPlayerController::BeginSelectAbilitySelection()
{
	if (!HasAuthority()) return;

	SelectAbilityRefreshCount = MaxSelectAbilityRefreshCount;
	GenerateSelectAbilityOptions(false);
	ClientShowSelectAbilityPopup();
}

void AMyHackSlashPlayerController::ServerPurchaseShopItem_Implementation(FName InShopRowName)
{
	if (InShopRowName.IsNone()) return;

	UHWaveManager* WaveManager = GetWorld() ? GetWorld()->GetSubsystem<UHWaveManager>() : nullptr;
	if (!WaveManager || WaveManager->GetCurrentWaveType() != EHWaveType::Shop)
	{
		return;
	}

	const FHWaveData& CurrentWave = WaveManager->GetCurrentWaveData();
	UDataTable* ShopTable = CurrentWave.ShopRewardTable;
	if (!ShopTable)
	{
		return;
	}

	FHShopRow* ShopRow = ShopTable->FindRow<FHShopRow>(InShopRowName, TEXT("AMyHackSlashPlayerController::ServerPurchaseShopItem"));
	if (!ShopRow)
	{
		return;
	}

	AHPlayerState* HPlayerState = GetPlayerState<AHPlayerState>();
	if (!HPlayerState || !HPlayerState->ConsumeGold(ShopRow->Price))
	{
		UE_LOG(LogTemp, Warning, TEXT("Shop purchase failed: Not enough gold or invalid PlayerState. Row: %s"), *InShopRowName.ToString());
		return;
	}

	ApplyPurchasedShopItem(*ShopRow);
	UE_LOG(LogTemp, Log, TEXT("Shop purchase completed on server. Row: %s"), *InShopRowName.ToString());
}

void AMyHackSlashPlayerController::ApplyPurchasedShopItem(const FHShopRow& InShopRow)
{
	AHPlayerState* HPlayerState = GetPlayerState<AHPlayerState>();
	if (!HPlayerState) return;

	UAbilitySystemComponent* ASC = HPlayerState->GetAbilitySystemComponent();
	AHPlayerCharacter* PlayerCharacter = Cast<AHPlayerCharacter>(GetPawn());

	switch (InShopRow.ItemType)
	{
	case EHShopItemType::Ability:
	{
		if (!ASC) return;

		FGameplayAttribute Attribute;
		const FString TagName = InShopRow.ItemTag.ToString();

		if (TagName.Contains(TEXT("AttackDamage"))) Attribute = UHCharacterAttributeSet::GetAttackDamageAttribute();
		else if (TagName.Contains(TEXT("MaxHealth"))) Attribute = UHCharacterAttributeSet::GetMaxHealthAttribute();
		else if (TagName.Contains(TEXT("MovementSpeed"))) Attribute = UHCharacterAttributeSet::GetMovementSpeedAttribute();
		else if (TagName.Contains(TEXT("CriticalRate"))) Attribute = UHCharacterAttributeSet::GetCriticalRateAttribute();

		if (Attribute.IsValid())
		{
			UGameplayEffect* GEBonus = NewObject<UGameplayEffect>(GetTransientPackage());
			GEBonus->DurationPolicy = EGameplayEffectDurationType::Infinite;

			const int32 ModIndex = GEBonus->Modifiers.Num();
			GEBonus->Modifiers.Add(FGameplayModifierInfo());
			FGameplayModifierInfo& ModInfo = GEBonus->Modifiers[ModIndex];
			ModInfo.Attribute = Attribute;
			ModInfo.ModifierOp = EGameplayModOp::Additive;
			ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(InShopRow.EffectValue));

			ASC->ApplyGameplayEffectToSelf(GEBonus, 1.0f, ASC->MakeEffectContext());
		}
		break;
	}
	case EHShopItemType::InstantEffect:
	{
		if (!ASC) return;

		if (InShopRow.ItemTag.ToString().Contains(TEXT("RestoreHealth")))
		{
			const float CurrentHP = ASC->GetNumericAttribute(UHCharacterAttributeSet::GetHealthAttribute());
			const float MaxHP = ASC->GetNumericAttribute(UHCharacterAttributeSet::GetMaxHealthAttribute());
			ASC->SetNumericAttributeBase(UHCharacterAttributeSet::GetHealthAttribute(), FMath::Min(CurrentHP + InShopRow.EffectValue, MaxHP));
		}
		break;
	}
	case EHShopItemType::SkillGem:
	{
		if (!PlayerCharacter) return;

		UHGemInventoryComponent* Inventory = PlayerCharacter->GetGemInventoryComponent();
		if (!Inventory) return;

		AMyHackSlashGameMode* GameMode = GetWorld() ? Cast<AMyHackSlashGameMode>(GetWorld()->GetAuthGameMode()) : nullptr;
		UHGemDataAsset* GemCollection = GameMode ? GameMode->GetGemCollectionDataAsset() : nullptr;
		if (!GemCollection) return;

		FString TagName = InShopRow.ItemTag.ToString();
		int32 LastDotIndex = INDEX_NONE;
		TagName.FindLastChar('.', LastDotIndex);
		const FString GemIDString = (LastDotIndex != INDEX_NONE) ? TagName.RightChop(LastDotIndex + 1) : TagName;

		const FName FullID = FName(*FString::Printf(TEXT("%s_T1"), *GemIDString));
		FHGemData GemData;
		if (GemCollection->FindGemData(FullID, GemData))
		{
			Inventory->AddGem(GemData);
		}
		break;
	}
	default:
		break;
	}
}

void AMyHackSlashPlayerController::ServerSelectAbilityReward_Implementation(FName InRewardRowName)
{
	if (InRewardRowName.IsNone()) return;
	if (!IsValidCurrentRewardOption(InRewardRowName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Rejected invalid select ability reward row: %s"), *InRewardRowName.ToString());
		return;
	}

	UHSelectAbilityManager* SelectAbilityManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHSelectAbilityManager>() : nullptr;
	if (!SelectAbilityManager)
	{
		return;
	}

	FHRewardOptionData RewardOption;
	if (!SelectAbilityManager->FindRewardOptionByRowName(InRewardRowName, RewardOption))
	{
		return;
	}

	AHPlayerCharacter* PlayerCharacter = Cast<AHPlayerCharacter>(GetPawn());
	AHPlayerState* HPlayerState = GetPlayerState<AHPlayerState>();
	SelectAbilityManager->ExecuteRewardForPlayer(PlayerCharacter, HPlayerState, RewardOption);

	SelectAbilityOptions.Empty();
	SelectAbilityRefreshCount = 0;
	OnSelectAbilityOptionsChanged.Broadcast();
	ForceNetUpdate();

	UE_LOG(LogTemp, Log, TEXT("Select ability reward completed on server. Row: %s"), *InRewardRowName.ToString());
}

void AMyHackSlashPlayerController::ServerRefreshSelectAbilityOptions_Implementation()
{
	GenerateSelectAbilityOptions(true);
}

void AMyHackSlashPlayerController::ClientShowSelectAbilityPopup_Implementation()
{
	if (UHUIManager* UIManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHUIManager>() : nullptr)
	{
		UIManager->ShowWidgetByName(TEXT("SelectAbilityPopupUI"));
	}
}

void AMyHackSlashPlayerController::ClientShowWaveResult_Implementation(int32 InWaveIndex, EHWaveType InWaveType, int32 InInterestGold, int32 InTotalGold)
{
	if (UHUIManager* UIManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHUIManager>() : nullptr)
	{
		if (UHWaveResultUI* ResultUI = Cast<UHWaveResultUI>(UIManager->ShowWidgetByName(TEXT("WaveResultUI"), 100)))
		{
			ResultUI->SetResultData(InWaveIndex, InWaveType, InInterestGold, InTotalGold);
		}
	}
}

void AMyHackSlashPlayerController::ClientHideWaveResult_Implementation()
{
	if (UHUIManager* UIManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHUIManager>() : nullptr)
	{
		if (UUserWidget* ResultUI = UIManager->GetWidgetByName(TEXT("WaveResultUI")))
		{
			UIManager->HideWidget(ResultUI);
		}
	}
}

void AMyHackSlashPlayerController::ServerContinueWaveFromResult_Implementation()
{
	if (UHWaveManager* WaveManager = GetWorld() ? GetWorld()->GetSubsystem<UHWaveManager>() : nullptr)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		WaveManager->PrepareNextWave();
		WaveManager->StartWave();

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (AMyHackSlashPlayerController* HPlayerController = Cast<AMyHackSlashPlayerController>(It->Get()))
			{
				HPlayerController->ClientHideWaveResult();
			}
		}
	}
}

void AMyHackSlashPlayerController::ServerPickupCoin_Implementation(AHCoin* InCoin)
{
	if (!InCoin) return;

	AHPlayerCharacter* PlayerCharacter = Cast<AHPlayerCharacter>(GetPawn());
	if (!PlayerCharacter) return;

	InCoin->TryPickup(PlayerCharacter);
}

void AMyHackSlashPlayerController::OnRep_SelectAbilityOptions()
{
	OnSelectAbilityOptionsChanged.Broadcast();
}

void AMyHackSlashPlayerController::OnRep_SelectAbilityRefreshCount()
{
	OnSelectAbilityOptionsChanged.Broadcast();
}

void AMyHackSlashPlayerController::GenerateSelectAbilityOptions(bool bInConsumeRefresh)
{
	if (!HasAuthority()) return;

	if (bInConsumeRefresh)
	{
		if (SelectAbilityRefreshCount <= 0) return;
		--SelectAbilityRefreshCount;
	}

	UHSelectAbilityManager* SelectAbilityManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHSelectAbilityManager>() : nullptr;
	if (!SelectAbilityManager) return;

	TArray<FHRewardOptionEntry> NewOptions;
	if (SelectAbilityManager->GetRandomRewardOptionEntries(NewOptions))
	{
		SelectAbilityOptions = NewOptions;
		OnSelectAbilityOptionsChanged.Broadcast();
		ForceNetUpdate();
	}
}

bool AMyHackSlashPlayerController::IsValidCurrentRewardOption(FName InRewardRowName) const
{
	if (InRewardRowName.IsNone()) return false;

	for (const FHRewardOptionEntry& Option : SelectAbilityOptions)
	{
		if (Option.RowName == InRewardRowName)
		{
			return true;
		}
	}

	return false;
}
