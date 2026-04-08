# 🌊 Wave & Economy System Design Document (Integrated)

본 문서는 **MyHackSlash** 프로젝트의 무한 맵 및 오브젝트 풀링 시스템과 통합된 웨이브 시스템 및 경제 시스템의 구조를 정의합니다.

---

## 1. Core Enumerations (핵심 열거형)
프로젝트 컨벤션(`EH` 접두사)에 따라 정의합니다.

| Enum Name | Values | Description |
| :--- | :--- | :--- |
| `EHWaveType` | `Battle`, `Boss`, `Reward`, `Shop` | 웨이브의 성격 |
| `EHWaveClearType` | `TimeSurvival`, `KillCount`, `ManualUI` | 웨이브 클리어 조건 |
| `EHWaveState` | `Ready`, `InProgress`, `Completed` | 웨이브 진행 상태 |

---

## 2. Data Structures (데이터 구조)
기획자가 에디터에서 직관적으로 편집할 수 있도록 `UDataAsset` 기반으로 설계합니다.

### 2.1 Wave Data (`FHWaveData`)
| Property Name | Type | Description |
| :--- | :--- | :--- |
| `WaveIndex` | `int32` | 웨이브 단계 (1, 2, 3...) |
| `WaveType` | `EHWaveType` | 웨이브 종류 |
| `ClearType` | `EHWaveClearType` | 클리어 조건 |
| `ClearValue` | `float` | 클리어 기준값 (초 또는 마리 수) |
| `MonsterSpawnerConfig` | `UHMonsterSpawnerDataAsset*` | [전투/보스용] 몬스터 스폰 세팅 |
| `RewardData` | `UPrimaryDataAsset*` | [보상/상점용] 등장할 아이템/상점 세팅 |

### 2.2 Wave Configuration Asset (`UHWaveConfigDataAsset`)
*   **역할**: 전체 웨이브 시퀀스를 관리하는 최상위 데이터 에셋.
*   **필드**: `TArray<FHWaveData> WaveList`

---

## 3. Core Component: UHWaveManager (World Subsystem)

`UHInfiniteMapManager`와 유사하게 `UWorldSubsystem`으로 구현하여 전역적인 접근성을 확보합니다.

### 3.1 주요 기능
*   **Wave Control**: `StartWave(int32 Index)`, `EndWave()`, `NextWave()`
*   **Progress Tracking**: 현재 웨이브의 남은 시간 또는 처치 수를 추적.
*   **System Integration**:
    *   `UHMonsterSpawnManager`: 전투 웨이브 시 몬스터 스폰 요청.
    *   `UHInfiniteMapManager`: 보스 웨이브 시 타일 갱신 일시 정지 등의 연동.
    *   `UHUIManager`: 웨이브 UI(타이머, 단계 표시) 갱신 델리게이트 호출.

### 3.2 이벤트 델리게이트
*   `FOnWaveStarted(int32 Index, EHWaveType Type)`
*   `FOnWaveProgressUpdated(float ProgressPercent)`
*   `FOnWaveCompleted(int32 Index)`

---

## 4. UI Integration: HWaveResultUI (웨이브 결과 UI)

웨이브가 종료되면 플레이어의 성과와 경제적 보상을 시각적으로 표시하고, 다음 단계로 넘어가는 브릿지 역할을 수행합니다.

### 4.1 표시 데이터
*   현재 웨이브 번호 및 타입 (Battle, Boss 등)
*   획득한 이자 골드 (Interest Gold)
*   현재 총 보유 골드 (Total Gold)

### 4.2 게임 흐름 제어 (Flow Control)
1.  **Wave End**: `UHWaveManager::EndWave()` 호출 시 `UGameplayStatics::SetGamePaused(true)`를 통해 게임 세계를 일시 정지합니다.
2.  **UI Show**: `UHUIManager`를 통해 `HWaveResultUI`를 화면에 출력합니다.
3.  **User Interaction**: 플레이어가 UI 배경(또는 스킵 버튼)을 클릭하면 `UHWaveResultUI::OnClickBackground()`가 호출됩니다.
4.  **Resume & Next**:
    *   `SetGamePaused(false)`로 게임을 재개합니다.
    *   `HWaveResultUI`를 숨깁니다.
    *   `UHWaveManager::PrepareNextWave()` 및 `StartWave()`를 호출하여 즉시 다음 웨이브를 시작합니다.

---

## 5. Economy: Interest System (이자 시스템)

웨이브 종료 시점에 플레이어의 자산을 기반으로 보너스 골드를 지급하여 전략적인 저축을 유도합니다.

*   **위치**: `UHWaveManager::CalculateInterest()` (EndWave 내부에서 호출).
*   **로직**:
    1.  `AHPlayerState`에서 현재 `Gold` 잔액 확인.
    2.  `Earned = FMath::Min(FMath::FloorToInt(CurrentGold * InterestRate), MaxInterest)` 계산.
    3.  플레이어에게 골드 추가 및 결과 UI에 데이터 전달.

---

## 6. Implementation Strategy (구현 상태)

1.  **[Done] 기반 타입 정의**: `HGlobalTypes.h` 또는 `HWaveTypes.h`에 Enums 및 Struct 정의.
2.  **[Done] 데이터 에셋 생성**: `UHWaveConfigDataAsset` 클래스 구현 및 에디터 테스트 데이터 생성.
3.  **[Done] 웨이브 매니저 골격**: `UHWaveManager` 서브시스템 생성 및 `StartWave` 기본 루프 구현.
4.  **[Done] 몬스터 스폰 연동**: 기존 `UHMonsterSpawnManager`를 웨이브 시스템에 통합.
5.  **[Done] 이자 시스템 및 UI**: `HWaveResultUI` 구현, 웨이브 클리어 시 일시 정지 및 결과 표시 로직 통합.
