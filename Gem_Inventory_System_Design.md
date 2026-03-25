# 💎 Skill Gem Inventory System Structure

본 문서는 스킬 젬 시스템의 클래스 간 관계와 인벤토리 관리 방식을 정의합니다.

## 1. Class Diagram & Hierarchy

### 1.1 Data Layer (UPrimaryDataAsset)
*   **UHGemDataAsset**: 젬의 정적 데이터를 정의합니다. (이름, 아이콘, 속성, GA 클래스 등)

### 1.2 Instance Layer (UObject)
*   **UHGemBase**: 런타임에 생성되는 젬의 기본 인스턴스입니다.
    *   **UHMainGem**: 액티브 스킬을 담당하며, 보조 젬 슬롯(`TArray<UHSupportGem*>`)을 가집니다.
    *   **UHSupportGem**: 패시브 효과를 담당하며, 메인 젬과의 호환성(`CanAttach`)을 검사합니다.

### 1.3 Management Layer (UActorComponent)
*   **UHGemInventoryComponent**: 캐릭터의 젬 보유 현황과 장착 상태를 총괄합니다.

---

## 2. Core Workflows

### 2.1 젬 생성 및 획득 (AddGem)
1. `UHGemDataAsset`을 인자로 전달받습니다.
2. `GemCategory`에 따라 `UHMainGem` 또는 `UHSupportGem` 인스턴스를 `NewObject`로 생성합니다.
3. 생성된 인스턴스를 `InventoryGems` 배열에 추가하여 관리합니다.

### 2.2 메인 젬 장착 및 GAS 연동 (EquipMainGem)
1. 인벤토리에 있는 `UHMainGem`을 선택한 슬롯(Index 0~3)에 배치합니다.
2. 장착 시 해당 젬의 `SkillAbilityClass`를 소유자의 `AbilitySystemComponent(ASC)`에 등록(`GiveAbility`)합니다.
3. 반환된 `FGameplayAbilitySpecHandle`을 저장하여, 장착 해제 시 해당 능력을 정확히 제거(`ClearAbility`)할 수 있도록 합니다.

### 2.3 보조 젬 연결 (AttachSupportToMain)
1. `UHSupportGem`이 대상 `UHMainGem`과 호환되는지 `CanAttach()`를 통해 확인합니다.
2. 호환될 경우 메인 젬의 `SupportGems` 리스트에 추가합니다.
3. (추후 구현) 보조 젬의 수치에 따라 메인 젬의 스킬 효과(데미지, 범위 등)를 실시간으로 보정합니다.

---

## 3. Communication (UI & Logic)
*   **Delegates**: `OnGemInventoryUpdated` 멀티캐스트 델리게이트를 통해 인벤토리의 변화(획득, 장착, 연결)를 UI(`HSkillGemInventory`)에 알립니다.
*   **Data Driven**: 모든 UI 아이콘과 젬의 속성은 `UHGemDataAsset`을 통해 참조하므로, 에디터에서 데이터 수정 시 즉시 반영됩니다.
