#include "DataAsset/HGemDataAsset.h"

bool UHGemDataAsset::FindGemData(FName InFullID, FHGemData& OutData) const
{
	if (!GemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHGemDataAsset: GemDataTable is not set!"));
		return false;
	}

	FHGemData* FoundRow = GemDataTable->FindRow<FHGemData>(InFullID, TEXT("UHGemDataAsset::FindGemData"));
	if (FoundRow)
	{
		OutData = *FoundRow;
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("UHGemDataAsset: Failed to find GemData with RowName [%s] in DataTable [%s]"), *InFullID.ToString(), *GemDataTable->GetName());
	return false;
}

void UHGemDataAsset::GetAllGemData(TArray<FHGemData>& OutDataList) const
{
	if (!GemDataTable) return;

	TArray<FHGemData*> AllRows;
	GemDataTable->GetAllRows<FHGemData>(TEXT("UHGemDataAsset::GetAllGemData"), AllRows);

	OutDataList.Empty();
	for (auto* Row : AllRows)
	{
		if (Row)
		{
			OutDataList.Add(*Row);
		}
	}
}

TSubclassOf<UGameplayAbility> UHGemDataAsset::FindAbilityClassByTagName(FName InTagName) const
{
	if (const TSubclassOf<UGameplayAbility>* FoundClass = SkillAbilityClassMap.Find(InTagName))
	{
		return *FoundClass;
	}

	return nullptr;
}

bool UHGemDataAsset::FindNextTierGemData(const FHGemData& InCurrentData, FHGemData& OutNextData) const
{
	if (InCurrentData.GemID.IsNone()) return false;

	// 다음 티어 ID 생성 (예: FireBall_T1 -> FireBall_T2)
	FName NextFullID = FName(*FString::Printf(TEXT("%s_T%d"), *InCurrentData.GemID.ToString(), InCurrentData.Tier + 1));
	
	return FindGemData(NextFullID, OutNextData);
}
