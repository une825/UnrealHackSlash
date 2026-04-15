#include "Item/HPotionItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "System/HObjectPoolManager.h"
#include "Unit/Player/HPlayerCharacter.h"

AHPotionItem::AHPotionItem()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshVisualOffset = FVector(0.0f, 0.0f, 15.0f);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);
	SphereComp->SetSphereRadius(35.0f); // 물약은 코인보다 조금 더 크게 설정
	SphereComp->SetCanEverAffectNavigation(false);

	// 물리 및 충돌 기본값은 꺼둔 상태로 설정 (경고 방지)
	SphereComp->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComp->SetSimulatePhysics(false);
	SphereComp->SetEnableGravity(true);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SphereComp->BodyInstance.bLockXRotation = true;
	SphereComp->BodyInstance.bLockYRotation = true;
	SphereComp->BodyInstance.bLockZRotation = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetRelativeLocation(MeshVisualOffset);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetCanEverAffectNavigation(false);

	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovementComponent->SetUpdatedComponent(MeshComp);
	RotatingMovementComponent->RotationRate = FRotator(0.0f, 180.0f, 0.0f); 

	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AHPotionItem::OnOverlapBegin);
}

void AHPotionItem::BeginPlay()
{
	Super::BeginPlay();
}

void AHPotionItem::PrepareFromPool()
{
	SetActorHiddenInGame(false);
	
	if (SphereComp)
	{
		// 1. 상태 초기화를 위해 물리/충돌 명시적으로 끔
		SphereComp->SetSimulatePhysics(false);
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 2. 충돌 설정 완료
		SphereComp->SetCollisionProfileName(TEXT("PhysicsActor"));
		SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		
		// 3. 속도 및 위치 초기화
		SphereComp->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
		SphereComp->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		if (MeshComp)
		{
			MeshComp->SetRelativeLocation(MeshVisualOffset);
			MeshComp->SetRelativeRotation(FRotator::ZeroRotator);
		}

		// 4. 물리 시뮬레이션 활성화 (충돌이 준비된 상태에서)
		SphereComp->SetSimulatePhysics(true);
		SphereComp->WakeRigidBody();

		// 5. 팝콘 효과
		FVector PopDirection = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 1.5f);
		PopDirection.Normalize();
		float PopForce = FMath::FRandRange(MinPopForce, MaxPopForce);
		
		SphereComp->SetPhysicsLinearVelocity(PopDirection * PopForce);
	}
}

void AHPotionItem::ReturnToPool()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	
	if (SphereComp)
	{
		SphereComp->SetSimulatePhysics(false);
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComp->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	}

	if (UHObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UHObjectPoolManager>())
	{
		PoolManager->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}

void AHPotionItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(OtherActor))
	{
		if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
		{
			// 획득 시 즉시 충돌 비활성화
			SetActorEnableCollision(false);
			if (SphereComp)
			{
				SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				SphereComp->SetSimulatePhysics(false);
			}

			// 회복 효과 적용
			if (HealEffectClass)
			{
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				Context.AddInstigator(this, this);

				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HealEffectClass, 1.0f, Context);
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}

			if (PickupSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
			}

			ReturnToPool();
		}
	}
}
