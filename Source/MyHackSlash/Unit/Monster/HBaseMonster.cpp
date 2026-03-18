// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit/Monster/HBaseMonster.h"
#include "System/HMonsterAIController.h"
#include "System/HObjectPoolManager.h" // 오브젝트 풀 매니저 포함
#include "DataAsset/HUnitProfileData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
#include "TimerManager.h"

AHBaseMonster::AHBaseMonster()
{
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

	// 일정 시간 후 오브젝트 풀로 반납 (HBaseCharacter에 정의된 DeadEventDelayTime 사용)
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
		// 풀 매니저가 없으면 그냥 파괴
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
