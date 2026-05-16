# Settings System Design Document

본 문서는 언어와 사운드 설정을 UI에서 변경하고 저장하기 위한 설정 시스템을 정의합니다.

---

## 1. Core Types

| Type | Description |
| :--- | :--- |
| `FHGameSettings` | 언어, 마스터 볼륨, BGM 볼륨, SFX 볼륨, 음소거 값을 담는 설정 데이터 |
| `UHSettingsSaveGame` | `FHGameSettings`를 저장하는 SaveGame 객체 |
| `UHSettingsManager` | 설정 로드/저장과 런타임 적용을 담당하는 GameInstanceSubsystem |
| `UHSettingsUI` | 설정 변경을 위한 UMG 위젯 C++ 베이스 클래스 |

---

## 2. Runtime Flow

1. `UHSettingsManager`가 GameInstanceSubsystem으로 초기화됩니다.
2. `PlayerSettings` SaveGame 슬롯에서 설정을 로드합니다.
3. 언어 설정은 `UHGlobalTextManager::SetActiveLanguage()`로 적용합니다.
4. BGM 볼륨은 현재 재생 중인 컴포넌트에 `UHSoundManager::SetBGMVolume()`로 즉시 적용합니다.
5. 이후 새로 재생되는 BGM/SFX는 `UHSoundManager`가 `UHSettingsManager`의 현재 설정을 조회해 Master/BGM/SFX/Mute 배율을 적용합니다.
   * BGM은 Mute 상태에서 0 볼륨으로 fade된 뒤에도, Mute 해제 시 `AdjustVolume()`으로 현재 컴포넌트 볼륨을 즉시 복원합니다.
6. `UHSettingsUI`는 위젯 입력을 `PendingSettings`에 보관하고, Apply 버튼에서 `UHSettingsManager::ApplySettings()`를 호출합니다.
7. 설정이 변경되면 `OnSettingsChanged`가 브로드캐스트되어 열려 있는 설정 UI가 다시 갱신됩니다.
8. `UHMainHudUI`의 `SettingButton`을 클릭하면 `UHUIManager::ShowWidgetByName(TEXT("SettingsUI"), 100)`로 설정창을 엽니다.
9. 솔로 플레이에서는 `UHSettingsUI`가 열릴 때 현재 pause 상태를 저장하고 게임을 일시정지합니다. 닫힐 때는 열리기 전 pause 상태로 복원합니다.
10. 멀티플레이에서는 설정창을 열어도 서버 게임을 일시정지하지 않습니다. 설정 UI는 해당 로컬 클라이언트의 입력 모드와 위젯 표시만 처리하며, 다른 플레이어와 몬스터/웨이브 진행에는 영향을 주지 않습니다.
    * 구현 기준은 `UWorld::GetNetMode() == NM_Standalone`입니다. Listen Server와 Client에서는 설정 UI가 `SetGamePaused()`를 호출하지 않습니다.

---

## 3. Widget Binding

`UHSettingsUI` 기반 위젯은 다음 이름의 UMG 요소를 바인딩할 수 있습니다. 모두 OptionalWidget입니다.

| Widget Name | Type | Description |
| :--- | :--- | :--- |
| `LanguageComboBox` | `UComboBoxString` | `Korean`, `English` 선택 |
| `MasterVolumeSlider` | `USlider` | 마스터 볼륨 0~1 |
| `BGMVolumeSlider` | `USlider` | BGM 볼륨 0~1 |
| `SFXVolumeSlider` | `USlider` | SFX 볼륨 0~1 |
| `MuteCheckBox` | `UCheckBox` | 전체 음소거 |
| `ApplyButton` | `UButton` | 현재 입력값 적용 및 저장 |
| `ResetButton` | `UButton` | 기본값으로 초기화 및 저장 |
| `CloseButton` | `UButton` | 설정창 닫기 |

텍스트 바인딩은 `TitleText`, `LanguageLabelText`, `SoundLabelText`, `MasterVolumeLabelText`, `BGMVolumeLabelText`, `SFXVolumeLabelText`, `MuteLabelText`, `ApplyButtonText`, `ResetButtonText`, `CloseButtonText`를 사용합니다.

Main HUD에서 설정창을 열려면 `UHMainHudUI` 기반 위젯에 `SettingButton` 이름의 `UButton`을 배치하고, `UHUIDataAsset`의 `WidgetList`에 `SettingsUI` 키로 설정 위젯 블루프린트를 등록해야 합니다.

---

## 4. Sound Limitations

현재 `UHSoundManager`를 통해 재생되는 BGM/SFX는 `UHSettingsManager`의 저장된 볼륨 설정을 적용합니다. 단, `UHSoundManager`를 거치지 않고 `UGameplayStatics` 등으로 직접 재생하는 사운드는 이 설정을 자동으로 따르지 않습니다.
