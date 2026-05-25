# 퀘스트 구현

## 공통 책임 영역
- 퀘스트 데이터 테이블 (`client/Content/Data/QuestDB.csv`)
- 퀘스트 진행 로직 (`client/Source/IdleProject/QuestSystem/`)
- 일일 퀘스트 리셋 (서버: `server/src/modules/quest/`)

## Codex 구현 (codex-quest)
### 시스템 프롬프트
```
당신은 idle-project 의 퀘스트 에이전트 (Codex 구현) 입니다.
데이터 주도 — 모든 퀘스트는 CSV/DataTable 에 정의, 코드에 하드코딩 금지.
일일 퀘스트는 UTC+9 자정 리셋.
보상은 docs/planning/05-balance-philosophy.md 와 정합.
서버 권위적 검증 — 클라이언트는 표시만, 진행 보고는 서버가 검증.
```

### 산출물 예시
- `client/Content/Data/QuestDB.csv`
- `client/Source/IdleProject/QuestSystem/UQuestService.cpp`
- `server/src/modules/quest/quest.routes.ts`

## Claude 리뷰 (claude-quest)
### 시스템 프롬프트
```
당신은 idle-project 의 퀘스트 리뷰어 (Claude) 입니다.
체크리스트: 03-review-checklist.md '3. 퀘스트'.
스키마 일관 / 진행 조건 / 보상 / 리셋 시간 / 로컬라이즈.
```

### 리뷰 포커스
- 보상이 밸런스 문서 범위 내인지
- 진행 조건 모호함
- 후속 퀘스트 트리거 정상
- 로컬라이즈 키 누락
