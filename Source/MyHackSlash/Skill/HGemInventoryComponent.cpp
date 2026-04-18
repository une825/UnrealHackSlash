#include "Skill/HGemInventoryComponent.h"
#include "Skill/SkillGem/HGemBase.h"
#include "Skill/SkillGem/HMainGem.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "DataAsset/HGemDataAsset.h"
#include "Mode/MyHackSlashGameMode.h"
#include "Skill/HEquipmentComponent.h"
#include "Unit/Player/HPlayerCharacter.h"

UHGemInventoryComponent::UHGemInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHGemInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InventoryGems.Empty();
}

UHGemBase* UHGemInventoryComponent::AddGem(const FHGemData& InGemData)
{
	if (InGemData.GemID == NAME_None) return nullptr;

	// 카테고리에 맞는 클래스 결정
	TSubclassOf<UHGemBase> GemClass = (InGemData.GemCategory == HEGemCategory::Main) 
		? UHMainGem::StaticClass() : UHSupportGem::StaticClass();

	// 인스턴스 생성 및 초기화
	UHGemBase* NewGem = NewObject<UHGemBase>(this, GemClass);
	NewGem->Initialize(InGemData);

	// 인벤토리에 새 젬 추가
	InventoryGems.Add(NewGem);

	// 자동 업그레이드 체크
	CheckAndUpgradeGems();

	OnGemInventoryUpdated.Broadcast();

	return NewGem;
}

void UHGemInventoryComponent::AddGemInstance(UHGemBase* InGemInstance)
{
	if (InGemInstance && !InventoryGems.Contains(InGemInstance))
	{
		InventoryGems.Add(InGemInstance);

		// 자동 업그레이드 체크
		CheckAndUpgradeGems();

		OnGemInventoryUpdated.Broadcast();
	}
}

void UHGemInventoryComponent::RemoveGemInstance(UHGemBase* InGemInstance)
{
	if (InGemInstance && InventoryGems.Contains(InGemInstance))
	{
		InventoryGems.Remove(InGemInstance);
		OnGemInventoryUpdated.Broadcast();
	}
}

void UHGemInventoryComponent::CheckAndUpgradeGems()
{
	UHGemDataAsset* GemCollection = nullptr;
	if (AMyHackSlashGameMode* GameMode = Cast<AMyHackSlashGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GemCollection = GameMode->GetGemCollectionDataAsset();
	}

	if (!GemCollection) return;

	UHEquipmentComponent* EquipComp = nullptr;
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
	{
		EquipComp = Player->GetEquipmentComponent();
	}

	// 더 이상 업그레이드할 것이 없을 때까지 반복 (연쇄 업그레이드 지원)
	while (TryUpgradeSingleGroup(GemCollection, EquipComp))
	{
		// 한 번이라도 업그레이드되면 다시 처음부터 체크 (새로운 조합 가능성 때문)
	}
}

void UHGemInventoryComponent::GatherAllGems(TMap<FName, TArray<UHGemBase*>>& OutGemGroups, TMap<UHGemBase*, int32>& OutMainSlotMap, TMap<UHGemBase*, int32>& OutSupportSlotMap) const
{
	// 1. 인벤토리 젬 수집
	for (UHGemBase* Gem : InventoryGems)
	{
		if (Gem)
		{
			const FHGemData& Data = Gem->GetGemData();
			FName FullID = FName(*FString::Printf(TEXT("%s_T%d"), *Data.GemID.ToString(), Data.Tier));
			OutGemGroups.FindOrAdd(FullID).Add(Gem);
		}
	}

	// 2. 장착된 젬 수집 (메인 + 보조)
	AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner());
	UHEquipmentComponent* EquipComp = Player ? Player->GetEquipmentComponent() : nullptr;

	if (EquipComp)
	{
		for (int32 i = 0; i < 4; ++i)
		{
			// 메인 젬 수집
			if (UHMainGem* MainGem = EquipComp->GetEquippedGem(i))
			{
				const FHGemData& MainData = MainGem->GetGemData();
				FName MainFullID = FName(*FString::Printf(TEXT("%s_T%d"), *MainData.GemID.ToString(), MainData.Tier));
				OutGemGroups.FindOrAdd(MainFullID).Add(MainGem);
				OutMainSlotMap.Add(MainGem, i);
			}

			// 슬롯에 있는 모든 보조 젬 수집 (메인 젬 장착 여부와 상관없이)
			TArray<UHSupportGem*> SupportGems = EquipComp->GetEquippedSupportGems(i);
			for (UHSupportGem* Support : SupportGems)
			{
				if (Support)
				{
					const FHGemData& SupportData = Support->GetGemData();
					FName SupportFullID = FName(*FString::Printf(TEXT("%s_T%d"), *SupportData.GemID.ToString(), SupportData.Tier));
					OutGemGroups.FindOrAdd(SupportFullID).Add(Support);
					OutSupportSlotMap.Add(Support, i);
				}
			}
		}
	}
}

bool UHGemInventoryComponent::TryUpgradeSingleGroup(UHGemDataAsset* GemCollection, UHEquipmentComponent* EquipComp)
{
	TMap<FName, TArray<UHGemBase*>> GemGroups;
	TMap<UHGemBase*, int32> MainSlotMap;
	TMap<UHGemBase*, int32> SupportSlotMap;

	GatherAllGems(GemGroups, MainSlotMap, SupportSlotMap);

	for (auto& Pair : GemGroups)
	{
		TArray<UHGemBase*>& Group = Pair.Value;
		if (Group.Num() < 3) continue;

		const FHGemData& CurrentData = Group[0]->GetGemData();
		FHGemData NextTierData;

		if (GemCollection->FindNextTierGemData(CurrentData, NextTierData))
		{
			// 1. 업그레이드 대상 식별 (전체 그룹에서 장착 중인 것을 최우선으로 찾음)
			UHGemBase* SourceGem = nullptr;
			for (UHGemBase* Gem : Group)
			{
				if (MainSlotMap.Contains(Gem) || SupportSlotMap.Contains(Gem))
				{
					SourceGem = Gem;
					break;
				}
			}

			// 2. 재료 3개 선정 (SourceGem이 있다면 반드시 포함)
			TArray<UHGemBase*> Ingredients;
			if (SourceGem) Ingredients.Add(SourceGem);
			for (UHGemBase* Gem : Group)
			{
				if (Ingredients.Num() >= 3) break;
				if (Gem != SourceGem) Ingredients.Add(Gem);
			}

			// 3. 데이터 캡처 (Distribution에 필요)
			int32 MainSlot = SourceGem ? MainSlotMap.FindRef(SourceGem, -1) : -1;
			int32 SupportSlot = SourceGem ? SupportSlotMap.FindRef(SourceGem, -1) : -1;

			// 4. 하위 젬 3개 제거
			ConsumeGems(Ingredients, SupportSlotMap, EquipComp);

			// 5. 상위 젬 생성
			TSubclassOf<UHGemBase> GemClass = (NextTierData.GemCategory == HEGemCategory::Main)
				? UHMainGem::StaticClass() : UHSupportGem::StaticClass();
			UHGemBase* NewTierGem = NewObject<UHGemBase>(this, GemClass);
			NewTierGem->Initialize(NextTierData);

			// 6. 결과물 배치 및 알림
			DistributeUpgradedGem(NewTierGem, SourceGem, EquipComp, MainSlot, SupportSlot);

			OnGemUpgraded.Broadcast(NewTierGem);
			OnGemInventoryUpdated.Broadcast();

			return true;
		}
	}

	return false;
}

void UHGemInventoryComponent::ConsumeGems(const TArray<UHGemBase*>& InGems, const TMap<UHGemBase*, int32>& InSupportSlotMap, UHEquipmentComponent* EquipComp)
{
	for (UHGemBase* Gem : InGems)
	{
		if (InventoryGems.Contains(Gem))
		{
			InventoryGems.Remove(Gem);
		}
		else if (EquipComp && InSupportSlotMap.Contains(Gem))
		{
			// 보조 젬은 EquipComp를 통해 해제 (슬롯에서도 제거됨)
			int32 SlotIdx = InSupportSlotMap.FindRef(Gem);
			EquipComp->UnequipSupportGem(SlotIdx, Cast<UHSupportGem>(Gem), false);
		}
		// 메인 젬은 DistributeUpgradedGem에서 Unequip을 통해 교체되므로 여기서 제거하지 않음
	}
}

void UHGemInventoryComponent::DistributeUpgradedGem(UHGemBase* NewGem, UHGemBase* SourceGem, UHEquipmentComponent* EquipComp, int32 MainSlot, int32 SupportSlot)
{
	if (SourceGem && EquipComp)
	{
		// 1. 메인 젬 업그레이드인 경우
		if (MainSlot != -1)
		{
			if (UHMainGem* NewMain = Cast<UHMainGem>(NewGem))
			{
				if (UHMainGem* OldMain = Cast<UHMainGem>(SourceGem))
				{
					// 중요: Unequip 전에 보조 젬들을 미리 복사 (Unequip이 내부 리스트를 비움)
					TArray<UHSupportGem*> OldSupports = OldMain->GetSupportGems();
					for (UHSupportGem* Support : OldSupports)
					{
						NewMain->AddSupportGem(Support);
					}
				}
				// 기존 장착 해제 후 새 젬 장착
				EquipComp->UnequipGem(MainSlot, false);
				EquipComp->EquipGem(MainSlot, NewMain);
				return;
			}
		}
		// 2. 보조 젬 업그레이드인 경우
		else if (SupportSlot != -1)
		{
			if (UHSupportGem* NewSupport = Cast<UHSupportGem>(NewGem))
			{
				// EquipComp를 통해 장착 (MainGem 연결 및 슬롯 데이터 동기화)
				EquipComp->EquipSupportGem(SupportSlot, NewSupport);
				return;
			}
		}
	}

	// 장착된 것이 없거나 배치 실패 시 인벤토리에 추가
	InventoryGems.Add(NewGem);
}
