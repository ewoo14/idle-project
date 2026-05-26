# PR #27 기획서 — 스킬 레벨업 / 포인트 분배 (심화)

> 기존 슬라이스 심화. PR #15 스킬 트리에서 미뤄둔 **스킬 성장**. 레벨업 시 스킬 포인트 획득 → 스킬에 투자해 랭크업(쿨다운↓ / 데미지↑). 스킬 트리에 진행 깊이를 더함. 능력치 포인트(GDD §119)와 별개의 스킬 포인트.

## 1. 목표 / DoD
레벨업 시 스킬 포인트를 얻고, 스킬에 투자해 랭크를 올리면 해당 스킬의 효과(쿨다운/데미지)가 강해진다. 자동 전투가 랭크 반영값으로 동작.

### DoD 검증
1. 레벨업(OnLevelUp) → 스킬 포인트 +1(또는 규칙). 보유 포인트 표시.
2. 스킬 랭크업(포인트 소비) → 랭크 0~maxRank(예 5). 랭크당 효과: 데미지 계수 +10% / 쿨다운 -5%(상한).
3. 자동 발동/데미지가 **랭크 반영 유효값**(GetEffectiveDamageCoeff/GetEffectiveCooldown) 사용.
4. 스킬 랭크업 UI(스킬 HUD 확장): 보유 포인트 + 스킬별 랭크/＋버튼.
5. 서버 Vitest(랭크 효과 공식) + UE 빌드/Automation GREEN. 기존 전투/스킬 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 스킬 코어 (메인, C++)
- USkillComponent: 스킬별 랭크 상태 `TMap<FName,int32> SkillRanks` + maxRank(예 5) + 스킬 포인트 `SkillPoints`.
- `GrantSkillPoint(n)` (레벨업 훅), `CanRankUp(SkillId)`, `RankUpSkill(SkillId)`(포인트 소비, maxRank 상한), `GetSkillRank(SkillId)`.
- 유효값: `GetEffectiveDamageCoeff(SkillId)` = base × (1 + rank×0.1), `GetEffectiveCooldown(SkillId)` = base × (1 - rank×0.05, 하한). TickSkills/ApplyDamageSkill/GetCooldownRatio 가 유효값 사용.
- 랭크 효과 순수 공식 헬퍼(서버 미러 가능 형태).
### 2.2 레벨업 연동
- UIdleGameInstance OnLevelUp → SkillComponent GrantSkillPoint(1) (또는 규칙). 직업 전환/환생 시 정책(V1: 환생 시 스킬 포인트 리셋 또는 보존 — 단순히 유지).
### 2.3 UI (디자이너)
- 스킬 HUD 확장: 보유 스킬 포인트 + 스킬별 현재 랭크/maxRank + 랭크업 버튼(포인트 있을 때).
### 2.4 데이터/밸런스
- 랭크당 +10% 데미지 / -5% 쿨다운, maxRank 5, 레벨당 포인트 1 (1차값) 문서.
### 2.5 테스트
- 서버 Vitest(랭크 효과 공식) + 클라 Automation(포인트 획득/랭크업/유효값/상한).

## 3. 범위 외
- 스킬 트리 분기(선행 스킬), 스킬 리스펙(초기화) 비용, 환생 전용 스킬 슬롯(별도 후속).
- 스킬별 고유 강화 효과(현재는 일률 +데미지/-쿨다운).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | 랭크/포인트 상태 + 유효값 + 레벨업 훅 + Automation | ✅ 메인 (`character`) |
| 디자이너 | 스킬 랭크업 UI | ✅ 보조 (`designer`) |
| 밸런스 | 랭크 효과/포인트 수치 + 문서 | ✅ 보조 (`balance`) |
| 백엔드 | (선택) 스킬 랭크/포인트 persist | ◻ 최소/후속 |
| QA | 포인트→랭크업→데미지 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 가 커밋/푸시 누락하므로 PM 이 확인·보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 랭크 유효값 누락 경로(일부 데미지가 base 사용) | 모든 데미지/쿨다운 조회를 GetEffective* 경유로 통일 + 테스트 |
| 환생/직업전환 시 포인트 정책 | V1 단순(유지), 정책 문서 명시 |
| 밸런스 폭주(랭크 누적) | maxRank 상한 + 1차값, 시뮬 후속 |

## 7. 후속
- 스킬 리스펙, 스킬 트리 선행 분기, 스킬별 고유 강화, 환생 전용 슬롯, 서버 persist/검증.
