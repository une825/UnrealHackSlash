#include "System/HGlobalTextManager.h"

void UHGlobalTextManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TextDataAsset = TextDataAssetPath.LoadSynchronous();
	if (TextDataAsset)
	{
		ActiveLanguage = TextDataAsset->DefaultLanguage;
		RefreshTextCache();
	}
}

void UHGlobalTextManager::SetTextDataAsset(UHGlobalTextDataAsset* InTextDataAsset)
{
	TextDataAsset = InTextDataAsset;
	if (!TextDataAsset)
	{
		ActiveTextCache.Empty();
		FallbackTextCache.Empty();
		return;
	}

	ActiveLanguage = TextDataAsset->DefaultLanguage;
	RefreshTextCache();
	OnLanguageChanged.Broadcast(ActiveLanguage);
}

bool UHGlobalTextManager::SetActiveLanguage(EHTextLanguage InLanguage)
{
	if (!TextDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHGlobalTextManager: TextDataAsset is null."));
		return false;
	}

	if (!TextDataAsset->GetTextTable(InLanguage))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHGlobalTextManager: TextTable for language [%d] is not set."), static_cast<int32>(InLanguage));
		return false;
	}

	ActiveLanguage = InLanguage;
	RefreshTextCache();
	OnLanguageChanged.Broadcast(ActiveLanguage);
	return true;
}

FText UHGlobalTextManager::GetText(FName InTextKey) const
{
	FText FoundText;
	if (FindText(InTextKey, FoundText))
	{
		return FoundText;
	}

	return FText::FromName(InTextKey);
}

bool UHGlobalTextManager::FindText(FName InTextKey, FText& OutText) const
{
	if (const FText* FoundText = ActiveTextCache.Find(InTextKey))
	{
		OutText = *FoundText;
		return true;
	}

	if (const FText* FoundText = FallbackTextCache.Find(InTextKey))
	{
		OutText = *FoundText;
		return true;
	}

	return false;
}

void UHGlobalTextManager::RefreshTextCache()
{
	ActiveTextCache.Empty();
	FallbackTextCache.Empty();

	if (!TextDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHGlobalTextManager: TextDataAsset is null."));
		return;
	}

	BuildTextCache(ActiveLanguage, ActiveTextCache);

	if (TextDataAsset->FallbackLanguage != ActiveLanguage)
	{
		BuildTextCache(TextDataAsset->FallbackLanguage, FallbackTextCache);
	}
}

void UHGlobalTextManager::BuildTextCache(EHTextLanguage InLanguage, TMap<FName, FText>& OutTextCache) const
{
	if (!TextDataAsset)
	{
		return;
	}

	UDataTable* TextTable = TextDataAsset->GetTextTable(InLanguage);
	if (!TextTable)
	{
		return;
	}

	for (const TPair<FName, uint8*>& RowPair : TextTable->GetRowMap())
	{
		const FHGlobalTextRow* TextRow = reinterpret_cast<const FHGlobalTextRow*>(RowPair.Value);
		if (!TextRow)
		{
			continue;
		}

		if (RowPair.Key.IsNone())
		{
			continue;
		}

		OutTextCache.Add(RowPair.Key, TextRow->Text);
	}
}
