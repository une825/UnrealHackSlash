#include "System/HSelectAbilityManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/HGemDataAsset.h"
#include "Skill/HGemInventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include <Unit/Player/HPlayerState.h>
#include "System/HFunctionLibrary.h"

void UHSelectAbilityManager::InitializeManager(UHSelectAbilityGradeDataAsset* InGradeDataAsset, UDataTable* InRewardTable, UHGemDataAsset* InGemCollection, TSubclassOf<class UGameplayEffect> InAddGoldEffectClass)
{
	GradeDataAsset = InGradeDataAsset;
	RewardDataTable = InRewardTable;
	GemCollection = InGemCollection;
	AddGoldEffectClass = InAddGoldEffectClass;

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

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(PlayerPawn);
	if (!ASCInterface)
	{
		// 플레이어 스테이트에서도 확인
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			ASCInterface = Cast<IAbilitySystemInterface>(PC->GetPlayerState<AHPlayerState>());
		}
	}

	UAbilitySystemComponent* TargetASC = ASCInterface ? ASCInterface->GetAbilitySystemComponent() : nullptr;
	if (!TargetASC)
	{
		UE_LOG(LogTemp, Error, TEXT("UHSelectAbilityManager: Could not find Target ASC for reward execution!"));
		return;
	}

	switch (InSelectedOption.RewardType)
	{
	case EHRewardType::GetSkillGem:
	{
		// 젬 지급 이벤트를 전송합니다.
		FGameplayEventData Payload;
		Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Character.Reward.GetGem"));
		Payload.OptionalObject = GemCollection; // 컬렉션 전달
		Payload.Instigator = PlayerPawn;
		Payload.Target = PlayerPawn;
		Payload.EventMagnitude = static_cast<float>(InSelectedOption.Amount);
		Payload.TargetData.Add(new FGameplayAbilityTargetData_ActorArray()); // 빈 타겟 데이터라도 추가 (안전용)
		
		// TargetID(젬 ID)를 컨텍스트나 다른 필드에 담아 보내야 하는데, 
		// 여기서는 간단히 Payload의 OptionalObject2 등을 활용하거나 
		// 특정 명명 규칙을 가진 태그를 추가하여 보낼 수 있습니다.
		// 일단 가장 범용적인 HandleGameplayEvent를 사용합니다.
		
		// 젬 ID를 식별하기 위해 데이터 에셋의 정보를 Payload의 InstigatorTags 등에 임시로 담을 수도 있습니다.
		if (InSelectedOption.TargetID.IsNone())
		{
			UE_LOG(LogTemp, Error, TEXT("UHSelectAbilityManager: TargetID is NONE for GetSkillGem reward!"));
			break;
		}

		FString TagString = UHFunctionLibrary::MakeGemTagString(InSelectedOption.TargetID);
		FGameplayTag GemIDTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
		
		if (GemIDTag.IsValid())
		{
			Payload.InstigatorTags.AddTag(GemIDTag);
		}
		else
		{
			// 태그가 없더라도 ID 정보를 전달하기 위해 최소한 부모 태그라도 넣어줄 수 있습니다.
			Payload.InstigatorTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Data.GemID")));
		}

		// 티어 정보를 추가합니다. (예: Data.Tier.1)
		FString TierTagString = FString::Printf(TEXT("Data.Tier.%d"), InSelectedOption.Tier);
		FGameplayTag TierTag = FGameplayTag::RequestGameplayTag(FName(*TierTagString), false);
		if (TierTag.IsValid())
		{
			Payload.InstigatorTags.AddTag(TierTag);
		}

		TargetASC->HandleGameplayEvent(Payload.EventTag, &Payload);
		UE_LOG(LogTemp, Log, TEXT("Reward Event Sent: GetGem (%s, Tier: %d)"), *InSelectedOption.TargetID.ToString(), InSelectedOption.Tier);
		break;
	}

	case EHRewardType::GetGold:
	{
		if (AddGoldEffectClass)
		{
			FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(AddGoldEffectClass, 1.0f, Context);
			if (SpecHandle.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Gold")), static_cast<float>(InSelectedOption.Amount));
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				UE_LOG(LogTemp, Log, TEXT("Reward Executed via GAS: Added %d Gold."), InSelectedOption.Amount);
			}
		}
		else
		{
			// 폴백: 클래스가 지정되지 않은 경우 기존 방식 유지 (또는 경고)
			if (AHPlayerState* PS = Cast<AHPlayerState>(TargetASC->GetOwnerActor()))
			{
				PS->AddGold(InSelectedOption.Amount);
				UE_LOG(LogTemp, Warning, TEXT("AddGoldEffectClass is NULL. Falling back to direct call."));
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
