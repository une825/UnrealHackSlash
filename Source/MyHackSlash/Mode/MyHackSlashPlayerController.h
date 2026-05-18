// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/HSelectAbilityData.h"
#include "System/HGlobalTypes.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include <EnhancedInputLibrary.h>

#include "MyHackSlashPlayerController.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
struct FHShopRow;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_MULTICAST_DELEGATE(FHOnSelectAbilityOptionsChanged);

UCLASS()
class AMyHackSlashPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyHackSlashPlayerController();

	void RequestPurchaseShopItem(FName InShopRowName);
	void RequestSelectAbilityReward(FName InRewardRowName);
	void RequestRefreshSelectAbilityOptions();
	void RequestContinueWaveFromResult();
	void BeginSelectAbilitySelection();

	UFUNCTION(Client, Reliable)
	void ClientShowWaveResult(int32 InWaveIndex, EHWaveType InWaveType, int32 InInterestGold, int32 InTotalGold);

	const TArray<FHRewardOptionEntry>& GetSelectAbilityOptions() const { return SelectAbilityOptions; }
	int32 GetSelectAbilityRefreshCount() const { return SelectAbilityRefreshCount; }
	bool CanRefreshSelectAbilityOptions() const { return SelectAbilityRefreshCount > 0; }

	FHOnSelectAbilityOptionsChanged OnSelectAbilityOptionsChanged;

	/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UNiagaraSystem* FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* DestinationClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WASDMoveAction;

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	virtual void SetupInputComponent() override;
	
	// To add mapping context
	virtual void BeginPlay();

	/** Input handlers for SetDestination action. */
	void OnInputStarted();
	void OnDestinationTriggered();
	void OnDestinationReleased();

	void OnWASDMove(const FInputActionValue& Value);

	void OnAttackTriggered();

	UFUNCTION(Server, Reliable)
	void ServerPurchaseShopItem(FName InShopRowName);

	UFUNCTION(Server, Reliable)
	void ServerSelectAbilityReward(FName InRewardRowName);

	UFUNCTION(Server, Reliable)
	void ServerRefreshSelectAbilityOptions();

	UFUNCTION(Client, Reliable)
	void ClientShowSelectAbilityPopup();

	UFUNCTION(Client, Reliable)
	void ClientHideWaveResult();

	UFUNCTION(Server, Reliable)
	void ServerContinueWaveFromResult();

	UFUNCTION()
	void OnRep_SelectAbilityOptions();

	UFUNCTION()
	void OnRep_SelectAbilityRefreshCount();

	void ApplyPurchasedShopItem(const FHShopRow& InShopRow);
	void GenerateSelectAbilityOptions(bool bInConsumeRefresh);
	bool IsValidCurrentRewardOption(FName InRewardRowName) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_SelectAbilityOptions)
	TArray<FHRewardOptionEntry> SelectAbilityOptions;

	UPROPERTY(ReplicatedUsing = OnRep_SelectAbilityRefreshCount)
	int32 SelectAbilityRefreshCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Select Ability", meta = (AllowPrivateAccess = "true"))
	int32 MaxSelectAbilityRefreshCount = 3;

	FVector CachedDestination;

	float FollowTime; // For how long it has been pressed
};


