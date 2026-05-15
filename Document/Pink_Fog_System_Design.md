# 🌸 Pink Fog System Design Document

본 문서는 **MyHackSlash** 프로젝트의 돌발 이벤트 시스템인 '핑크 안개(Pink Fog)' 시스템의 설계를 정의합니다.

---

## 1. 개요 (Overview)

'핑크 안개'는 전투 중 일정 확률로 발생하는 환경 이벤트입니다. 전장이 핑크색 안개로 뒤덮이며, 몬스터들이 광폭화되는 대신 처치 시 획득하는 보상이 대폭 증가하는 **하이리스크-하이리턴(High Risk, High Return)** 기믹을 제공합니다.

---

## 2. 핵심 메커니즘 (Core Mechanics)

### 2.1 발생 및 해제 (Trigger & Dissipation)
*   **발생 조건**: 
    *   전투 웨이브(`EHWaveType::Battle`) 진행 중 일정 시간 간격(`OccurrenceCheckInterval`)마다 확률적으로 발생.
    *   웨이브 시작 시 체크 타이머가 가동되며, 안개가 발동되거나 웨이브가 종료될 때까지 지속적으로 체크함.
*   **지속 시간**: 발생 후 30초~60초간 유지 (데이터 에셋에서 설정).
*   **단계 변화**:
    1.  **Fading In (5s)**: 경고 메시지와 함께 안개가 서서히 짙어짐.
    2.  **Active**: 안개가 완전히 끼고 강화 효과가 적용됨.
    3.  **Fading Out (5s)**: 안개가 서서히 걷히며 원래 상태로 복구.

### 2.2 게임플레이 효과 (Gameplay Effects)
*   **몬스터 강화 (Monster Buff)**:
    *   `GE_PinkFog_MonsterBuff`: 공격력 +50%, 이동속도 +30%, 공격속도 +20% 증가.
    *   외형적으로 핑크색 오라 또는 메쉬 색상 변경(Material Parameter) 적용.
*   **보상 강화 (Reward Boost)**:
    *   `GE_PinkFog_RewardBoost`: 골드 및 경험치 획득량 2배 증가.
    *   안개 지속 시간 동안 발생하는 모든 처치 이벤트에 적용.

---

## 3. 시각 및 연출 (Visuals & Presentation)

### 3.1 환경 변화 (Environment)
*   **Post-Process**: 핑크색 컬러 그레이딩 및 틴트(Tint) 적용.
*   **Exponential Height Fog**: 안개의 밀도를 높이고 색상을 핑크색(#FF69B4)으로 변경.
*   **Niagara**: 화면 전체에 흩날리는 핑크색 꽃잎 또는 안개 입자 효과 추가.

### 3.2 UI 피드백
*   **Warning Overlay**: 안개 발생 시 화면 중앙에 "핑크 안개가 전장을 뒤덮습니다!" 경고 문구 출력.
*   **Status Icon**: 안개 활성 시간 동안 HUD에 핑크 안개 아이콘 및 남은 시간 표시.

---

## 4. 기술적 설계 (Technical Implementation)

### 4.1 HPinkFogManager (World Subsystem)
전역적인 안개 상태와 로직을 관리합니다.
*   **State Management**: `EPinkFogState { Inactive, FadingIn, Active, FadingOut }` 관리.
*   **Timer**: 안개의 지속 시간 및 페이드 시간 처리.
*   **Broadcasting**: 상태 변경 시 전용 델리게이트(`OnPinkFogStateChanged`) 호출.

### 4.2 GAS Integration
*   **Gameplay Tags**:
    *   `State.Event.PinkFog.Active`: 안개 활성화 여부 판별.
    *   `Effect.PinkFog.MonsterBuff`: 몬스터 강화 GE 태그.
*   **Global Multiplier**: `HCharacterAttributeSet`에서 보상 계산 시 `State.Event.PinkFog.Active` 태그 존재 여부를 체크하여 배율 적용.

### 4.3 데이터 에셋 (HPinkFogConfigDataAsset)
*   `OccurrenceProbability`: 발생 확률.
*   `ActiveDuration`: 지속 시간.
*   `FadeDuration`: 페이드 인/아웃 시간.
*   `MonsterBuffEffect`: 적용할 GE 클래스.
*   `RewardMultiplier`: 보상 배율 수치.

---

## 5. 작업 우선순위 (Implementation Roadmap)

1.  **[Stage 1] 시스템 골격**: `HPinkFogManager` 서브시스템 및 데이터 에셋 정의.
2.  **[Stage 2] 시각 효과**: 포스트 프로세스 및 안개 색상 제어 로직 구현.
3.  **[Stage 3] GAS 연동**: 몬스터 버프 GE 제작 및 보상 배율 로직 수정.
4.  **[Stage 4] UI & 연출**: 경고 문구 및 상태 표시 UI 추가.
