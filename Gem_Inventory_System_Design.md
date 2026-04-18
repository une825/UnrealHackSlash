# 💎 Skill Gem Inventory & Equipment System Structure

본 문서는 스킬 젬의 보관(Inventory)과 장착(Equipment) 시스템의 클래스 간 관계와 관리 방식을 정의합니다.

## 1. Class Diagram & Hierarchy

### 1.1 Data Layer (UPrimaryDataAsset)
*   **UHGemDataAsset**: 젬의 정적 데이터를 정의합니다. (이름, 아이콘, 속성, GA 클래스 등)

### 1.2 Instance Layer (UObject)
*   **UHGemBase**: 런타임에 생성되는 젬의 기본 인스턴스입니다. (Main/Support 분기)

### 1.3 Management Layer (UActorComponent)
*   **UHGemInventoryComponent (가방)**: 플레이어가 소유한 젬 인스턴스들을 보관합니다. 장착 시 여기서 제거되고, 해제 시 다시 추가됩니다.
*   **UHEquipmentComponent (장착 슬롯)**: 현재 전투에 사용 중인 젬 슬롯(최대 4개)을 관리합니다. 장착 시 GAS 능력을 부여하고 `InputID`를 할당합니다.

---

## 2. Core Workflows

### 2.1 젬의 소유권 이전 (Ownership Transfer)
*   **Equip (인벤토리 -> 장착)**: `Inventory->RemoveGemInstance()` 호출 후 `Equipment->EquipGem()` 실행.
*   **Unequip (장착 -> 인벤토리)**: `Equipment->UnequipGem()` 내부에서 기존 젬을 `Inventory->AddGemInstance()`로 반환.

### 2.2 GAS 연동 및 입력 매핑
*   **InputID Mapping**: 슬롯 인덱스에 따라 `InputID`를 부여합니다. (Slot 0 -> InputID 1, Slot 1 -> InputID 2 등)
*   **Dynamic Update**: 젬 교체 시 기존 능력을 `ClearAbility`하고 새 능력을 `GiveAbility` 함으로써 실시간 스킬 교체를 지원합니다.

### 2.3 젬 자동 업그레이드 워크플로우 (Auto-Upgrade)
*   **Trigger**: 인벤토리에 새로운 젬 인스턴스가 추가(`AddGem`)되거나, 기존 인스턴스가 인벤토리로 반환(`AddGemInstance`)될 때마다 실행됩니다.
*   **Logic (CheckAndUpgradeGems)**:
    1.  **통합 그룹화**: 인벤토리 내의 젬뿐만 아니라 **현재 장착 중인 메인 젬**까지 포함하여 `GemID`와 `Tier`를 기준으로 그룹화합니다.
    2.  **업그레이드 조건**: 동일한 ID와 티어를 가진 젬이 **3개 이상** 모이고, `GemCollection`에 다음 티어 데이터가 정의되어 있을 경우 합성을 진행합니다.
    3.  **장착 우선순위 (Smart Upgrade)**: 
        *   합성 재료 중 장착 중인 젬이 포함되어 있다면, 업그레이드 결과물을 **기존 장착 슬롯에 자동으로 재장착**합니다.
        *   이때, 기존 메인 젬에 연결되어 있던 **보조 젬(Support Gems) 리스트를 새 젬으로 전이**시켜 전투 연속성을 유지합니다.
    4.  **인스턴스 교체**: 하위 티어 인스턴스 3개를 제거하고, 상위 티어의 새 `UObject` 인스턴스를 생성하여 초기화합니다.
    5.  **연쇄 업그레이드 (Chain Upgrade)**: `do-while` 루프를 통해 업그레이드 결과물이 다시 3개가 되어 추가 합성이 가능한 경우, 최종 단계까지 연속적으로 수행합니다.
*   **Notification**: 업그레이드 완료 시 `OnGemUpgraded` 및 `OnGemInventoryUpdated` 이벤트를 발생시켜 UI와 관련 시스템에 알립니다.

---

## 3. UI System: Drag & Drop

### 3.1 Drag Phase (`UHGemInventoryEntryUI`, `UHEquipGemSlotUI`)
*   사용자가 인벤토리 또는 장착 슬롯의 젬 아이콘을 드래그하면 `UHGemDragDropOp`이 생성됩니다.
*   **Payload**: 드래그 중인 `UHGemBase` 인스턴스 정보와 시작된 슬롯 위치(`SourceSlotIndex`)가 담깁니다.
*   **Visual**: 젬의 아이콘이 마우스 커서를 따라다니도록 `DefaultDragVisual`이 설정됩니다.

### 3.2 Drop Phase (`UHEquipGemSlotUI`, `UHGemInventoryEntryUI`)
*   **장착 슬롯에 드롭 (`UHEquipGemSlotUI`)**: 인벤토리에서 온 젬을 해당 슬롯에 장착(`EquipGem`)하거나 메인 젬에 연결합니다.
*   **인벤토리에 드롭 (`UHGemInventoryEntryUI`)**: 장착 슬롯에서 온 젬을 해제(`UnequipGem`)하고 인벤토리로 반환합니다. 보조 젬인 경우 해당 메인 젬과의 연결만 끊고 인벤토리에 추가됩니다.

---

## 4. Communication (UI & Logic)
*   **Delegates**: 
    *   `OnGemInventoryUpdated`: 인벤토리 구성 변화 시 호출.
    *   `OnEquipmentChanged`: 장착 슬롯 구성 변화 시 호출.
*   **Data Driven**: 모든 UI 구성 요소는 `UHGemDataAsset`의 데이터를 기반으로 실시간 갱신됩니다.
