#pragma once

#include "CoreMinimal.h"
#include "System/HGlobalTypes.h"
#include "DataAsset/HGemDataAsset.h"
#include "HGemReplicationTypes.generated.h"

USTRUCT(BlueprintType)
struct FHGemInstanceData
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gem")
	FGuid InstanceId;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gem")
	FName GemID = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gem")
	int32 Tier = 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gem")
	HEGemCategory GemCategory = HEGemCategory::Main;

	bool IsValid() const
	{
		return InstanceId.IsValid() && !GemID.IsNone();
	}

	static FHGemInstanceData FromGemData(const FHGemData& InGemData, const FGuid& InInstanceId)
	{
		FHGemInstanceData Result;
		Result.InstanceId = InInstanceId;
		Result.GemID = InGemData.GemID;
		Result.Tier = InGemData.Tier;
		Result.GemCategory = InGemData.GemCategory;
		return Result;
	}

	FHGemData ToDisplayGemData() const
	{
		FHGemData Result;
		Result.GemID = GemID;
		Result.Tier = Tier;
		Result.GemCategory = GemCategory;
		return Result;
	}
};
