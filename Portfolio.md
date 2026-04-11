# [Portfolio] HackSlash - UE5 Action RPG

## 1. 프로젝트 개요
*   **프로젝트 성격**: Unreal Engine 5 기반 하이엔드 핵앤슬래시 액션 RPG
*   **주요 목표**: 대규모 적을 상대하는 전투 메커니즘 구현 및 유연한 스킬/보상 시스템 구축
*   **핵심 기술**: C++, Gameplay Ability System (GAS), Data-Driven Design, Enhanced Input

## 2. 핵심 기술적 성과

### 🚀 Gameplay Ability System (GAS) 기반 전투 프레임워크
*   **복잡한 데미지 계산 로직 구현**: `UGameplayEffectExecutionCalculation`을 상속받은 `HExecutionCalculation_Damage`를 통해 공격자의 스탯(공격력, 치명타율)과 배율을 결합한 정교한 데미지 산출 로직 구축.
*   **데이터 중심의 이펙트 관리**: `SetByCaller` 방식을 활용하여 코드와 블루프린트 간의 의존성을 낮추고, 런타임에 유연하게 수치를 조정할 수 있는 `GameplayEffect` 구조 설계.
*   **확장성 있는 속성 정의**: `HCharacterAttributeSet`을 통해 체력, 마나, 공격력 등 캐릭터 필수 스탯을 관리하고 Replicated 변수를 고려한 기초 설계 수행.

### 💎 데이터 주도형 스킬 젬 및 보상 시스템
*   **Modular Skill System**: 젬(Gem) 아이템을 장착하여 스킬을 획득하는 시스템을 `DataAsset` 기반으로 구현하여 기획자가 코드 수정 없이 새로운 스킬을 쉽게 추가할 수 있는 환경 제공.
*   **확률 기반 보상 매니저**: `UHSelectAbilityManager`를 통해 등급별(Gold, Silver, Bronze) 보상 생성 및 가중치 기반 랜덤 선택 로직 구현.
*   **이벤트 기반 통신**: `GameplayEvent`와 `Payload`를 활용하여 UI 선택 결과가 실제 캐릭터의 스탯이나 스킬 획득으로 이어지는 느슨한 결합(Loose Coupling) 구조 설계.

### 🛠 아키텍처 및 최적화
*   **컴포넌트 기반 설계**: 인벤토리 및 스킬 관리를 독립적인 `ActorComponent`로 분리하여 코드 재사용성 및 유지보수성 향상.
*   **UI/로직 분리**: UI 매니저 패턴을 도입하여 복잡한 팝업 및 보상 선택 인터페이스와 게임 로직 사이의 명확한 책임 분리.

## 3. 트러블슈팅 및 문제 해결

### Case 1: 런타임 동적 GameplayTag 생성 시 크래시 해결
*   **문제**: `FGameplayTag::RequestGameplayTag`를 통해 동적으로 태그를 생성할 때, 존재하지 않는 태그 경로 요청 시 `block index out of range` 크래시 발생.
*   **분석**: 엔진 내부적으로 태그 테이블에 등록되지 않은 문자열을 잘못된 형식으로 요청할 때 발생하는 안정성 문제 확인.
*   **해결**: 태그 생성 전 `IsNone()` 유효성 검사 추가 및 `RequestGameplayTag`의 `ErrorIfNotFound` 파라미터를 `false`로 설정하여 안전한 폴백(Fallback) 로직 구현. 시스템 안정성 대폭 향상.

### Case 2: GameplayEffect 데이터 미설정(SetByCaller) 에러 대응
*   **문제**: `SetByCaller`를 기대하는 GE에서 데이터가 입력되지 않아 발생하는 `GetMagnitude` 에러 해결.
*   **해결**: `MakeOutgoingSpec` 호출 후 `SetSetByCallerMagnitude`를 반드시 거치도록 로직을 정교화하고, `ExecutionCalculation` 내부에서 데이터 유무를 방어적으로 체크하도록 개선.

## 4. 학습 및 성장 경험
*   **엔진 심화 이해**: 단순 블루프린트 사용을 넘어 GAS의 내부 동작 방식과 C++ 프레임워크의 연동 과정을 깊이 있게 학습.
*   **유지보수 고려**: 프로젝트 규모가 커짐에 따라 `Project_Standard_Convention`을 정의하고 준수하며 협업 및 장기적 유지보수를 위한 코드 품질 관리의 중요성 체득.
