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

	if (nullptr == GemCollection)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHGemInventoryComponent: GemCollectionDataAsset is null! Upgrade failed."));
		return;
	}

	// 장착 컴포넌트 가져오기
	UHEquipmentComponent* EquipComp = nullptr;
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
	{
		EquipComp = Player->GetEquipmentComponent();
	}

	bool bHasUpgraded = false;

	// 더 이상 업그레이드할 것이 없을 때까지 반복
	do 
	{
		bHasUpgraded = false;

		// 1. GemID별로 현재 보유 중인 모든 인스턴스(인벤토리 + 장착) 리스트 작성
		TMap<FName, TArray<UHGemBase*>> GemGroups;
		
		// 인벤토리 젬 추가
		for (UHGemBase* Gem : InventoryGems)
		{
			if (Gem)
			{
				GemGroups.FindOrAdd(Gem->GetGemData().GemID).Add(Gem);
			}
		}

		// 장착된 메인 젬 추가
		TMap<UHGemBase*, int32> EquippedSlotMap; // 어떤 젬이 몇 번 슬롯에 있는지 기록
		if (EquipComp)
		{
			for (int32 i = 0; i < 4; ++i)
			{
				if (UHMainGem* EquippedGem = EquipComp->GetEquippedGem(i))
				{
					GemGroups.FindOrAdd(EquippedGem->GetGemData().GemID).Add(EquippedGem);
					EquippedSlotMap.Add(EquippedGem, i);
				}
			}
		}

		// 2. 그룹별로 3개 이상인지 체크
		for (auto& Pair : GemGroups)
		{
			const FName CurrentGemID = Pair.Key;
			TArray<UHGemBase*>& Group = Pair.Value;

			if (Group.Num() >= 3)
			{
				const FHGemData& CurrentData = Group[0]->GetGemData();
				if (CurrentData.NextTierGemID != NAME_None)
				{
					FHGemData NextTierData;
					if (GemCollection->FindGemData(CurrentData.NextTierGemID, NextTierData))
					{
						// --- 업그레이드 실행 ---
						
						// 장착된 젬이 그룹에 포함되어 있는지 확인 (우선순위)
						UHGemBase* EquippedGemToUpgrade = nullptr;
						int32 TargetEquipSlot = -1;

						for (int32 i = 0; i < 3; ++i)
						{
							if (EquippedSlotMap.Contains(Group[i]))
							{
								EquippedGemToUpgrade = Group[i];
								TargetEquipSlot = EquippedSlotMap[Group[i]];
								break; 
							}
						}

						// 하위 젬 3개 제거 (인벤토리 혹은 장착 슬롯에서)
						for (int32 i = 0; i < 3; ++i)
						{
							UHGemBase* GemToRemove = Group[i];
							
							if (InventoryGems.Contains(GemToRemove))
							{
								InventoryGems.Remove(GemToRemove);
							}
						}

						// 상위 젬 생성 및 초기화
						TSubclassOf<UHGemBase> GemClass = (NextTierData.GemCategory == HEGemCategory::Main) 
							? UHMainGem::StaticClass() : UHSupportGem::StaticClass();
						
						UHGemBase* NewTierGem = NewObject<UHGemBase>(this, GemClass);
						NewTierGem->Initialize(NextTierData);

						// 장착된 젬이 업그레이드된 경우 자동으로 다시 장착
						if (EquippedGemToUpgrade && EquipComp)
						{
							if (UHMainGem* NewMainGem = Cast<UHMainGem>(NewTierGem))
							{
								// 1. 기존에 장착되어 있던 보조 젬들도 계승
								if (UHMainGem* OldMainGem = Cast<UHMainGem>(EquippedGemToUpgrade))
								{
									for (UHSupportGem* Support : OldMainGem->GetSupportGems())
									{
										NewMainGem->AddSupportGem(Support);
									}
								}

								// 2. 인벤토리 반환 없이 기존 슬롯 비우기 (중요: 루프 및 중복 생성 방지)
								EquipComp->UnequipGem(TargetEquipSlot, false);

								// 3. 새 젬을 같은 슬롯에 장착
								EquipComp->EquipGem(TargetEquipSlot, NewMainGem);
							}
						}
						else
						{
							// 인벤토리로 추가
							InventoryGems.Add(NewTierGem);
						}

						// 알림 발송
						OnGemUpgraded.Broadcast(NewTierGem);
						OnGemInventoryUpdated.Broadcast();
						
						bHasUpgraded = true;
						break; 
					}
				}
			}
		}

	} while (bHasUpgraded);
}
