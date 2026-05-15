#include "System/HPinkFogManager.h"
#include "DataAsset/HPinkFogConfigDataAsset.h"
#include "System/HWaveManager.h"
#include "System/HMonsterSpawnManager.h"
#include "Unit/Monster/HBaseMonster.h"
#include "AbilitySystemComponent.h"
#include "Mode/MyHackSlashGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void UHPinkFogManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHPinkFogManager::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TickTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(OccurrenceCheckTimerHandle);
	}

	if (SpawnedFogNiagara)
	{
		SpawnedFogNiagara->DestroyComponent();
		SpawnedFogNiagara = nullptr;
	}

	Super::Deinitialize();
}

void UHPinkFogManager::StartPinkFog()
{
	if (CurrentState != EHPinkFogState::Inactive) return;

	UE_LOG(LogTemp, Warning, TEXT("UHPinkFogManager::StartPinkFog - Starting Event"));

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(OccurrenceCheckTimerHandle);
	}

	if (!Config)
	{
		if (AMyHackSlashGameMode* GameMode = Cast<AMyHackSlashGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			Config = GameMode->GetPinkFogConfig();
		}
	}

	if (!Config)
	{
		UE_LOG(LogTemp, Error, TEXT("UHPinkFogManager: Config is null. Cannot start Pink Fog."));
		return;
	}

	CacheVisualActors();
	SetState(EHPinkFogState::FadingIn);
}

void UHPinkFogManager::StopPinkFog()
{
	if (CurrentState == EHPinkFogState::Inactive) return;
	SetState(EHPinkFogState::FadingOut);
}

float UHPinkFogManager::GetRewardMultiplier() const
{
	if (CurrentState == EHPinkFogState::Active && Config)
	{
		return Config->RewardMultiplier;
	}
	return 1.0f;
}

void UHPinkFogManager::OnWaveStarted(int32 Index, EHWaveType Type, EHWaveClearType ClearType)
{
	// 전투 웨이브에서만 확률적으로 발생 체크 시작
	if (Type != EHWaveType::Battle) return;

	// Config 로드 시도
	if (!Config)
	{
		if (AMyHackSlashGameMode* GameMode = Cast<AMyHackSlashGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			Config = GameMode->GetPinkFogConfig();
		}
	}

	if (Config)
	{
		// 이미 안개가 활성화되어 있거나 체크 중이면 무시
		if (CurrentState != EHPinkFogState::Inactive || OccurrenceCheckTimerHandle.IsValid()) return;

		// 일정 시간마다 발생 여부 체크 타이머 시작
		GetWorld()->GetTimerManager().SetTimer(OccurrenceCheckTimerHandle, this, &UHPinkFogManager::CheckPinkFogOccurrence, Config->OccurrenceCheckInterval, true);
		
		UE_LOG(LogTemp, Log, TEXT("Pink Fog occurrence check started. Interval: %f"), Config->OccurrenceCheckInterval);
	}
}

void UHPinkFogManager::OnWaveCompleted(int32 Index)
{
	// 웨이브 종료 시 체크 타이머 중지
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(OccurrenceCheckTimerHandle);
	}
}

void UHPinkFogManager::CheckPinkFogOccurrence()
{
	if (!Config || CurrentState != EHPinkFogState::Inactive)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(OccurrenceCheckTimerHandle);
		}
		return;
	}

	float Rand = FMath::FRand();
	if (Rand <= Config->OccurrenceProbability)
	{
		UE_LOG(LogTemp, Log, TEXT("Pink Fog event triggered during battle wave!"));
		StartPinkFog();
	}
}

void UHPinkFogManager::OnMonsterSpawned(AHBaseMonster* Monster)
{
	if (!Monster || !Config || !Config->MonsterBuffEffect) return;

	// 안개 활성 상태(FadingIn 포함)라면 스폰 즉시 버프 적용
	if (CurrentState == EHPinkFogState::Active || CurrentState == EHPinkFogState::FadingIn)
	{
		if (UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent())
		{
			// [Clean Fix] 중복 적용 방지: 이미 버프 태그를 가지고 있다면 스킵
			FGameplayTag BuffTag = FGameplayTag::RequestGameplayTag(TEXT("Effect.PinkFog.MonsterBuff"));
			if (ASC->GetTagCount(BuffTag) > 0) return;

			float SpeedBefore = Monster->GetMovementSpeed();
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			ASC->ApplyGameplayEffectToSelf(Config->MonsterBuffEffect.GetDefaultObject(), 1.0f, Context);
			float SpeedAfter = Monster->GetMovementSpeed();

			UE_LOG(LogTemp, Log, TEXT("UHPinkFogManager: [Spawn] Applied Buff to %s. Speed: %f -> %f"), 
				*Monster->GetName(), SpeedBefore, SpeedAfter);
		}
	}
}

void UHPinkFogManager::SetState(EHPinkFogState NewState)
{
	if (CurrentState == NewState) return;

	EHPinkFogState OldState = CurrentState;
	CurrentState = NewState;
	StateTimer = 0.0f;

	// 상태 변경 알림
	OnPinkFogStateChanged.Broadcast(CurrentState);

	// 몬스터 버프 처리
	if (CurrentState == EHPinkFogState::FadingIn && OldState == EHPinkFogState::Inactive)
	{
		// 안개 시작 시 기존 모든 몬스터에게 버프 적용
		TArray<AActor*> Monsters;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHBaseMonster::StaticClass(), Monsters);

		FGameplayTag BuffTag = FGameplayTag::RequestGameplayTag(TEXT("Effect.PinkFog.MonsterBuff"));

		for (AActor* Actor : Monsters)
		{
			if (AHBaseMonster* Monster = Cast<AHBaseMonster>(Actor))
			{
				if (UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent())
				{
					// [Clean Fix] 중복 적용 방지
					if (ASC->GetTagCount(BuffTag) > 0) continue;

					FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
					ASC->ApplyGameplayEffectToSelf(Config->MonsterBuffEffect.GetDefaultObject(), 1.0f, Context);
				}
			}
		}
	}
	else if (CurrentState == EHPinkFogState::Inactive)
	{
		// 안개 종료 시 모든 몬스터에게서 버프 제거
		TArray<AActor*> Monsters;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHBaseMonster::StaticClass(), Monsters);

		for (AActor* Actor : Monsters)
		{
			if (AHBaseMonster* Monster = Cast<AHBaseMonster>(Actor))
			{
				if (UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent())
				{
					float SpeedBefore = Monster->GetMovementSpeed();
					FGameplayEffectQuery Query;
					Query.EffectDefinition = Config->MonsterBuffEffect;
					int32 RemovedCount = ASC->RemoveActiveEffects(Query);
					float SpeedAfter = Monster->GetMovementSpeed();
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("UHPinkFogManager: Monster %s has NO ASC!"), *Monster->GetName());
				}
			}
		}
	}

	// Tick 시작/종료 제어
	if (CurrentState != EHPinkFogState::Inactive)
	{
		if (!TickTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().SetTimer(TickTimerHandle, this, &UHPinkFogManager::TickPinkFog, 0.016f, true);
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(TickTimerHandle);
		UpdateVisuals(0.0f); // 완전히 끄기
	}
}

void UHPinkFogManager::TickPinkFog()
{
	float DeltaTime = 0.016f; // 약 60fps 간이 틱

	switch (CurrentState)
	{
	case EHPinkFogState::FadingIn:
		UpdateFading(DeltaTime);
		break;
	case EHPinkFogState::Active:
		UpdateActive(DeltaTime);
		break;
	case EHPinkFogState::FadingOut:
		UpdateFading(DeltaTime);
		break;
	default:
		break;
	}
}

void UHPinkFogManager::UpdateFading(float InDeltaTime)
{
	if (!Config) return;

	StateTimer += InDeltaTime;
	float Alpha = FMath::Clamp(StateTimer / Config->FadeDuration, 0.0f, 1.0f);

	if (CurrentState == EHPinkFogState::FadingIn)
	{
		CurrentFogIntensity = Alpha;
		UpdateVisuals(CurrentFogIntensity);

		if (StateTimer >= Config->FadeDuration)
		{
			SetState(EHPinkFogState::Active);
		}
	}
	else if (CurrentState == EHPinkFogState::FadingOut)
	{
		CurrentFogIntensity = 1.0f - Alpha;
		UpdateVisuals(CurrentFogIntensity);

		if (StateTimer >= Config->FadeDuration)
		{
			SetState(EHPinkFogState::Inactive);
		}
	}
}

void UHPinkFogManager::UpdateActive(float InDeltaTime)
{
	if (!Config) return;

	CurrentFogIntensity = 1.0f;
	UpdateVisuals(CurrentFogIntensity);

	StateTimer += InDeltaTime;
	if (StateTimer >= Config->ActiveDuration)
	{
		SetState(EHPinkFogState::FadingOut);
	}
}

void UHPinkFogManager::UpdateVisuals(float InIntensity)
{
	if (!Config) return;

	// 1. Exponential Height Fog 제어 (Volumetric Fog 활용)
	if (CachedFogActor)
	{
		if (UExponentialHeightFogComponent* FogComp = CachedFogActor->GetComponent())
		{
			// 안개 색상을 서서히 핑크로 변경
			FLinearColor TargetColor = Config->FogColor;
			FLinearColor CurrentColor = FMath::Lerp(FLinearColor::White, TargetColor, InIntensity);
			
			FogComp->SetFogInscatteringColor(CurrentColor);
			
			// 밀도 제어
			float BaseDensity = 0.02f;
			float TargetDensity = 0.35f; // 기존 0.12f에서 대폭 상향
			FogComp->SetFogDensity(FMath::Lerp(BaseDensity, TargetDensity, InIntensity));

			// [Visual Upgrade] Volumetric Fog 활성화 및 속성 조절
			bool bWantVolumetric = (InIntensity > 0.01f);
			if (FogComp->bEnableVolumetricFog != bWantVolumetric)
			{
				FogComp->SetVolumetricFog(bWantVolumetric);
			}

			if (bWantVolumetric)
			{
				// 산란광 색상 및 강도 조절
				FogComp->VolumetricFogAlbedo = CurrentColor.ToFColor(true);
				FogComp->SetVolumetricFogScatteringDistribution(FMath::Lerp(0.2f, 0.7f, InIntensity)); // 빛 산란 정도
				FogComp->SetVolumetricFogExtinctionScale(FMath::Lerp(1.0f, 10.0f, InIntensity)); // 기존 4.0f에서 10.0f로 상향 (안개 입자 농도)
			}
		}
	}

	// 2. Niagara Particle 효과 제어
	if (Config->FogNiagaraSystem)
	{
		if (InIntensity > 0.01f)
		{
			if (!SpawnedFogNiagara)
			{
				// 플레이어 카메라 근처 혹은 월드 중앙 등에 스폰 (보통 카메라 부착 혹은 전역 범위 시스템 권장)
				SpawnedFogNiagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Config->FogNiagaraSystem, FVector::ZeroVector);
				if (SpawnedFogNiagara)
				{
					SpawnedFogNiagara->SetAbsolute(true, true, true); // 월드 고정
				}
			}

			if (SpawnedFogNiagara)
			{
				// 나이아가라 파라미터 업데이트 (강도에 따라 입자 수 조절 등)
				SpawnedFogNiagara->SetVariableFloat(TEXT("FogIntensity"), InIntensity);
				SpawnedFogNiagara->SetVariableLinearColor(TEXT("FogColor"), Config->FogColor);
			}
		}
		else if (SpawnedFogNiagara)
		{
			SpawnedFogNiagara->Deactivate();
			SpawnedFogNiagara = nullptr;
		}
	}

	// 3. Post Process 제어 (Bloom, Vignette 추가)
	if (CachedPPVolume)
	{
		// Bloom 강화 (몽환적인 분위기)
		CachedPPVolume->Settings.bOverride_BloomIntensity = (InIntensity > 0.0f);
		CachedPPVolume->Settings.BloomIntensity = FMath::Lerp(0.675f, 2.5f, InIntensity);

		// Vignette (시야를 좁혀 안개 속에 있는 느낌 강화)
		CachedPPVolume->Settings.bOverride_VignetteIntensity = (InIntensity > 0.0f);
		CachedPPVolume->Settings.VignetteIntensity = FMath::Lerp(0.0f, 0.8f, InIntensity);

		CachedPPVolume->Priority = InIntensity > 0.0f ? 100.0f : -1.0f;
	}
}

void UHPinkFogManager::CacheVisualActors()
{
	if (!GetWorld()) return;

	// 이미 캐싱되어 있다면 스킵
	if (CachedFogActor && CachedPPVolume) return;

	TArray<AActor*> FoundActors;
	
	// 1. Fog 찾기
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		CachedFogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
	}

	// 2. PostProcess 찾기
	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), FoundActors);
	for (AActor* Actor : FoundActors)
	{
		APostProcessVolume* PPV = Cast<APostProcessVolume>(Actor);
		if (PPV && PPV->bUnbound) // 무한 범위 포스트 프로세스 우선 선택
		{
			CachedPPVolume = PPV;
			break;
		}
	}
}
