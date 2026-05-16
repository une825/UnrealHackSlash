#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "System/HGlobalTypes.h"
#include "HGameState.generated.h"

class UHGemDataAsset;
class UHSelectAbilityGradeDataAsset;
class UDataTable;

USTRUCT(BlueprintType)
struct FHReplicatedWaveState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "H|Wave")
	EHWaveState WaveState = EHWaveState::Ready;

	UPROPERTY(BlueprintReadOnly, Category = "H|Wave")
	int32 WaveIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "H|Wave")
	EHWaveType WaveType = EHWaveType::Battle;

	UPROPERTY(BlueprintReadOnly, Category = "H|Wave")
	EHWaveClearType ClearType = EHWaveClearType::TimeSurvival;

	UPROPERTY(BlueprintReadOnly, Category = "H|Wave")
	float ProgressPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "H|Wave")
	float CurrentValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "H|Wave")
	float TargetValue = 0.0f;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FHOnReplicatedWaveStateChanged, const FHReplicatedWaveState&);

UCLASS()
class MYHACKSLASH_API AHGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetGemCollectionDataAsset(UHGemDataAsset* InGemCollectionDataAsset);
	UHGemDataAsset* GetGemCollectionDataAsset() const { return GemCollectionDataAsset; }

	void SetSelectAbilityDataAssets(UHSelectAbilityGradeDataAsset* InGradeDataAsset, UDataTable* InRewardDataTable);
	UHSelectAbilityGradeDataAsset* GetSelectAbilityGradeDataAsset() const { return SelectAbilityGradeDataAsset; }
	UDataTable* GetSelectAbilityRewardDataTable() const { return SelectAbilityRewardDataTable; }

	void SetReplicatedWaveState(const FHReplicatedWaveState& InWaveState);
	const FHReplicatedWaveState& GetReplicatedWaveState() const { return ReplicatedWaveState; }

	FHOnReplicatedWaveStateChanged OnReplicatedWaveStateChanged;

private:
	UFUNCTION()
	void OnRep_ReplicatedWaveState();

private:
	UPROPERTY(Replicated)
	TObjectPtr<UHGemDataAsset> GemCollectionDataAsset;

	UPROPERTY(Replicated)
	TObjectPtr<UHSelectAbilityGradeDataAsset> SelectAbilityGradeDataAsset;

	UPROPERTY(Replicated)
	TObjectPtr<UDataTable> SelectAbilityRewardDataTable;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedWaveState)
	FHReplicatedWaveState ReplicatedWaveState;
};
