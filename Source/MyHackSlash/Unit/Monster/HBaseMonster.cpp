// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit/Monster/HBaseMonster.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Item/HCoin.h"
#include "AbilitySystemComponent.h"
#include "System/HMonsterAIController.h"
#include "System/HObjectPoolManager.h"
#include "DataAsset/HUnitProfileData.h"
#include "DataAsset/HMonsterStatRow.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
#include "TimerManager.h"

AHBaseMonster::AHBaseMonster()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UHCharacterAttributeSet>(TEXT("AttributeSet"));
}

float AHBaseMonster::GetExpReward() const
{
	return CurrentMonsterStat.ExpReward;
}

void AHBaseMonster::InitializeStat(int32 InNewLevel)
{
	Super::InitializeStat(InNewLevel);

	if (UnitProfileData && UnitProfileData->UnitType == EHUnitType::Monster)
	{
		if (FMonsterStatRow* StatRow = UnitProfileData->GetMonsterStatRowByLevel(Level))
		{
			// 공통 스탯 복사
			CurrentStat = *StatRow;
			CurrentMonsterStat = *StatRow;

			// 체력 초기화 (AttributeSet 사용)
			if (AttributeSet)
			{
				AttributeSet->SetMaxHealth(StatRow->MaxHP);
				AttributeSet->SetHealth(StatRow->MaxHP);
			}

			GetCharacterMovement()->MaxWalkSpeed = CurrentStat.MovementSpeed;
			
			OnHPChanged.Broadcast(GetCurrentHP(), GetMaxHP());

			UE_LOG(LogTemp, Log, TEXT("Monster Initialized Level %d (HP: %f)"), Level, GetMaxHP());
		}
	}
}

void AHBaseMonster::SetDead()
{
	Super::SetDead();

	// AI 및 이동 로직 중지
	if (auto* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();
	}

	// 가해자가 있는 경우 델리게이트 호출 (QuestManager 등에서 처리)
	if (LastDamageCauser.IsValid())
	{
		OnMonsterDead.Broadcast(LastDamageCauser.Get(), this);
	}

	// 코인 드랍
	if (CoinClass)
	{
		if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
		{
			if (AHCoin* NewCoin = Cast<AHCoin>(PoolManager->SpawnFromPool(CoinClass, GetActorLocation(), GetActorRotation())))
			{
				NewCoin->PrepareFromPool(CurrentMonsterStat.GoldReward);
			}
		}
		else if (AHCoin* NewCoin = GetWorld()->SpawnActor<AHCoin>(CoinClass, GetActorLocation(), GetActorRotation()))
		{
			NewCoin->SetGoldAmount(CurrentMonsterStat.GoldReward);
		}
	}

	// 일정 시간 후 오브젝트 풀로 반납
	FTimerHandle ReturnTimerHandle;
	GetWorldTimerManager().SetTimer(ReturnTimerHandle, FTimerDelegate::CreateUObject(this, &AHBaseMonster::ReturnToPool), DeadEventDelayTime, false);
}

void AHBaseMonster::ReturnToPool()
{
	if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
	{
		PoolManager->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}

void AHBaseMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AHBaseMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// 초기 어빌리티 부여
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityClass);
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}
}

void AHBaseMonster::SetAIAttackDelegate(const FAIMonsterAttackFinished& InOnAttackFinished)
{
	OnAttackFinished = InOnAttackFinished;
}

void AHBaseMonster::AttackByAI()
{
	// 기존 ProcessAttack() 대신 GAS 기반의 Attack() 호출
	Attack();
}

void AHBaseMonster::NotifyAttackEnd()
{
	Super::NotifyAttackEnd();

	if (OnAttackFinished.IsBound())
	{
		OnAttackFinished.Execute();
	}
}
