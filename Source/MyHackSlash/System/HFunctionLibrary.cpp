#include "System/HFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Mode/MyHackSlashGameMode.h"
#include "DataAsset/HGemDataAsset.h"
#include "Engine/Texture2D.h"

FString UHFunctionLibrary::MakeGemTagString(FName InGemID)
{
	if (InGemID.IsNone()) return TEXT("Data.GemID");
	return FString::Printf(TEXT("Data.GemID.%s"), *InGemID.ToString());
}

class UTexture2D* UHFunctionLibrary::GetGemIcon(const UObject* WorldContextObject, FName InGemID, int32 InTier)
{
	if (nullptr == WorldContextObject || InGemID.IsNone()) return nullptr;

	AMyHackSlashGameMode* GameMode = Cast<AMyHackSlashGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (nullptr == GameMode) return nullptr;

	UHGemDataAsset* GemCollection = GameMode->GetGemCollectionDataAsset();
	if (nullptr == GemCollection) return nullptr;

	// FullID 생성 (예: FireBall_T1)
	FName FullID = *FString::Printf(TEXT("%s_T%d"), *InGemID.ToString(), InTier);

	FHGemData FoundData;
	if (GemCollection->FindGemData(FullID, FoundData))
	{
		return FoundData.Icon;
	}

	return nullptr;
}
