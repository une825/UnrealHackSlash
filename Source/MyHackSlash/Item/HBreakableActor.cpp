// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/HBreakableActor.h"
#include "Item/HCoin.h"
#include "System/HObjectPoolManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attribute/HCharacterAttributeSet.h"
#include "DataAsset/HBreakableRewardDataAsset.h"
#include "Item/HMagnetItem.h"
#include "Item/HPotionItem.h"

AHBreakableActor::AHBreakableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);
	BoxComponent->SetCollisionProfileName(TEXT("BlockAll"));
	BoxComponent->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	BoxComponent->SetMobility(EComponentMobility::Movable);

	// 플레이어 공격 채널(HAttack)에 반응하도록 설정
	BoxComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(BoxComponent);
	
	// 캐릭터 공격에 감지되도록 설정
	MeshComponent->SetCollisionProfileName(TEXT("NoCollision")); // 실제 충돌은 BoxComponent가 담당
	MeshComponent->SetMobility(EComponentMobility::Movable);

	// GAS 컴포넌트 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UHCharacterAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AHBreakableActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AHBreakableActor::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// 사망(파괴) 태그 감시 바인딩
		FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(TEXT("Object.State.Broken"));
		AbilitySystemComponent->RegisterGameplayTagEvent(DeadTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AHBreakableActor::OnDeadTagChanged);
	}

	PrepareFromPool();
}

void AHBreakableActor::OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0 && !bIsBroken)
	{
		OnBreak();
	}
}

void AHBreakableActor::PrepareFromPool()
{
	bIsBroken = false;
	
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 스탯 초기화
	if (AbilitySystemComponent)
	{
		// 사망 태그 제거 (풀 재사용 시 중요)
		AbilitySystemComponent->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Object.State.Broken")));

		if (InitStatEffect)
		{
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddInstigator(this, this);

			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitStatEffect, 1.0f, EffectContext);
			if (SpecHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
		else if (AttributeSet)
		{
			// 기본 체력 설정 (임시 폴백)
			AttributeSet->SetMaxHealth(50.0f);
			AttributeSet->SetHealth(50.0f);
		}
	}
}

void AHBreakableActor::OnBreak()
{
	if (bIsBroken) return;
	bIsBroken = true;

	// 1. 피드백 (VFX/SFX)
	if (BreakEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BreakEffect, GetActorLocation());
	}
	if (BreakSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BreakSound, GetActorLocation());
	}

	// 2. 보상 드랍
	DropReward();

	// 3. 풀로 반납
	ReturnToPool();
}

void AHBreakableActor::DropReward()
{
	if (!RewardDataAsset) return;

	FHBreakableRewardEntry SelectedReward;
	if (RewardDataAsset->GetRandomReward(SelectedReward))
	{
		if (SelectedReward.RewardClass)
		{
			// 드랍 개수 결정
			int32 Count = FMath::RandRange(SelectedReward.MinAmount, SelectedReward.MaxAmount);
			UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>();

			for (int32 i = 0; i < Count; ++i)
			{
				// 약간의 랜덤 위치 분산
				FVector SpawnLoc = GetActorLocation() + FVector(0, 0, DropHeightOffset);
				SpawnLoc += FVector(FMath::FRandRange(-30.0f, 30.0f), FMath::FRandRange(-30.0f, 30.0f), 0.0f);

				AActor* NewItem = nullptr;
				if (PoolManager)
				{
					// 풀 매니저를 통한 스폰 시도
					NewItem = PoolManager->SpawnFromPool(SelectedReward.RewardClass, SpawnLoc, FRotator::ZeroRotator);
				}
				
				if (!NewItem)
				{
					// 풀링 대상이 아니거나 매니저가 없는 경우 일반 스폰
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					SpawnParams.Owner = this;
					
					NewItem = GetWorld()->SpawnActor<AActor>(SelectedReward.RewardClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
				}

				if (NewItem)
				{
					// 코인인 경우 기본 금액 설정
					if (AHCoin* Coin = Cast<AHCoin>(NewItem))
					{
						Coin->PrepareFromPool(20);
					}
					// 자석인 경우 초기화 호출
					else if (AHMagnetItem* Magnet = Cast<AHMagnetItem>(NewItem))
					{
						Magnet->PrepareFromPool();
					}
					// 포션인 경우 초기화 호출
					else if (AHPotionItem* Potion = Cast<AHPotionItem>(NewItem))
					{
						Potion->PrepareFromPool();
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("HBreakableActor: Failed to spawn reward item of class %s"), *SelectedReward.RewardClass->GetName());
				}
			}
		}
	}
}

void AHBreakableActor::ReturnToPool()
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
