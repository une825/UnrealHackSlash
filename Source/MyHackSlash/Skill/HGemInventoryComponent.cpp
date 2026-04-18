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

		// 1. GemID와 Tier별로 현재 보유 중인 모든 인스턴스(인벤토리 + 장착) 리스트 작성
		TMap<FName, TArray<UHGemBase*>> GemGroups;
		
		// 인벤토리 젬 추가
		for (UHGemBase* Gem : InventoryGems)
		{
			if (Gem)
			{
				const FHGemData& Data = Gem->GetGemData();
				// 동일한 GemID와 Tier를 가진 젬끼리 묶기 위해 FullID를 생성하여 키로 사용합니다.
				FName FullID = FName(*FString::Printf(TEXT("%s_T%d"), *Data.GemID.ToString(), Data.Tier));
				GemGroups.FindOrAdd(FullID).Add(Gem);
			}
		}

		// 장착된 젬 추가 (메인 + 보조)
		TMap<UHGemBase*, int32> EquippedMainSlotMap; // 메인 젬: 슬롯 인덱스
		TMap<UHGemBase*, UHMainGem*> EquippedSupportMap; // 보조 젬: 소속 메인 젬
		
		if (EquipComp)
		{
			for (int32 i = 0; i < 4; ++i)
			{
				if (UHMainGem* EquippedMain = EquipComp->GetEquippedGem(i))
				{
					// 메인 젬 추가
					const FHGemData& MainData = EquippedMain->GetGemData();
					FName MainFullID = FName(*FString::Printf(TEXT("%s_T%d"), *MainData.GemID.ToString(), MainData.Tier));
					GemGroups.FindOrAdd(MainFullID).Add(EquippedMain);
					EquippedMainSlotMap.Add(EquippedMain, i);

					// 해당 메인 젬에 박힌 보조 젬들도 추가
					for (UHSupportGem* Support : EquippedMain->GetSupportGems())
					{
						if (Support)
						{
							const FHGemData& SupportData = Support->GetGemData();
							FName SupportFullID = FName(*FString::Printf(TEXT("%s_T%d"), *SupportData.GemID.ToString(), SupportData.Tier));
							GemGroups.FindOrAdd(SupportFullID).Add(Support);
							EquippedSupportMap.Add(Support, EquippedMain);
						}
					}
				}
			}
		}

		// 2. 그룹별로 3개 이상인지 체크
		for (auto& Pair : GemGroups)
		{
			TArray<UHGemBase*>& Group = Pair.Value;

			if (Group.Num() >= 3)
			{
				const FHGemData& CurrentData = Group[0]->GetGemData();
				FHGemData NextTierData;
				
				// NextTierGemID 대신 FindNextTierGemData를 사용하여 다음 티어 데이터가 있는지 확인합니다.
				if (GemCollection->FindNextTierGemData(CurrentData, NextTierData))
				{
					// --- 업그레이드 실행 ---
						
						// 장착된 젬이 그룹에 포함되어 있는지 확인 (우선순위)
						UHGemBase* EquippedGemToUpgrade = nullptr;

						for (int32 i = 0; i < 3; ++i)
						{
							if (EquippedMainSlotMap.Contains(Group[i]) || EquippedSupportMap.Contains(Group[i]))
							{
								EquippedGemToUpgrade = Group[i];
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
							else if (EquippedSupportMap.Contains(GemToRemove))
							{
								// 장착된 보조 젬인 경우 메인 젬에서 제거
								if (UHMainGem* OwnerMain = EquippedSupportMap[GemToRemove])
								{
									OwnerMain->RemoveSupportGem(Cast<UHSupportGem>(GemToRemove));
								}
							}
							// 메인 젬 제거는 EquipComp->UnequipGem에서 처리됨
						}

						// 상위 젬 생성 및 초기화
						TSubclassOf<UHGemBase> GemClass = (NextTierData.GemCategory == HEGemCategory::Main) 
							? UHMainGem::StaticClass() : UHSupportGem::StaticClass();
						
						UHGemBase* NewTierGem = NewObject<UHGemBase>(this, GemClass);
						NewTierGem->Initialize(NextTierData);

						// 장착된 젬이 업그레이드된 경우 자동으로 다시 장착
						if (EquippedGemToUpgrade && EquipComp)
						{
							// 1. 메인 젬 업그레이드인 경우
							if (EquippedMainSlotMap.Contains(EquippedGemToUpgrade))
							{
								int32 TargetSlot = EquippedMainSlotMap[EquippedGemToUpgrade];
								if (UHMainGem* NewMainGem = Cast<UHMainGem>(NewTierGem))
								{
									// 기존 보조 젬 계승
									if (UHMainGem* OldMainGem = Cast<UHMainGem>(EquippedGemToUpgrade))
									{
										for (UHSupportGem* Support : OldMainGem->GetSupportGems())
										{
											NewMainGem->AddSupportGem(Support);
										}
									}
									EquipComp->UnequipGem(TargetSlot, false);
									EquipComp->EquipGem(TargetSlot, NewMainGem);
								}
							}
							// 2. 보조 젬 업그레이드인 경우
							else if (EquippedSupportMap.Contains(EquippedGemToUpgrade))
							{
								if (UHMainGem* OwnerMain = EquippedSupportMap[EquippedGemToUpgrade])
								{
									if (UHSupportGem* NewSupportGem = Cast<UHSupportGem>(NewTierGem))
									{
										OwnerMain->AddSupportGem(NewSupportGem);
									}
								}
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

	} while (bHasUpgraded);
}
