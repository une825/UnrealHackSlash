# Gemini CLI Instructions

- 모든 대화와 코드 설명은 **한국어**로 진행해 주세요.
- 기술적인 용어는 문맥에 따라 영문을 병기하거나 적절한 한국어 용어를 사용해 주세요.

## 코딩 컨벤션 (Coding Conventions)

- 프로젝트의 명명 규칙, UI 데이터 정의 및 기타 개발 표준은 루트 경로에 있는 **`Project_Standard_Convention.md`**를 최우선으로 참고하여 준수합니다.
- 새로운 클래스나 기능을 추가할 때 해당 문서의 규칙(H 접두사, In/Out 파라미터 등)을 철저히 따릅니다.

## 설계 및 개발 프로세스 (Design & Development Process)

- 코드 분석 및 수정 전에는 반드시 프로젝트 루트 경로에 있는 `~System_Design.md` 및 `Project_Standard_Convention.md` 파일을 먼저 읽고 시스템의 의도를 파악해야 합니다.
- 코드를 수정한 후에는 변경 사항에 맞춰 관련된 `.md` 설계 문서 파일들을 최신 상태로 업데이트해야 합니다.

# 
Behavioral guidelines to reduce common LLM coding mistakes. Merge with project-specific instructions as needed.

**Tradeoff:** These guidelines bias toward caution over speed. For trivial tasks, use judgment.

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:
- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:
- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:
- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:
```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

---

**These guidelines are working if:** fewer unnecessary changes in diffs, fewer rewrites due to overcomplication, and clarifying questions come before implementation rather than after mistakes.