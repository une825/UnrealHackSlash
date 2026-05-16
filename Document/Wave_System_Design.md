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

### 3.3 Multiplayer Replication

`UHWaveManager`는 서버 권한 `UWorldSubsystem`으로 웨이브를 진행하되, 클라이언트가 읽어야 하는 표시 상태는 `AHGameState`의 `FHReplicatedWaveState`로 복제합니다. 복제 상태에는 웨이브 상태, 표시 웨이브 번호, 웨이브 타입, 클리어 타입, 진행률, 현재값, 목표값이 포함됩니다.

`UHWaveManager`는 웨이브 초기화, 시작, 진행 업데이트, 종료, 다음 웨이브 준비 시점에 `AHGameState::SetReplicatedWaveState()`를 호출합니다. `UHMainHudUI`는 `AHGameState`의 복제 상태 변경 delegate를 통해 클라이언트 HUD를 갱신하고, 서버 로컬 초기 타이밍에서만 기존 `UHWaveManager` delegate를 보완 경로로 사용합니다.

### 3.4 Replicated Object Pooling

몬스터처럼 게임플레이 결과가 있는 복제 액터를 풀에서 재사용할 때는 서버가 활성화/비활성화 직전에 dormancy를 깨우고 hidden, collision, tick, 위치 변경 후 `ForceNetUpdate()`를 호출합니다. 이 처리는 풀에 반납된 액터가 클라이언트에서 숨김 상태로 남거나 재사용 위치가 늦게 전파되는 문제를 줄이기 위한 필수 정책입니다.

---

## 4. UI Integration: HWaveResultUI (웨이브 결과 UI)

웨이브가 종료되면 플레이어의 성과와 경제적 보상을 시각적으로 표시하고, 다음 단계로 넘어가는 브릿지 역할을 수행합니다.

### 4.1 표시 데이터
*   현재 웨이브 번호 및 타입 (Battle, Boss 등)
*   획득한 이자 골드 (Interest Gold)
*   현재 총 보유 골드 (Total Gold)

### 4.2 게임 흐름 제어 (Flow Control)
1.  **Wave End**: `UHWaveManager::EndWave()` 호출 시 `UGameplayStatics::SetGamePaused(true)`를 통해 게임 세계를 일시 정지합니다.
2.  **UI Show**: 서버가 각 `AMyHackSlashPlayerController`의 `ClientShowWaveResult()`를 호출하고, owning client의 `UHUIManager`가 `HWaveResultUI`를 화면에 출력합니다.
3.  **User Interaction**: 플레이어가 UI 배경(또는 스킵 버튼)을 클릭하면 `UHWaveResultUI::OnClickBackground()`가 호출됩니다.
4.  **Resume & Next**:
    *   `AMyHackSlashPlayerController::ServerContinueWaveFromResult()`가 서버에 다음 웨이브 진행을 요청합니다.
    *   서버가 `SetGamePaused(false)`로 게임을 재개하고 모든 클라이언트에 `ClientHideWaveResult()`를 보냅니다.
    *   서버가 `UHWaveManager::PrepareNextWave()` 및 `StartWave()`를 호출하여 즉시 다음 웨이브를 시작합니다.

---

## 5. Economy: Interest System (이자 시스템)

웨이브 종료 시점에 플레이어의 자산을 기반으로 보너스 골드를 지급하여 전략적인 저축을 유도합니다.

*   **위치**: `UHWaveManager::CalculateInterest()` (EndWave 내부에서 호출).
*   **로직**:
    1.  서버가 모든 `AMyHackSlashPlayerController`를 순회합니다.
    2.  각 플레이어의 `AHPlayerState`에서 현재 `Gold` 잔액을 확인합니다.
    3.  `Earned = FMath::Min(FMath::FloorToInt(CurrentGold * InterestRate), MaxInterest)`를 개인별로 계산합니다.
    4.  해당 플레이어에게 골드를 추가하고 `ClientShowWaveResult()`로 결과 UI 데이터를 전달합니다.

### 5.1 Shop Purchase Authority (상점 구매 권한)

상점 웨이브의 구매 판정은 서버에서만 수행합니다. 클라이언트 UI는 상품 행 이름(`RowName`)만 owning `AMyHackSlashPlayerController`로 전달하고, 서버 RPC가 현재 웨이브의 `ShopRewardTable`에서 행 데이터를 다시 조회합니다.

서버는 현재 웨이브가 `EHWaveType::Shop`인지 확인한 뒤 `AHPlayerState::ConsumeGold()`로 비용을 검증하고, 상품 타입에 따라 스탯 GameplayEffect, 즉시 회복, 스킬 젬 지급을 적용합니다. 클라이언트가 가격이나 효과 값을 직접 보내지 않기 때문에 UI 변조로 구매 결과를 조작할 수 없습니다.

### 5.2 Reward Selection Authority (보상 선택 권한)

보상 선택 UI는 표시용으로 `FHRewardOptionEntry`를 사용하여 보상 데이터와 데이터 테이블 행 이름(`RowName`)을 함께 보관합니다. 선택 시 클라이언트는 `RowName`만 서버 RPC로 보내고, 서버는 `UHSelectAbilityManager`의 보상 테이블에서 행을 다시 조회한 뒤 owning 플레이어에게 보상을 지급합니다.

서버는 선택지 3개를 생성해 owning `AMyHackSlashPlayerController`에 복제하고, UI는 복제된 선택지만 표시합니다. 새로고침도 서버 RPC로 요청하며, 서버는 현재 `PlayerController`에 보관된 선택지에 포함된 `RowName`만 유효한 선택으로 인정합니다.

---

## 6. Implementation Strategy (구현 상태)

1.  **[Done] 기반 타입 정의**: `HGlobalTypes.h` 또는 `HWaveTypes.h`에 Enums 및 Struct 정의.
2.  **[Done] 데이터 에셋 생성**: `UHWaveConfigDataAsset` 클래스 구현 및 에디터 테스트 데이터 생성.
3.  **[Done] 웨이브 매니저 골격**: `UHWaveManager` 서브시스템 생성 및 `StartWave` 기본 루프 구현.
4.  **[Done] 몬스터 스폰 연동**: 기존 `UHMonsterSpawnManager`를 웨이브 시스템에 통합.
5.  **[Done] 이자 시스템 및 UI**: `HWaveResultUI` 구현, 웨이브 클리어 시 일시 정지 및 결과 표시 로직 통합.
6.  **[Done] 상점 구매 서버 RPC**: 상점 UI는 `RowName`만 전송하고 서버가 현재 웨이브 상점 테이블을 재조회하여 골드 소비와 상품 지급을 처리.
7.  **[Done] 보상 선택 서버 RPC 1차 전환**: 보상 UI는 `RowName`만 전송하고 서버가 보상 테이블을 재조회하여 owning 플레이어에게 보상을 지급.
8.  **[Done] 서버 생성 보상 선택지 검증**: 서버가 선택지와 새로고침 횟수를 `PlayerController`에 복제하고, 현재 선택지에 포함된 `RowName`만 허용.
9.  **[Done] 웨이브 표시 상태 GameState 복제**: `FHReplicatedWaveState`를 통해 웨이브 번호, 타입, 클리어 조건, 진행률을 클라이언트 HUD에 전파.
10. **[Done] 웨이브 결과 UI owning client 표시**: 서버가 모든 PlayerController에 개인별 이자를 계산하고 `ClientShowWaveResult()`로 결과 UI를 표시.
