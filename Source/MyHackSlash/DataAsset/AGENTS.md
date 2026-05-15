# DATA ASSET KNOWLEDGE BASE

## OVERVIEW
Centralized data management for game systems using Unreal Engine's Data Assets and Primary Data Assets.

## WHERE TO LOOK
| Asset Type | File | Description |
|------------|------|-------------|
| Gems | `HGemDataAsset.h` | Gem properties, tiers, and support effects. |
| Monsters | `HMonsterDataAsset.h` | Soft references to monster classes for spawning. |
| Unit Profiles | `HUnitProfileData.h` | Animation montages, AI parameters, and hit effects. |
| Waves | `HWaveConfigDataAsset.h` | Wave sequences and economy settings (interest rates). |
| Rewards | `HSelectAbilityData.h` | Reward selection logic, grades, and probabilities. |
| Shop | `HShopRow.h` | Shop item definitions, pricing, and weights. |
| UI | `HUIDataAsset.h` | UI-related data and assets. |
| Spawning | `HMonsterSpawnerDataAsset.h` | Configuration for monster spawn waves and logic. |
| Rewards | `HBreakableRewardDataAsset.h` | Data for rewards dropped from breakable objects. |
| Sounds | `HSoundDataAsset.h` | Centralized sound asset management for game events. |

## CONVENTIONS
*   Use `UPrimaryDataAsset` for assets that need to be loaded via Asset Manager.
*   Use `TSoftClassPtr` or `TSoftObjectPtr` for heavy assets to support asynchronous loading.
*   Define `FTableRowBase` structures within the same header as the Data Asset for DataTable integration.
*   Use `FGameplayTag` for flexible identification of items, effects, and costs.
*   Category naming follows the pattern "H|SystemName" or "SystemName".
*   Ensure all `UPROPERTY` fields are properly categorized for easier editing in the Unreal Editor.
*   Use `meta = (AssetBundles = "BundleName")` for assets that should be grouped during loading.
*   Prefer `TMap` for lookups when mapping tags or names to specific assets or classes.

## ANTI-PATTERNS
*   Avoid hardcoding asset paths; use `TSoftObjectPtr` or `TSoftClassPtr` in Data Assets.
*   Do not store runtime state in Data Assets; they are for static configuration.
*   Avoid direct logic implementation in Data Assets; keep them as data containers with minimal helper functions.
*   Do not repeat parent content from the root `AGENTS.md`.
