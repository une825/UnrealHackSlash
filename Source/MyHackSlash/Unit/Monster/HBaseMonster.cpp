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

void AHBaseMonster::InitializeStat(int32 NewLevel)
{
	Super::InitializeStat(NewLevel);

	if (UnitProfileData && UnitProfileData->UnitType == EUnitType::Monster)
	{
		if (FMonsterStatRow* StatRow = UnitProfileData->GetMonsterStatRowByLevel(Level))
		{
			CurrentStat = *StatRow;
			CurrentMonsterStat = *StatRow;

			GetCharacterMovement()->MaxWalkSpeed = CurrentStat.MovementSpeed;
			UE_LOG(LogTemp, Log, TEXT("Monster Initialized Level %d"), Level);
		}
	}
}

void AHBaseMonster::SetDead()
{
	if (IsDead) return;

	IsDead = true;

	// AI 및 이동 로직 중지
	if (auto* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();
	}
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	GetCharacterMovement()->DisableMovement();

	// 가해자가 플레이어인 경우 경험치 지급
	if (LastDamageCauser.IsValid())
	{
		if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(LastDamageCauser.Get()))
		{
			Player->AddExp(CurrentMonsterStat.ExpReward);
		}
	}

	// 래그돌 활성화
	EnableRagdoll();

	// 공격자(가해자) 정보를 기반으로 임펄스 적용
	float FinalImpulseForce = 500.0f; 
	FVector ImpulseDir = -GetActorForwardVector(); 

	if (LastDamageCauser.IsValid())
	{
		if (AHBaseCharacter* Attacker = Cast<AHBaseCharacter>(LastDamageCauser.Get()))
		{
			if (UHUnitProfileData* AttackerProfile = Attacker->GetUnitProfileData())
			{
				FinalImpulseForce = AttackerProfile->DeathImpulseForce;
			}
		}

		ImpulseDir = GetActorLocation() - LastDamageCauser->GetActorLocation();
		ImpulseDir.Normalize();
	}

	ImpulseDir.Z = 0.5f; 
	ImpulseDir.Normalize();
	GetMesh()->AddImpulse(ImpulseDir * FinalImpulseForce, NAME_None, true);

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

void AHBaseMonster::EnableRagdoll()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), true, true);
	
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
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
