#include "Mode/HGameState.h"
#include "Net/UnrealNetwork.h"

void AHGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHGameState, GemCollectionDataAsset);
	DOREPLIFETIME(AHGameState, SelectAbilityGradeDataAsset);
	DOREPLIFETIME(AHGameState, SelectAbilityRewardDataTable);
	DOREPLIFETIME(AHGameState, ReplicatedWaveState);
}

void AHGameState::SetGemCollectionDataAsset(UHGemDataAsset* InGemCollectionDataAsset)
{
	if (!HasAuthority()) return;

	GemCollectionDataAsset = InGemCollectionDataAsset;
	ForceNetUpdate();
}

void AHGameState::SetSelectAbilityDataAssets(UHSelectAbilityGradeDataAsset* InGradeDataAsset, UDataTable* InRewardDataTable)
{
	if (!HasAuthority()) return;

	SelectAbilityGradeDataAsset = InGradeDataAsset;
	SelectAbilityRewardDataTable = InRewardDataTable;
	ForceNetUpdate();
}

void AHGameState::SetReplicatedWaveState(const FHReplicatedWaveState& InWaveState)
{
	if (!HasAuthority()) return;

	ReplicatedWaveState = InWaveState;
	OnReplicatedWaveStateChanged.Broadcast(ReplicatedWaveState);
	ForceNetUpdate();
}

void AHGameState::OnRep_ReplicatedWaveState()
{
	OnReplicatedWaveStateChanged.Broadcast(ReplicatedWaveState);
}
