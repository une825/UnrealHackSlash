#include "DataAsset/HSoundDataAsset.h"
#include "Sound/SoundBase.h"

USoundBase* UHSoundDataAsset::GetBGM(FName InKey) const
{
	if (const TObjectPtr<USoundBase>* BGM = BGMMap.Find(InKey))
	{
		return *BGM;
	}
	return nullptr;
}

USoundBase* UHSoundDataAsset::GetSFX(FName InKey) const
{
	if (const TObjectPtr<USoundBase>* SFX = SFXMap.Find(InKey))
	{
		return *SFX;
	}
	return nullptr;
}
