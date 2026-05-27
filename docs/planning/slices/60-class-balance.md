# PR #60 기획서 — 직업 밸런스 정밀 튜닝 (시스템 심화/밸런스)

<!-- markdownlint-disable MD013 MD022 MD026 MD032 MD058 -->

> **사용자 방향(시스템 심화/밸런스).** #57에서 8직업 추가 시 [4] 리뷰가 "정밀 튜닝은 후속"으로 명시 연기. 데이터 확인 결과 **파워 크리프**: 신규 직업이 원조보다 동일 역할에서 더 잘 스케일링 — Berserker STR 성장 1.8 vs Warrior 1.4(+28%), Summoner INT 1.7 vs Mage 1.4(+21%), 게다가 Berserker는 방어 스탯도 준수해 전사 상위호환. "왜 전사/마법사를 고르나" 문제. **8직업 상대 파워를 측정·검증 도구로 정량화하고, 아웃라이어를 타깃 밴드로 튜닝**(역할 차별화는 유지하되 직군 내 상위호환 제거). client + balance(검증 도구) + backend(parity) 멀티시스템(+qa).

## 1. 목표 / DoD
8직업이 동일 투자(레벨/장비/스탯 분배) 대비 **역할에 맞는 균형 잡힌 전투 성능**을 갖는다 — DPS 직군은 서로 ±15% 밴드 내, 탱커(Paladin)/힐러(Cleric)는 생존/유틸로 DPS 일부를 의도적 트레이드(직군 내 상위호환·열위호환 제거).

### DoD 검증
1. **클래스 파워 비교 도구**: balance-sim(또는 신규 도구)에 직업별 유효 전투 지표 산출 — 동일 레벨(예 Lv50/Lv100)·동일 스탯분배·동일 장비 가정에서 각 직업의 유효 공격력(물리/마법 적합)·실효 DPS(스킬 계수·쿨다운·크리 반영)·CP(#49). 8직업 비교표 + 밴드 이탈 직업 식별. 리포트 산출.
2. **파워 크리프 교정(튜닝)**: 도구 결과로 아웃라이어 조정 —
   - **직군 내 상위호환 제거**: Berserker(물리) vs Warrior, Summoner(마법) vs Mage 등 신규-원조 쌍이 한쪽 strict-better 아니게. 차별화는 트레이드오프로(예 Berserker=고DPS·저생존 글래스캐넌, Warrior=균형). 단순 "더 높은 성장"이 아니라 프로필 재배분.
   - **탱/힐 역할 정합**: Paladin은 생존(CON/방어) 대가로 DPS 하향 의도 명확화(클리어 가능 수준 유지), Cleric 힐 유틸 정합.
   - FClassGrowth(기본/성장) 및/또는 스킬 계수(DamageCoeff/Cooldown) 조정. **클라 StatFormulas.cpp + 서버 stats.ts + 클라 SkillComponent + 서버 skills.ts 동기**.
3. **목표 밴드 정의**: DPS 6직군(Warrior/Mage/Archer/Thief/Berserker/Summoner) 유효 전투 출력 중앙값 대비 ±15% 내. Paladin은 DPS -25~40% 허용(생존 보상), Cleric은 자가회복 유틸 반영. 도구로 검증.
4. **클라/서버 parity**: FClassGrowth(stats.ts)·스킬 계수(skills.ts) 변경 전부 클라↔서버 동일값 + parity 테스트(Math.fround). DeriveStats/ClassDamage Automation 정합.
5. **회귀안전**: 튜닝은 상대 밸런스 교정 — 절대 파워 급변 지양(진행 페이싱 balance-sim median 5~10h 유지 확인). 기존 직업 식별/스킬 구조 무변경(값만).
6. **테스트**: 클라 Automation(8직업 DeriveStats/ClassDamage/유효출력 밴드) + 서버 vitest(stats/skills parity·밸런스 단언) + balance-sim 클래스 비교 리포트. UE 빌드/Automation + 서버 build/test/lint GREEN.

## 2. 범위 (In Scope)
### 2.1 클래스 비교 검증 도구 (balance)
- tools/balance-sim 에 직업별 유효 전투 지표 산출 + 비교표/밴드 이탈 리포트 + 단조/밴드 vitest.
### 2.2 밸런스 튜닝 (character + backend 미러)
- FClassGrowth 8직업 재조정(파워 크리프 교정, 역할 트레이드오프) — 클라 StatFormulas.cpp + 서버 stats.ts. 필요 시 스킬 계수(클라 SkillComponent + 서버 skills.ts) 조정. parity 유지.
### 2.3 테스트 (character/backend/balance) — 위 DoD 6.
### 2.4 문서 (디자이너/PM)
- docs/planning/05-balance-philosophy.md 에 직업 밸런스 원칙(직군 밴드·탱/힐 트레이드오프) 추가. 직업별 컨셉 1줄.

## 3. 범위 외 (후속)
- 신규 직업 추가, 전직/승급, 직업 전용 장비/세트, 스킬 메커니즘 신규(콤보 등), 경제 리밸런스(현재 건강 — 진행 5.3h/엔드 싱크 충분).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 밸런스 (메인) | 클래스 비교 도구(balance-sim) + 밴드 분석 + 튜닝 제안값 산출 + vitest | ✅ 메인 (`balance`) |
| 캐릭터·전투 | FClassGrowth/스킬 계수 튜닝(클라) + DeriveStats/ClassDamage Automation 정합 | ✅ 보조 (`character`) |
| 백엔드 | 서버 stats.ts/skills.ts 튜닝 미러 + parity vitest | ✅ 보조 (`backend`) |
| QA | 8직업 동일 투자 전투 출력·역할 정합·클리어 가능 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex balance 메인(+character/backend/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증(UE+서버) → [N] **CI 그린 확정**(server-ci 포함) + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). [[project-infinite-growth]] 무한 성장 정합(상대 밸런스만 교정, 절대 캡 도입 금지).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 밸런스 주관성 | 정량 비교 도구(유효 출력/CP) + 명시 목표 밴드(±15%/탱힐 트레이드) — 측정 기반 튜닝 |
| 절대 파워 급변(진행 페이싱 깨짐) | 상대 교정 위주, balance-sim median 5~10h 유지 검증, 무한 성장 곡선 무변경 |
| 클라/서버 parity 깨짐 | stats.ts/skills.ts 동기 + parity vitest(Math.fround) + DeriveStats/ClassDamage Automation |
| 과교정(원조 너프로 약화) | 직군 중앙값 기준 조정(원조 상향 또는 신규 하향 균형), 절대 약화 지양 |
| 기존 직업 구조 회귀 | EClassId/스킬 ID/구조 무변경, 수치만 |

## 7. 후속
- 전직/승급, 직업 전용 장비, 스킬 메커니즘 심화(콤보/연계), 추가 직업. (밸런스 원칙 문서가 향후 직업 추가의 가이드.)

## 8. Codex TM Fix Notes

- `tools/balance-sim` class DPS now routes both base hits and active skill
  pressure through `computeClassDamage` with Lv-scaled review defense, instead
  of multiplying raw effective attack.
- `client/Content/Data/SkillDB.csv` now matches the tuned Berserker and
  Summoner coefficients already present in `USkillComponent` and `skills.ts`.
- `server/tests/class-balance.test.ts` guards Lv50/Lv100 +/-15% DPS rows,
  Paladin/Cleric role exceptions, and tuned SkillDB coefficient parity.
- Latest simulator output: 1000 runs, median 5.328h, p10 4.919h, p90 5.751h,
  min/max 4.564h / 6.144h.
- Latest Lv100 DPS deltas: Warrior -7%, Mage +3%, Archer -4%, Thief +1%,
  Berserker 0%, Summoner -8%; Paladin and Cleric remain below 0.8x median
  while preserving tank/healer stat compensation.
