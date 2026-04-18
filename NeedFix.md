# 🛠️ 일반 시스템 개선 및 불필요 코드 정리 (Need Fix)

본 문서는 GAS 외의 일반적인 시스템(UI, Manager, Inventory 등)에서 발견된 개선 필요 사항과 구조적 결합도를 낮추기 위한 작업 목록을 정의합니다.

---

## 1. UI 및 데이터 흐름 개선

### 1.1 상점 구매 로직의 하드코딩 문자열 매칭
*   **현상**: `UHShopEntryUI::OnSelectButtonClicked`에서 `ItemTag.ToString().Contains(TEXT("AttackDamage"))`와 같이 문자열 포함 여부로 속성을 판별함.
*   **문제점**: 태그 이름 변경 시 오작동 위험이 크며, GAS의 의도(태그 기반 매칭)에 어긋남.
*   **개선**: `FGameplayTag`를 직접 비교하거나, 태그와 `FGameplayAttribute`를 매핑하는 데이터 에셋(또는 전역 맵)을 구축하여 타입 안정성 확보.

### 1.2 상점 즉시 효과(InstantEffect)의 데이터 주도형 전환
*   **현상**: 포션 회복 등의 효과가 코드 내에 직접 구현되어 있음.
*   **개선**: 각 `InstantEffect` 타입을 처리하는 전용 `UGameplayEffect` 클래스를 데이터 에셋에 지정하거나, 태그별 처리 핸들러를 등록하는 방식으로 유연성 증대.

### 1.3 UI 전반의 매직 넘버 전수 조사
*   **현상**: 아직 일부 UI 클래스에 하드코딩된 수치(슬롯 개수, 크기 등)가 남아있을 수 있음.
*   **개선**: `MaxInventorySlotCount` 사례와 같이 에디터에서 제어 가능한 변수로 전환하여 유지보수성 향상.

---

## 2. 요약: 현재 진행 상황 (Checklist)
- [ ] **상점 상품 판별 로직에서 하드코딩 문자열 제거 (태그 기반 매칭으로 개선)**
- [ ] **UI 전반의 매직 넘버 전수 조사 및 에디터 변수화**
- [ ] **상점 즉시 효과 처리 로직의 데이터 주도형(GE 연동 등) 리팩토링**

---
## 🏁 일반 시스템 개선 리포트 (General Systems Report)

1.  **사운드 시스템**: `FGameplayTag` 기반 관리로 전환 완료.
2.  **보상 및 상점 시스템**: 
    *   `Infinite GE`를 통한 영구적 스탯 보너스 구조 적용 완료 (레벨업 시 스탯 유지).
    *   `HSelectAbilityManager` 보상 로직 순수화 완료.
3.  **젬 시스템**: 
    *   인벤토리 UI 매직 넘버 제거 및 변수화 완료.
    *   `CheckAndUpgradeGems` 함수의 복잡도 개선 및 로직 분리(Gather, Consume, Distribute) 리팩토링 완료.

---
*최종 업데이트: 2026-04-18*
*작성자: Gemini CLI (AI Assistant)*
