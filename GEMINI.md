# Gemini CLI Instructions

- 모든 대화와 코드 설명은 **한국어**로 진행해 주세요.
- 기술적인 용어는 문맥에 따라 영문을 병기하거나 적절한 한국어 용어를 사용해 주세요.

## 코딩 컨벤션 (Coding Conventions)

### 함수 파라미터 명명 규칙
- 함수의 모든 파라미터에는 용도에 따라 `In` 또는 `Out` 접두사를 붙여야 합니다.
  - 입력 파라미터: `In` (예: `InNewLevel`, `InDamageAmount`)
  - 출력 파라미터: `Out` (예: `OutResult`, `OutHitLocation`)
- 언리얼 엔진의 기존 오버라이드 함수라도 프로젝트 내 구현부에서는 가급적 이 규칙을 준수합니다.
