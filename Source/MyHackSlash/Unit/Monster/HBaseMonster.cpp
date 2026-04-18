// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit/Monster/HBaseMonster.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Item/HCoin.h"
#include "AbilitySystemComponent.h"
#include <AbilitySystemBlueprintLibrary.h>
#include "System/HMonsterAIController.h"
#include "System/HObjectPoolManager.h"
#include "DataAsset/HUnitProfileData.h"
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
	return AttributeSet ? AttributeSet->GetExpReward() : 0.0f;
}

void AHBaseMonster::InitializeStat(int32 InNewLevel)
{
	// Super::InitializeStat에서 InitStatEffect를 실행함.
	Super::InitializeStat(InNewLevel);

	if (AttributeSet)
	{
		OnHPChanged.Broadcast(GetCurrentHP(), GetMaxHP());
		//UE_LOG(LogTemp, Log, TEXT("Monster Initialized Level %d (HP: %f, ExpReward: %f)"), 
		//	InNewLevel, GetMaxHP(), AttributeSet->GetExpReward());
	}
}

void AHBaseMonster::ResetCharacter()
{
	Super::ResetCharacter();

	// AI를 다시 연결하여 동작하게 함
	SpawnDefaultController();
}

void AHBaseMonster::SetDead()
{
	Super::SetDead();

	// AI 로직 완전히 중지 및 포커스 해제
	if (auto* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();
		AICon->ClearFocus(EAIFocusPriority::Gameplay); // 모든 우선순위 포커스 해제
		AICon->UnPossess(); // 컨트롤러와 폰 연결 끊기 (AI 로직 완전 정지)
	}

	// 가해자에게 사망 이벤트 전송 (GAS 기반 경험치 및 퀘스트 처리)
	if (LastDamageCauser.IsValid())
	{
		AActor* Killer = LastDamageCauser.Get();
		
		// 가해자의 ASC를 찾아서 이벤트를 보냅니다.
		if (UAbilitySystemComponent* KillerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Killer))
		{
			FGameplayEventData Payload;
			Payload.Instigator = this;
			Payload.Target = Killer;
			Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Character.MonsterKilled"));
			Payload.EventMagnitude = GetExpReward(); // 보상 경험치를 Magnitude에 담음

			KillerASC->HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		// 기존 델리게이트 호출 (레거시/퀘스트 매니저 연동용 유지)
		OnMonsterDead.Broadcast(Killer, this);
	}

	// 코인 드랍
	if (CoinClass && AttributeSet)
	{
		float GoldReward = AttributeSet->GetGoldReward();
		if (GoldReward > 0)
		{
			if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
			{
				if (AHCoin* NewCoin = Cast<AHCoin>(PoolManager->SpawnFromPool(CoinClass, GetActorLocation(), GetActorRotation())))
				{
					NewCoin->PrepareFromPool(GoldReward);
				}
			}
			else if (AHCoin* NewCoin = GetWorld()->SpawnActor<AHCoin>(CoinClass, GetActorLocation(), GetActorRotation()))
			{
				NewCoin->SetGoldAmount(GoldReward);
			}
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
