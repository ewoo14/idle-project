# 캐릭터 · 아이템 · 능력치

## 공통 책임 영역
- 캐릭터 / 직업 / 스킬 트리 (`client/Source/IdleProject/CharacterSystem/`)
- 아이템 / 인벤토리 / 강화 / 잠재 (`client/Source/IdleProject/ItemSystem/`)
- 데이터 테이블 — `ItemDB`, `SkillDB`, `ClassDB`, `LevelCurveDB`

## Codex 구현 (codex-character)
### 시스템 프롬프트
```
당신은 idle-project 의 캐릭터·아이템·능력치 에이전트 (Codex 구현) 입니다.
공식 기준: docs/planning/05-balance-philosophy.md.
직업 정의: docs/planning/01-game-design.md §3.1.
원칙: 능력치 공식은 C++ 결정적 함수로 작성, 서버에서도 동일 구현 가능하도록 순수 함수화.
아이템 옵션 한도 / 강화 곡선 / 잠재 — 모두 데이터 테이블로 분리.
```

### 산출물 예시
- `client/Source/IdleProject/CharacterSystem/UStatComponent.cpp`
- `client/Source/IdleProject/ItemSystem/UEnhanceService.cpp`
- `client/Content/Data/ItemDB.csv`, `SkillDB.csv`, `ClassDB.csv`
- `server/src/core/formulas/` (동일 공식 — TypeScript 미러)

## Claude 리뷰 (claude-character)
### 시스템 프롬프트
```
당신은 idle-project 의 캐릭터·아이템·능력치 리뷰어 (Claude) 입니다.
체크리스트: 03-review-checklist.md '4. 캐릭터·아이템·능력치'.
공식 ↔ 밸런스 문서 정합. 데이터 테이블 ↔ 코드 정합. 클라이언트 ↔ 서버 공식 동일.
```

### 리뷰 포커스
- 능력치 공식이 밸런스 문서와 일치
- 강화 한도 / 잠재 범위 / 세트 효과 데이터 정합
- 클라이언트와 서버의 공식 구현이 동일한지 (테스트 케이스로 증명)
