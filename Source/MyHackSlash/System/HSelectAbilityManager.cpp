#include "System/HSelectAbilityManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/HGemDataAsset.h"
#include "Skill/HGemInventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include <Unit/Player/HPlayerState.h>

void UHSelectAbilityManager::InitializeManager(UHSelectAbilityGradeDataAsset* InGradeDataAsset, UDataTable* InRewardTable, UHGemDataAsset* InGemCollection)
{
	GradeDataAsset = InGradeDataAsset;
	RewardDataTable = InRewardTable;
	GemCollection = InGemCollection;

	// 매니저 초기화 시 횟수 초기화
	ResetRefreshCount();
}

bool UHSelectAbilityManager::GetRandomRewardOptions(TArray<FHRewardOptionData>& OutOptions)
{
	if (!GradeDataAsset || !RewardDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHSelectAbilityManager: DataAsset or DataTable is not set."));
		return false;
	}

	// 1. 이번 팝업의 등급 결정
	EHAbilityGrade TargetGrade = RollGrade();

	// 2. DataTable에서 해당 등급에 맞는 모든 옵션 필터링
	TArray<FHRewardOptionData*> AllMatchingOptions;
	RewardDataTable->GetAllRows<FHRewardOptionData>(TEXT("HSelectAbilityManager::GetRandomRewardOptions"), AllMatchingOptions);

	TArray<FHRewardOptionData> FilteredPool;
	for (auto* Option : AllMatchingOptions)
	{
		if (Option && Option->Grade == TargetGrade)
		{
			FilteredPool.Add(*Option);
		}
	}

	if (FilteredPool.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHSelectAbilityManager: No rewards found for grade %d."), (int32)TargetGrade);
		return false;
	}

	// 3. 필터링된 풀에서 무작위로 3개 추출 (중복 방지를 위해 섞은 후 순차적으로 선택)
	// 배열 섞기
	int32 LastIndex = FilteredPool.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 IndexToSwap = UKismetMathLibrary::RandomIntegerInRange(i, LastIndex);
		if (i != IndexToSwap)
		{
			FilteredPool.Swap(i, IndexToSwap);
		}
	}

	// 최대 3개까지만 결과 배열에 추가
	int32 PickCount = FMath::Min(FilteredPool.Num(), 3);
	OutOptions.Empty();
	for (int32 i = 0; i < PickCount; ++i)
	{
		OutOptions.Add(FilteredPool[i]);
	}

	return OutOptions.Num() > 0;
}

void UHSelectAbilityManager::ExecuteReward(const FHRewardOptionData& InSelectedOption)
{
	UE_LOG(LogTemp, Log, TEXT("Executing Reward: %s (Type: %d, ID: %s, Amount: %d)"), 
		*InSelectedOption.Description.ToString(), (int32)InSelectedOption.RewardType, *InSelectedOption.TargetID.ToString(), InSelectedOption.Amount);

	switch (InSelectedOption.RewardType)
	{
	case EHRewardType::GetSkillGem:
	{
		// 1. 젬 컬렉션에서 데이터 찾기
		if (GemCollection)
		{
			FHGemData FoundData;
			if (GemCollection->FindGemData(InSelectedOption.TargetID, FoundData))
			{
				// 2. 플레이어 캐릭터로부터 젬 인벤토리 컴포넌트 가져오기
				APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
				if (PlayerPawn)
				{
					UHGemInventoryComponent* GemInventory = PlayerPawn->FindComponentByClass<UHGemInventoryComponent>();
					if (GemInventory)
					{
						// 3. Amount 만큼 반복하여 인벤토리에 젬 추가
						const int32 ActualGrantCount = FMath::Max(1, InSelectedOption.Amount);
						for (int32 i = 0; i < ActualGrantCount; ++i)
						{
							GemInventory->AddGem(FoundData);
						}
						UE_LOG(LogTemp, Log, TEXT("%d Gem(s) (%s) added to inventory via UHGemInventoryComponent"), ActualGrantCount, *FoundData.GemName.ToString());
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("UHSelectAbilityManager: GemID %s not found in GemCollection!"), *InSelectedOption.TargetID.ToString());
			}
		}
		break;
	}

	case EHRewardType::GetGold:
	{
		// 플레이어 스테이트를 찾아 골드 추가
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			if (AHPlayerState* PS = PC->GetPlayerState<AHPlayerState>())
			{
				PS->AddGold(InSelectedOption.Amount);
				UE_LOG(LogTemp, Log, TEXT("Reward Executed: Added %d Gold. Total: %d"), InSelectedOption.Amount, PS->GetCurrentGold());
			}
		}
		break;
	}

	case EHRewardType::GetReroll:
		CurrentRefreshCount += InSelectedOption.Amount;
		UE_LOG(LogTemp, Log, TEXT("Refresh count increased by %d. Current: %d"), InSelectedOption.Amount, CurrentRefreshCount);
		break;

	default:
		break;
	}
}

EHAbilityGrade UHSelectAbilityManager::RollGrade() const
{
	if (!GradeDataAsset || GradeDataAsset->GradeProbabilities.Num() == 0)
	{
		return EHAbilityGrade::Silver;
	}

	float RandomValue = UKismetMathLibrary::RandomFloatInRange(0.0f, 100.0f);
	float CumulativeProbability = 0.0f;

	for (const auto& ProbData : GradeDataAsset->GradeProbabilities)
	{
		CumulativeProbability += ProbData.Probability;
		if (RandomValue <= CumulativeProbability)
		{
			return ProbData.Grade;
		}
	}

	// 설정된 확률 합이 100% 미만이거나 난수가 범위를 벗어난 경우 마지막 등급 반환
	return GradeDataAsset->GradeProbabilities.Last().Grade;
}
