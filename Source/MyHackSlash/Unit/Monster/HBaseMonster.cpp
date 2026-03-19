// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit/Monster/HBaseMonster.h"
#include "Unit/Player/HPlayerCharacter.h"
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
}

float AHBaseMonster::GetExpReward() const
{
	return CurrentMonsterStat.ExpReward;
}

void AHBaseMonster::InitializeStat(int32 InNewLevel)
{
	Super::InitializeStat(InNewLevel);

	if (UnitProfileData && UnitProfileData->UnitType == EUnitType::Monster)
	{
		if (FMonsterStatRow* StatRow = UnitProfileData->GetMonsterStatRowByLevel(Level))
		{
			// 공통 스탯 복사
			CurrentStat = *StatRow;
			CurrentMonsterStat = *StatRow;

			// 체력 초기화
			MaxHP = StatRow->MaxHP;
			CurrentHP = MaxHP;

			GetCharacterMovement()->MaxWalkSpeed = CurrentStat.MovementSpeed;
			
			OnHPChanged.Broadcast(CurrentHP, MaxHP);

			UE_LOG(LogTemp, Log, TEXT("Monster Initialized Level %d (HP: %f)"), Level, MaxHP);
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

void AHBaseMonster::SetAIAttackDelegate(const FAIMonsterAttackFinished& InOnAttackFinished)
{
	OnAttackFinished = InOnAttackFinished;
}

void AHBaseMonster::AttackByAI()
{
	ProcessAttack();
}

void AHBaseMonster::NotifyAttackEnd()
{
	Super::NotifyAttackEnd();

	if (OnAttackFinished.IsBound())
	{
		OnAttackFinished.Execute();
	}
}
