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

## 4. Economy: Interest System (이자 시스템)

웨이브 종료 시점에 플레이어의 자산을 기반으로 보너스 골드를 지급합니다.

*   **위치**: `UHWaveManager::EndWave()` 내부에서 호출.
*   **로직**:
    1.  `AHPlayerState`에서 현재 `Gold` 잔액 확인.
    2.  `Earned = FMath::Min(FMath::FloorToInt(CurrentGold * InterestRate), MaxInterest)` 계산.
    3.  플레이어에게 골드 추가 및 UI 알림 발송.

---

## 5. Implementation Strategy (구현 우선순위)

1.  **기반 타입 정의**: `HGlobalTypes.h` 또는 `HWaveTypes.h`에 Enums 및 Struct 정의.
2.  **데이터 에셋 생성**: `UHWaveConfigDataAsset` 클래스 구현 및 에디터 테스트 데이터 생성.
3.  **웨이브 매니저 골격**: `UHWaveManager` 서브시스템 생성 및 `StartWave` 기본 루프 구현.
4.  **몬스터 스폰 연동**: 기존 `UHMonsterSpawnManager`를 웨이브 시스템에 통합.
5.  **이자 시스템 및 UI**: 웨이브 클리어 시 골드 지급 로직 및 간단한 UI 연동.
