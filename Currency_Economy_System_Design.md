# 💰 Currency & Economy System Design

본 문서는 **MyHackSlash** 프로젝트의 재화(Currency) 및 경제(Economy) 시스템 설계를 정의합니다. 플레이어의 자원 획득, 소비, 데이터 동기화 구조를 설명합니다.

---

## 1. 개요 (Overview)

본 프로젝트의 경제 시스템은 플레이어의 성장과 선택의 재미를 극대화하기 위해 설계되었습니다. 재화는 크게 **영구 보관형 재화**와 **세션 한정형 재화**로 분류됩니다.

---

## 2. 재화 종류 (Currency Types)

### 2.1 골드 (Gold)
*   **성격**: 플레이어의 성장을 지원하는 핵심 재화.
*   **관리 주체**: `AHPlayerState` (`CurrentGold`)
*   **획득 경로**:
    *   보상 팝업 (`UHSelectAbilityPopupUI`) 선택지 보상.
    *   몬스터 처치 및 퀘스트 보상 (추후 확장 예정).
*   **사용처**: 상점 구매, 업그레이드 (추후 확장 예정).
*   **데이터 동기화**: `FOnGoldChanged` 델리게이트를 통해 UI(`UHMainHudUI`)에 실시간 반영.

### 2.2 새로고침 횟수 (Refresh Count / Reroll)
*   **성격**: 보상 선택 시 무작위성을 제어하기 위한 세션 한정형 자원.
*   **관리 주체**: `UHSelectAbilityManager` (`CurrentRefreshCount`)
*   **초기화**: 레벨업 팝업 초기화 시 또는 게임 시작 시 `ResetRefreshCount()` 호출.
*   **획득 경로**: 보상 팝업 내 `EHRewardType::GetReroll` 타입 보상 획득 시 증가.
*   **사용처**: 보상 팝업(`UHSelectAbilityPopupUI`)의 새로고침 버튼 클릭 시 소모.

---

## 3. 시스템 아키텍처 (System Architecture)

### 3.1 클래스별 역할 (Responsibilities)

| 클래스 | 역할 |
| :--- | :--- |
| **`AHPlayerState`** | 플레이어의 영구적/세션 재화 데이터(골드 등)를 보관하고 변경 이벤트를 발행. |
| **`UHSelectAbilityManager`** | 보상 테이블 기반의 확률 로직을 수행하며, 세션 내 새로고침 횟수를 관리. |
| **`UHMainHudUI`** | 플레이어의 재화 상태를 실시간으로 모니터링하고 시각적으로 표시. |
| **`UHSelectAbilityPopupUI`** | 새로고침 버튼 인터랙션을 처리하고, 매니저를 통해 자원 소모 및 갱신 수행. |

### 3.2 재화 획득 워크플로우 (Reward Workflow)
1.  **Reward Trigger**: 유저가 보상 선택지에서 특정 보상(골드, 리롤 등)을 클릭.
2.  **Manager Execution**: `UHSelectAbilityManager::ExecuteReward()` 호출.
3.  **Data Update**:
    *   **Gold**: `PlayerState->AddGold()` 호출 -> 델리게이트 브로드캐스트.
    *   **Reroll**: `Manager->CurrentRefreshCount` 증가.
4.  **UI Feedback**: `MainHud` 또는 `PopupUI`가 변경된 데이터를 감지하여 텍스트 갱신.

---

## 4. UI 및 사용자 경험 (UI & UX)

### 4.1 실시간 갱신 (Event-Driven UI)
*   모든 재화 표시는 틱(Tick)이 아닌 **델리게이트 기반**으로 업데이트됩니다.
*   `AHPlayerState::OnGoldChanged`에 `UHMainHudUI::RefreshGold`를 바인딩하여 성능 최적화와 즉각적인 반응성을 확보합니다.

### 4.2 인터랙션 제어
*   새로고침 횟수가 0인 경우, `UHSelectAbilityPopupUI`의 새로고침 버튼은 비활성화(`SetIsEnabled(false)`) 처리됩니다.
*   남은 횟수는 `RefreshCountText`를 통해 사용자에게 명시적으로 표시됩니다.

---

## 5. 향후 확장 계획 (Future Extensions)

*   **다양한 재화 추가**: 젬 업그레이드 전용 보석, 유료 재화(필요 시) 등.
*   **경제 밸런싱**: 등급별 골드 지급량 밸런스 조정 (`FHRewardOptionData` 테이블).
*   **상점 시스템**: 획득한 골드를 소비할 수 있는 인게임/아웃게임 상점 구현.
