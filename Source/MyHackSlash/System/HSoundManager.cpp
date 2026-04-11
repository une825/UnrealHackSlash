#include "System/HSoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "DataAsset/HSoundDataAsset.h"

void UHSoundManager::InitializeManager(UHSoundDataAsset* InConfig)
{
	if (InConfig)
	{
		SoundConfig = InConfig;
	}
}

void UHSoundManager::PlaySoundAtLocationThrottled(USoundBase* InSound, FVector InLocation, float InVolumeMultiplier, float InPitchMultiplier, float InStartTime)
{
	if (!InSound) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 1. Throttling 체크: 특정 사운드가 너무 짧은 간격으로 호출되는지 확인
	if (float* LastPlayed = LastPlayedTimes.Find(InSound))
	{
		if (CurrentTime - (*LastPlayed) < MinPlaybackInterval)
		{
			// 너무 짧은 시간에 반복되면 재생을 스킵하거나 볼륨을 낮춰줍니다.
			return;
		}
	}

	// 2. 사운드 재생
	UGameplayStatics::PlaySoundAtLocation(this, InSound, InLocation, InVolumeMultiplier, InPitchMultiplier, InStartTime);

	// 3. 마지막 재생 시간 업데이트
	LastPlayedTimes.Add(InSound, CurrentTime);
}

void UHSoundManager::PlaySFXByKey(FName InKey, FVector InLocation, float InVolumeMultiplier, bool bIsUI)
{
	if (!SoundConfig) return;

	if (USoundBase* SFX = SoundConfig->GetSFX(InKey))
	{
		if (bIsUI)
		{
			// UI 사운드는 Throttling 없이 즉시 재생 (결과 창 등 중요 사운드)
			if (UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(this, SFX, InLocation, FRotator::ZeroRotator, InVolumeMultiplier))
			{
				AudioComp->SetUISound(true);
			}
		}
		else
		{
			PlaySoundAtLocationThrottled(SFX, InLocation, InVolumeMultiplier);
		}
	}
}

void UHSoundManager::PlaySFX(FGameplayTag InTag, FVector InLocation, float InVolumeMultiplier, bool bIsUI)
{
	if (!SoundConfig) return;

	if (USoundBase* SFX = SoundConfig->GetSound(InTag))
	{
		if (bIsUI)
		{
			if (UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(this, SFX, InLocation, FRotator::ZeroRotator, InVolumeMultiplier))
			{
				AudioComp->SetUISound(true);
			}
		}
		else
		{
			PlaySoundAtLocationThrottled(SFX, InLocation, InVolumeMultiplier);
		}
	}
}

void UHSoundManager::PlayBGM(USoundBase* InBGM, float InFadeInTime, float InFadeOutTime, bool bIsUI)
{
	if (!InBGM) return;

	// 이미 동일한 BGM이 재생 중이라면 무시
	if (CurrentBGMAsset == InBGM && CurrentBGMComponent && CurrentBGMComponent->IsPlaying())
	{
		return;
	}

	// 기존 BGM이 있다면 페이드 아웃 후 정지
	if (CurrentBGMComponent)
	{
		CurrentBGMComponent->FadeOut(InFadeOutTime, 0.0f);
	}

	// 새로운 BGM 재생
	CurrentBGMAsset = InBGM;
	CurrentBGMComponent = UGameplayStatics::SpawnSound2D(this, InBGM, 1.0f, 1.0f, 0.0f, nullptr, true);
	
	if (CurrentBGMComponent)
	{
		CurrentBGMComponent->SetUISound(bIsUI);
		CurrentBGMComponent->FadeIn(InFadeInTime);
	}
}

void UHSoundManager::PlayBGMByKey(FName InKey, float InFadeInTime, float InFadeOutTime, bool bIsUI)
{
	if (!SoundConfig) return;

	if (USoundBase* BGM = SoundConfig->GetBGM(InKey))
	{
		PlayBGM(BGM, InFadeInTime, InFadeOutTime, bIsUI);
	}
}

void UHSoundManager::PlayBGMByTag(FGameplayTag InTag, float InFadeInTime, float InFadeOutTime, bool bIsUI)
{
	if (!SoundConfig) return;

	if (USoundBase* BGM = SoundConfig->GetSound(InTag))
	{
		PlayBGM(BGM, InFadeInTime, InFadeOutTime, bIsUI);
	}
}

void UHSoundManager::StopBGM(float InFadeOutTime)
{
	if (CurrentBGMComponent)
	{
		CurrentBGMComponent->FadeOut(InFadeOutTime, 0.0f);
		CurrentBGMComponent = nullptr;
		CurrentBGMAsset = nullptr;
	}
}

void UHSoundManager::SetBGMVolume(float InVolumeMultiplier)
{
	if (CurrentBGMComponent)
	{
		CurrentBGMComponent->SetVolumeMultiplier(InVolumeMultiplier);
	}
}

void UHSoundManager::SetBGMPitch(float InPitchMultiplier)
{
	if (CurrentBGMComponent)
	{
		CurrentBGMComponent->SetPitchMultiplier(InPitchMultiplier);
	}
}
