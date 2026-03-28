#include "UI/HDamageTextActor.h"
#include "Components/WidgetComponent.h"
#include "UI/HDamageTextWidget.h"
#include "System/HObjectPoolManager.h"

AHDamageTextActor::AHDamageTextActor()
{
	PrimaryActorTick.bCanEverTick = true;

	DamageWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidgetComponent"));
	RootComponent = DamageWidgetComponent;

	// 화면 공간에 위젯을 띄우도록 설정 (성능 및 시인성 향상)
	DamageWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidgetComponent->SetDrawAtDesiredSize(true);
}

void AHDamageTextActor::BeginPlay()
{
	Super::BeginPlay();
}

void AHDamageTextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 위로 서서히 떠오르는 연출
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Z += FloatingSpeed * DeltaTime;
	SetActorLocation(CurrentLocation);

	ElapsedTime += DeltaTime;
	if (ElapsedTime >= LifeTime)
	{
		// 시간이 다 되면 오브젝트 풀로 반환
		if (UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>())
		{
			Pool->ReturnToPool(this);
		}
	}
}

void AHDamageTextActor::InitializeDamageText(float InDamage, bool bInIsCritical)
{
	ElapsedTime = 0.0f;
	
	if (UHDamageTextWidget* DamageWidget = Cast<UHDamageTextWidget>(DamageWidgetComponent->GetUserWidgetObject()))
	{
		DamageWidget->SetDamage(InDamage, bInIsCritical);
	}
}
