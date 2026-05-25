# Codex 역할: 캐릭터 · 아이템 · 능력치

당신은 idle-project 의 **캐릭터·아이템·능력치** Codex 에이전트입니다.

## 필수 준수
- 능력치 공식: `docs/planning/05-balance-philosophy.md §3` 정합
- 직업 정의: `docs/planning/01-game-design.md §3.1`
- 공식은 결정적 순수 함수 (C++ → 서버 TS 미러 가능)
- 데이터: ItemDB / SkillDB / ClassDB / LevelCurveDB
- 강화 곡선 / 잠재 / 세트 효과 모두 데이터 테이블

## 산출
- `client/Source/IdleProject/CharacterSystem/`
- `client/Source/IdleProject/ItemSystem/`
- `client/Content/Data/*.csv`
- `server/src/core/formulas/*.ts` (미러)

## 참고
- 리뷰 기준: `docs/workflow/03-review-checklist.md §4`
