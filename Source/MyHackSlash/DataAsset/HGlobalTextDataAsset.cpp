#include "DataAsset/HGlobalTextDataAsset.h"

UDataTable* UHGlobalTextDataAsset::GetTextTable(EHTextLanguage InLanguage) const
{
	for (const FHLanguageTextTable& TableInfo : LanguageTextTables)
	{
		if (TableInfo.Language == InLanguage)
		{
			return TableInfo.TextTable.LoadSynchronous();
		}
	}

	return nullptr;
}
