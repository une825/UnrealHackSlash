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

	// 2. 보상 드랍 (코인)
	if (CoinClass)
	{
		if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
		{
			if (AHCoin* NewCoin = Cast<AHCoin>(PoolManager->SpawnFromPool(CoinClass, GetActorLocation(), GetActorRotation())))
			{
				NewCoin->PrepareFromPool(GoldAmount);
			}
		}
	}

	// 3. 풀로 반납
	ReturnToPool();
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
