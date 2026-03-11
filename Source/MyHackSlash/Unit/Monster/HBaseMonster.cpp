// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Monster/HBaseMonster.h"
#include "Components/CapsuleComponent.h"

AHBaseMonster::AHBaseMonster()
{
}

void AHBaseMonster::SetDead()
{
	Super::SetDead();
	
	EnableRagdoll();

	// 맞은 방향(HitDirection)과 힘(Strength)을 활용
	if (LastDamageCauser.IsValid())
	{
		const float Radius = 500.0f;
		const float Strength = 600.0f;

		//FVector HitLocation = GetActorLocation();
		//FVector HitDirection = HitLocation - LastDamageCauser->GetActorLocation();
		//FVector Impulse = HitDirection * Strength;
		//GetMesh()->AddImpulseAtLocation(Impulse, HitLocation, TEXT("spine_03"));
		
		GetMesh()->AddRadialImpulse(LastDamageCauser->GetActorLocation(), Radius, Strength, ERadialImpulseFalloff::RIF_Linear, true);
	}

	FTimerHandle DeadTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DeadTimerHandle,
		FTimerDelegate::CreateLambda([this]() { Destroy(); }), DeadEventDelayTime, false);
}

void AHBaseMonster::EnableRagdoll()
{
	//// 애니메이션 중지
	//GetMesh()->SetUpdateAnimationInEditor(false);
	//GetMesh()->SetAnimInstanceClass(nullptr);

	//GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll")); // 기본 제공되는 Ragdoll 프로필 권장
	GetMesh()->SetSimulatePhysics(true);

	// 캡슐 컴포넌트는 물리 연산에서 제외 (시체끼리 엉킴 방지)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

float AHBaseMonster::GetAIPatrolRadius()
{
	return 800.0f;
}

float AHBaseMonster::GetAIDetectRange()
{
	return 400.0f;
}

float AHBaseMonster::GetAIAttackRange()
{
	return 10;// Stat->GetTotalStat().AttackRange + Stat->GetAttackRadius() * 2;
}

float AHBaseMonster::GetAITurnSpeed()
{
	return 2.0f;
}

void AHBaseMonster::SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAttackFinished = InOnAttackFinished;
}

void AHBaseMonster::AttackByAI()
{
}
