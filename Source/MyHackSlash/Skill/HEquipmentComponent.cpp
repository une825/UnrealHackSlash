#include "Skill/HEquipmentComponent.h"
#include "Net/UnrealNetwork.h"
#include "Skill/SkillGem/HMainGem.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Skill/HGemInventoryComponent.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Mode/MyHackSlashGameMode.h"

UHEquipmentComponent::UHEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	EquippedMainGems.Init(nullptr, 4); // 4개 슬롯 초기화
	EquippedMainGemData.SetNum(4);
	SlotSupportGems.SetNum(4);         // 4개 슬롯 보조 젬 리스트 초기화
	SlotSupportGemData.SetNum(4);
}

void UHEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}

void UHEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHEquipmentComponent, EquippedMainGemData);
	DOREPLIFETIME(UHEquipmentComponent, SlotSupportGemData);
}

void UHEquipmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateAutoCast(DeltaTime);
}

bool UHEquipmentComponent::EquipGem(int32 InSlotIndex, UHMainGem* InGem)
{
	if (!EquippedMainGems.IsValidIndex(InSlotIndex) || !InGem)
	{
		return false;
	}

	if (AActor* Owner = GetOwner())
	{
		if (!Owner->HasAuthority())
		{
			ServerEquipGemById(InSlotIndex, InGem->GetInstanceId());
			return true;
		}
	}

	if (nullptr == ASC)
	{
		ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	}

	// 1. 다른 슬롯에 이미 장착되어 있었다면 해제 (이동 처리)
	for (int32 i = 0; i < EquippedMainGems.Num(); ++i)
	{
		if (EquippedMainGems[i] == InGem)
		{
			UnequipGem(i, false);
			break;
		}
	}

	// 2. 인벤토리에서 제거
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
	{
		if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
		{
			InvComp->RemoveGemInstance(InGem);
		}
	}

	// 3. 현재 타겟 슬롯의 기존 메인 젬 해제
	UnequipGem(InSlotIndex, true);

	// 4. 새 메인 젬 장착
	EquippedMainGems[InSlotIndex] = InGem;

	// 5. 자동 발동 타이머 초기화 (슬롯 1~3인 경우)
	if (InSlotIndex > 0)
	{
		AutoCastTimers.Add(InSlotIndex, 0.0f); // 즉시 첫 발사 시도
	}

	// 6. 슬롯에 남아있던 보조 젬들을 새 메인 젬에 연결 시도
	// 호환되지 않는 보조 젬은 자동으로 인벤토리로 반환 처리
	TArray<UHSupportGem*> IncompatibleGems;
	for (UHSupportGem* SupportGem : SlotSupportGems[InSlotIndex].SupportGems)
	{
		if (SupportGem)
		{
			// 메인 젬에 연결 시도 (내부적으로 CanAttach 체크 수행)
			if (!InGem->AddSupportGem(SupportGem))
			{
				IncompatibleGems.Add(SupportGem);
			}
		}
	}

	// 호환되지 않는 젬들을 슬롯에서 제거하고 인벤토리로 반환
	for (UHSupportGem* IncompatibleGem : IncompatibleGems)
	{
		UnequipSupportGem(InSlotIndex, IncompatibleGem, true);
	}

	// 7. GAS 어빌리티 부여
	if (AMyHackSlashGameMode* GameMode = Cast<AMyHackSlashGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UHGemDataAsset* GemCollection = GameMode->GetGemCollectionDataAsset())
		{
			TSubclassOf<UGameplayAbility> FindAbility = GemCollection->FindAbilityClassByTagName(InGem->GetGemData().AbilityTagName);
			if (ASC && FindAbility)
			{
				FGameplayAbilitySpec Spec(FindAbility);
				Spec.InputID = InSlotIndex + 1;
				Spec.SourceObject = InGem;

				FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
				EquippedAbilityHandles.Add(InSlotIndex, Handle);
			}
		}
	}

	OnEquipmentChanged.Broadcast();
	SyncEquippedGemDataFromInstances();
	return true;
}

void UHEquipmentComponent::UpdateAutoCast(float InDeltaTime)
{
	if (!ASC) return;
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	// 슬롯 1부터 3까지 자동 발동 체크
	for (int32 i = 1; i <= 3; ++i)
	{
		UHMainGem* MainGem = GetEquippedGem(i);
		if (!MainGem)
		{
			AutoCastTimers.Remove(i);
			continue;
		}

		if (!AutoCastTimers.Contains(i))
		{
			AutoCastTimers.Add(i, 0.0f);
		}

		AutoCastTimers[i] -= InDeltaTime;

		if (AutoCastTimers[i] <= 0.0f)
		{
			if (EquippedAbilityHandles.Contains(i))
			{
				FGameplayAbilitySpecHandle Handle = EquippedAbilityHandles[i];
				
				// 어빌리티 실행 시도
				if (ASC->TryActivateAbility(Handle))
				{
					// 발동 성공 시 쿨타임 갱신 (AttackSpeedRate 적용)
					float BaseCoolDown = MainGem->GetGemData().CoolDown;
					
					// 플레이어의 공격 속도 속성 가져오기
					float AttackSpeedRate = 1.0f;
					if (AHBaseCharacter* Character = Cast<AHBaseCharacter>(GetOwner()))
					{
						AttackSpeedRate = Character->GetAttackSpeedRate();
					}

					// 쿨타임 = 기본 쿨타임 / 공격 속도 배율
					AutoCastTimers[i] = BaseCoolDown / FMath::Max(0.1f, AttackSpeedRate);
				}
			}
		}
	}
}

bool UHEquipmentComponent::EquipSupportGem(int32 InSlotIndex, UHSupportGem* InSupportGem)
{
	if (!SlotSupportGems.IsValidIndex(InSlotIndex) || !InSupportGem)
	{
		return false;
	}

	if (AActor* Owner = GetOwner())
	{
		if (!Owner->HasAuthority())
		{
			ServerEquipSupportGemById(InSlotIndex, InSupportGem->GetInstanceId());
			return true;
		}
	}

	// 1. 이미 다른 슬롯에 장착되어 있었다면 해제 (이동 처리)
	for (int32 i = 0; i < SlotSupportGems.Num(); ++i)
	{
		if (SlotSupportGems[i].SupportGems.Contains(InSupportGem))
		{
			UnequipSupportGem(i, InSupportGem, false);
			break;
		}
	}

	// 2. 최대 3개 제한 체크
	if (SlotSupportGems[InSlotIndex].SupportGems.Num() >= 3)
	{
		return false;
	}

	// 3. 슬롯 보조 젬 리스트에 추가
	SlotSupportGems[InSlotIndex].SupportGems.AddUnique(InSupportGem);

	// 4. 인벤토리에서 제거
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
	{
		if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
		{
			InvComp->RemoveGemInstance(InSupportGem);
		}
	}

	// 5. 현재 장착된 메인 젬이 있다면 연결 시도
	if (UHMainGem* MainGem = EquippedMainGems[InSlotIndex])
	{
		MainGem->AddSupportGem(InSupportGem);
	}

	OnEquipmentChanged.Broadcast();
	SyncEquippedGemDataFromInstances();
	return true;
}

void UHEquipmentComponent::UpdateAbilitySpec(int32 InSlotIndex)
{
}

void UHEquipmentComponent::SyncEquippedGemDataFromInstances()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	EquippedMainGemData.SetNum(EquippedMainGems.Num());
	for (int32 i = 0; i < EquippedMainGems.Num(); ++i)
	{
		EquippedMainGemData[i] = EquippedMainGems[i]
			? FHGemInstanceData::FromGemData(EquippedMainGems[i]->GetGemData(), EquippedMainGems[i]->GetInstanceId())
			: FHGemInstanceData();
	}

	SlotSupportGemData.SetNum(SlotSupportGems.Num());
	for (int32 i = 0; i < SlotSupportGems.Num(); ++i)
	{
		SlotSupportGemData[i].SupportGems.Empty();
		for (UHSupportGem* SupportGem : SlotSupportGems[i].SupportGems)
		{
			if (SupportGem)
			{
				SlotSupportGemData[i].SupportGems.Add(FHGemInstanceData::FromGemData(SupportGem->GetGemData(), SupportGem->GetInstanceId()));
			}
		}
	}
}

void UHEquipmentComponent::OnRep_EquippedMainGemData()
{
	RebuildReplicatedDisplayGems();
}

void UHEquipmentComponent::OnRep_SlotSupportGemData()
{
	RebuildReplicatedDisplayGems();
}

void UHEquipmentComponent::RebuildReplicatedDisplayGems()
{
	EquippedMainGems.SetNum(EquippedMainGemData.Num());
	for (int32 i = 0; i < EquippedMainGemData.Num(); ++i)
	{
		const FHGemInstanceData& GemData = EquippedMainGemData[i];
		if (!GemData.IsValid() || GemData.GemCategory != HEGemCategory::Main)
		{
			EquippedMainGems[i] = nullptr;
			continue;
		}

		UHMainGem* NewGem = NewObject<UHMainGem>(this);
		NewGem->SetInstanceId(GemData.InstanceId);
		NewGem->Initialize(GemData.ToDisplayGemData());
		EquippedMainGems[i] = NewGem;
	}

	SlotSupportGems.SetNum(SlotSupportGemData.Num());
	for (int32 i = 0; i < SlotSupportGemData.Num(); ++i)
	{
		SlotSupportGems[i].SupportGems.Empty();
		for (const FHGemInstanceData& SupportGemData : SlotSupportGemData[i].SupportGems)
		{
			if (!SupportGemData.IsValid() || SupportGemData.GemCategory != HEGemCategory::Support) continue;

			UHSupportGem* NewSupportGem = NewObject<UHSupportGem>(this);
			NewSupportGem->SetInstanceId(SupportGemData.InstanceId);
			NewSupportGem->Initialize(SupportGemData.ToDisplayGemData());
			SlotSupportGems[i].SupportGems.Add(NewSupportGem);

			if (EquippedMainGems.IsValidIndex(i) && EquippedMainGems[i])
			{
				EquippedMainGems[i]->AddSupportGem(NewSupportGem);
			}
		}
	}

	OnEquipmentChanged.Broadcast();
}

void UHEquipmentComponent::ServerEquipGemById_Implementation(int32 InSlotIndex, FGuid InGemInstanceId)
{
	if (!EquippedMainGems.IsValidIndex(InSlotIndex) || !InGemInstanceId.IsValid())
	{
		return;
	}

	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
	{
		if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
		{
			for (UHGemBase* Gem : InvComp->GetInventoryGems())
			{
				if (Gem && Gem->GetInstanceId() == InGemInstanceId)
				{
					EquipGem(InSlotIndex, Cast<UHMainGem>(Gem));
					return;
				}
			}
		}
	}

	if (UHMainGem* MainGem = FindMainGemByInstanceId(InGemInstanceId))
	{
		EquipGem(InSlotIndex, MainGem);
		return;
	}
}

void UHEquipmentComponent::ServerEquipSupportGemById_Implementation(int32 InSlotIndex, FGuid InGemInstanceId)
{
	if (!SlotSupportGems.IsValidIndex(InSlotIndex) || !InGemInstanceId.IsValid())
	{
		return;
	}

	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
	{
		if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
		{
			for (UHGemBase* Gem : InvComp->GetInventoryGems())
			{
				if (Gem && Gem->GetInstanceId() == InGemInstanceId)
				{
					EquipSupportGem(InSlotIndex, Cast<UHSupportGem>(Gem));
					return;
				}
			}
		}
	}

	if (UHSupportGem* SupportGem = FindSupportGemByInstanceId(InGemInstanceId))
	{
		EquipSupportGem(InSlotIndex, SupportGem);
		return;
	}
}

void UHEquipmentComponent::ServerUnequipGem_Implementation(int32 InSlotIndex)
{
	if (!EquippedMainGems.IsValidIndex(InSlotIndex))
	{
		return;
	}

	UnequipGem(InSlotIndex);
}

void UHEquipmentComponent::ServerUnequipSupportGem_Implementation(int32 InSlotIndex, FGuid InGemInstanceId)
{
	if (!SlotSupportGems.IsValidIndex(InSlotIndex) || !InGemInstanceId.IsValid())
	{
		return;
	}

	UHSupportGem* SupportGem = FindSupportGemByInstanceId(InGemInstanceId);
	if (!SupportGem || !SlotSupportGems[InSlotIndex].SupportGems.Contains(SupportGem))
	{
		return;
	}

	UnequipSupportGem(InSlotIndex, SupportGem);
}

void UHEquipmentComponent::UnequipSupportGem(int32 InSlotIndex, UHSupportGem* InSupportGem, bool bInReturnToInventory)
{
	if (!SlotSupportGems.IsValidIndex(InSlotIndex) || !InSupportGem) return;

	if (AActor* Owner = GetOwner())
	{
		if (!Owner->HasAuthority())
		{
			ServerUnequipSupportGem(InSlotIndex, InSupportGem->GetInstanceId());
			return;
		}
	}

	// 1. 슬롯 보조 젬 리스트에서 제거
	SlotSupportGems[InSlotIndex].SupportGems.Remove(InSupportGem);

	// 2. 메인 젬이 있다면 거기서도 제거
	if (UHMainGem* MainGem = EquippedMainGems[InSlotIndex])
	{
		MainGem->RemoveSupportGem(InSupportGem);
	}

	// 3. 인벤토리로 반환
	if (bInReturnToInventory)
	{
		if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
		{
			if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
			{
				InvComp->AddGemInstance(InSupportGem);
			}
		}
	}

	OnEquipmentChanged.Broadcast();
	SyncEquippedGemDataFromInstances();
}

void UHEquipmentComponent::UnequipGem(int32 InSlotIndex, bool bInReturnToInventory)
{
	if (!EquippedMainGems.IsValidIndex(InSlotIndex) || !EquippedMainGems[InSlotIndex]) return;

	if (AActor* Owner = GetOwner())
	{
		if (!Owner->HasAuthority())
		{
			ServerUnequipGem(InSlotIndex);
			return;
		}
	}

	UHMainGem* RemovedGem = EquippedMainGems[InSlotIndex];

	// 1. 보조 젬들은 슬롯에 남기고 메인 젬 내부에서만 제거
	for (UHSupportGem* SupportGem : SlotSupportGems[InSlotIndex].SupportGems)
	{
		RemovedGem->RemoveSupportGem(SupportGem);
	}

	// 2. GAS 연동: Ability 제거
	if (ASC && EquippedAbilityHandles.Contains(InSlotIndex))
	{
		ASC->ClearAbility(EquippedAbilityHandles[InSlotIndex]);
		EquippedAbilityHandles.Remove(InSlotIndex);
	}

	// 3. 장착 해제
	EquippedMainGems[InSlotIndex] = nullptr;
	AutoCastTimers.Remove(InSlotIndex);

	// 4. 인벤토리로 반환
	if (bInReturnToInventory)
	{
		if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
		{
			if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
			{
				InvComp->AddGemInstance(RemovedGem);
			}
		}
	}

	OnEquipmentChanged.Broadcast();
	SyncEquippedGemDataFromInstances();
}

UHMainGem* UHEquipmentComponent::GetEquippedGem(int32 InSlotIndex) const
{
	if (EquippedMainGems.IsValidIndex(InSlotIndex))
	{
		return EquippedMainGems[InSlotIndex];
	}
	return nullptr;
}

TArray<UHSupportGem*> UHEquipmentComponent::GetEquippedSupportGems(int32 InSlotIndex) const
{
	TArray<UHSupportGem*> Result;
	if (SlotSupportGems.IsValidIndex(InSlotIndex))
	{
		for (auto& Gem : SlotSupportGems[InSlotIndex].SupportGems)
		{
			Result.Add(Gem);
		}
	}
	return Result;
}

UHMainGem* UHEquipmentComponent::FindMainGemByInstanceId(const FGuid& InInstanceId) const
{
	if (!InInstanceId.IsValid()) return nullptr;

	for (UHMainGem* MainGem : EquippedMainGems)
	{
		if (MainGem && MainGem->GetInstanceId() == InInstanceId)
		{
			return MainGem;
		}
	}

	return nullptr;
}

UHSupportGem* UHEquipmentComponent::FindSupportGemByInstanceId(const FGuid& InInstanceId) const
{
	if (!InInstanceId.IsValid()) return nullptr;

	for (const FHSlotSupportGemList& SupportGemList : SlotSupportGems)
	{
		for (UHSupportGem* SupportGem : SupportGemList.SupportGems)
		{
			if (SupportGem && SupportGem->GetInstanceId() == InInstanceId)
			{
				return SupportGem;
			}
		}
	}

	return nullptr;
}
