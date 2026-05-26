# PR #48 기획서 — 스킬 무한 랭크 (무한 성장)

> 무한 성장([[project-infinite-growth]]) 마지막 명확한 캡. 스킬 랭크(#27)는 **MaxRank 5** 로 낮다. **50랭크**로 확장해 스킬 포인트(레벨업당 1, 레벨 무한)를 무한히 투자할 수 있게 한다. 랭크당 데미지 +10%(무한)·쿨다운 -5%(floor 0.1로 하한). 저랭크(0~5) 동작은 기존과 동일(회귀 안전). client+server+UI+balance+qa 5-team.

## 1. 목표 / DoD
스킬을 +50랭크까지 올릴 수 있고, 랭크가 오를수록 스킬 데미지가 무한히 커지며(쿨다운은 하한까지 감소), HUD 에 rank/50 으로 표시된다.

### DoD 검증
1. MaxRank 5→50. CanRankUp/RankUp 가 0~50 동작(스킬 포인트 소모).
2. GetEffectiveDamageCoeff = DamageCoeff × (1 + rank×0.1) — rank 50 = ×6.0(무한 선형).
3. GetEffectiveCooldown = max(0.1, Cooldown × (1 - rank×0.05)) — rank 20+ 하한 0.1(쿨다운은 캡, 데미지는 무한).
4. 저랭크(0~5) 데미지/쿨다운 기존과 동일(회귀). HUD rank/50 표기. 서버 SkillRank 스케일 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN.

## 2. 범위 (In Scope)
### 2.1 랭크 캡 확장 (메인, C++)
- `USkillComponent::MaxRank` 5→50(SkillComponent.h). GetSkillRank clamp(0,MaxRank), CanRankUp(rank<MaxRank && SkillPoints>0) 그대로(캡만 확장).
- GetEffectiveDamageCoeff ×(1+rank×0.1) / GetEffectiveCooldown max(0.1, ×(1-rank×0.05)) 공식 유지(50랭크까지 자연 적용). MaxRank 하드코딩/가정 호출부(HUD, 테스트) 점검.
### 2.2 서버 미러 (백엔드)
- `server/src/core/formulas/skillRank.ts`: MAX_SKILL_RANK=50, getEffectiveDamageCoeff(baseCoeff, rank)=baseCoeff×(1+rank×0.1)(Math.fround float parity), getEffectiveCooldown(baseCd, rank)=max(0.1, baseCd×(1-rank×0.05))(fround) 미러 + parity 테스트. (클라 USkillComponent 스케일 공식과 일치.)
### 2.3 UI (디자이너)
- 스킬 패널(#27) 랭크 표기 "rank/50"(MaxRank 참조, 하드코딩 5 제거). 랭크업 버튼/포인트 표시 그대로. 로컬라이즈 정합.
### 2.4 데이터/밸런스
- 50랭크 데미지 곡선(×6) + 스킬 포인트 경제(50×직업스킬수 ≈ 레벨 요구) + 무한 성장 맥락 문서.
### 2.5 테스트
- 서버 Vitest(스케일/미러/parity) + 클라 Automation(MaxRank 50, GetEffectiveDamageCoeff rank 50 ×6·저랭크 회귀, GetEffectiveCooldown floor, RankUp 50까지).

## 3. 범위 외
- 랭크업 비용 골드화(현재 스킬 포인트만), 50 초과 랭크, 랭크별 스킬 효과 변화(데미지/쿨다운 스케일만)(후속).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | MaxRank 50 + 호출부 + Automation | ✅ 메인 (`character`) |
| 백엔드 | skillRank.ts 스케일 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 스킬 패널 rank/50 표기 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 50랭크 곡선 + 스킬포인트 경제 + 문서 | ✅ 보조 (`balance`) |
| QA | 50랭크/스케일/저랭크 회귀 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| MaxRank 5 하드코딩 잔존(HUD/테스트) | 전수 점검 + MaxRank 참조, 빌드/Automation |
| 저랭크 회귀 | 공식 불변(캡만 확장), rank 0~5 기존 동일 테스트 |
| 쿨다운 0/음수 | 기존 max(0.1, ...) floor 유지(rank20+ 0.1 하한) |
| 서버↔클라 스케일 parity | skillRank.ts Math.fround float parity + 경계 테스트 |

## 7. 후속
- 랭크업 골드 비용, 50 초과, 랭크별 효과 변화, 스킬 트리 분기.
