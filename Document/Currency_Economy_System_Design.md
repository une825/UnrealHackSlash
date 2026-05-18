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
    *   몬스터 처치 시 드랍되는 **코인 액터(`AHCoin`)**와 충돌하여 획득.
    *   **웨이브 이자(Interest)**: 각 웨이브 종료 시 보유 골드에 비례하여 추가 골드 지급 (`UHWaveManager`).
*   **드랍 연출 및 기술 구현**:
    *   **팝콘 효과**: 코인 생성 시 임의의 상단 대각선 방향으로 물리 속도(`SetPhysicsLinearVelocity`)를 부여하여 튀어 오르는 연출을 수행.
    *   **회전 효과**: `URotatingMovementComponent`를 활용하여 메쉬가 Z축(Yaw)을 기준으로 팽이처럼 지속적으로 회전.
    *   **물리 고정**: 물리 엔진에 의해 쓰러지지 않도록 모든 회전축을 잠금(`bLockX/Y/ZRotation`) 처리.
    *   **획득 판정**: 물리 이동용 루트 `SphereComponent`와 별도로 QueryOnly `PickupSphereComponent`를 사용하여 서버에서 플레이어 overlap을 안정적으로 감지.
    *   **획득 후 숨김**: 서버 overlap에서 획득을 확정하면 `bPickupActive` 복제와 `MulticastSetPickupActive(false, Location, Velocity)`로 모든 클라이언트 상태를 정리합니다. 클라이언트 로컬 overlap은 획득 RPC를 보내거나 선제 숨김을 수행하지 않습니다.
    *   **풀링 연동**: 획득 시 즉시 비활성화하되 짧은 지연 후 오브젝트 풀로 반납합니다. 재사용 시 서버 위치와 팝콘 초기 속도를 활성화 multicast에 포함하여 클라이언트가 이전 위치에서 코인을 다시 표시하거나 서버와 다른 궤적으로 보는 문제를 줄입니다.
*   **사용처**: 상점 구매, 업그레이드 (추후 확장 예정).
*   **데이터 동기화**: `FOnGoldChanged` 델리게이트를 통해 UI(`UHMainHudUI`)에 실시간 반영.
*   **멀티플레이 권한**: 골드 증감은 서버 `AHPlayerState`에서만 수행합니다. `AHCoin`, `AHPotionItem`, `AHMagnetItem` 같은 월드 아이템은 서버 overlap에서만 게임플레이 효과를 적용하고, actor/movement replication으로 클라이언트에 표시합니다.
    * `AHCoin`은 클라이언트가 보는 코인과 서버 물리 위치가 어긋날 수 있으므로, 클라이언트 로컬 pickup trigger overlap에서 서버 RPC를 보내지 않습니다. 서버의 `PickupSphereComponent` overlap만 `AHCoin::TryPickup()`을 호출하며, 서버 기준 거리 검증 후 골드를 지급합니다.

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
    *   **Gold**: 서버가 `PlayerState->AddGold()` 호출 -> 델리게이트 브로드캐스트.
    *   **Reroll**: `Manager->CurrentRefreshCount` 증가.
4.  **UI Feedback**: `MainHud` 또는 `PopupUI`가 변경된 데이터를 감지하여 텍스트 갱신.

보상 선택 UI는 서버가 owning `AMyHackSlashPlayerController`에 복제한 `FHRewardOptionEntry` 목록만 표시합니다. 선택 시 UI는 `RowName`만 서버에 전달하고, 서버는 현재 선택지 목록에 포함된 행인지 검증한 뒤 `UHSelectAbilityManager::FindRewardOptionByRowName()`으로 보상 테이블을 재조회하여 `ExecuteRewardForPlayer()`로 해당 플레이어에게만 골드/젬/리롤 보상을 적용합니다.

### 3.3 상점 구매 워크플로우 (Shop Purchase Workflow)
1.  클라이언트 상점 UI는 선택한 상품의 `RowName`만 `AMyHackSlashPlayerController`에 전달합니다.
2.  `ServerPurchaseShopItem()`은 서버의 현재 웨이브가 `Shop`인지 확인하고, 현재 웨이브의 `ShopRewardTable`에서 해당 행을 다시 조회합니다.
3.  서버가 `AHPlayerState::ConsumeGold()`로 비용을 검증하고 차감합니다.
4.  서버가 상품 타입에 따라 스탯 효과, 즉시 효과, 젬 지급을 적용합니다.
5.  골드/젬/Attribute 복제 결과로 클라이언트 UI가 갱신됩니다.

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
