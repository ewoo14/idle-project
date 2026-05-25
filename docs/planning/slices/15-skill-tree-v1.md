# PR #15 기획서 — 스킬 트리 V1 (원 슬라이스 ID: S5, M2)

> 마일스톤 M2(장비 & 스킬, Loop V1)의 스킬 슬라이스. 전사(Warrior) 기준 **액티브 4 + 패시브 2 + 궁극기 1**, 자동 발동, 쿨다운 표시. GDD §3.4, 아키텍처 §2(전투=C++), 밸런스 §3 기반.

---

## 1. 목표 / DoD

자동 전투 중 캐릭터가 **스킬을 쿨다운 기반으로 자동 발동**하고, 패시브가 능력치에 반영되며, 궁극기 게이지가 누적되어 발동되고, HUD 에 **쿨다운/게이지가 표시**된다.

### DoD 검증 시나리오
1. PIE 진입 → 전사 자동 전투 시작 → 액티브 스킬 4종이 각자 쿨다운마다 자동 발동(로그/이펙트), 일반 공격과 병행.
2. 패시브 2종이 시작 시 능력치(ATK/최대HP)에 반영됨(HUD 능력치 확인).
3. 전투(공격/피격)로 궁극기 게이지 누적 → 100% 시 궁극기 자동 발동 → 게이지 리셋.
4. HUD 에 액티브 4종 쿨다운(남은 시간/비율) + 궁극기 게이지 바 표시.
5. `Build.bat` 성공 + 자동화 테스트(스킬 쿨다운/게이지/패시브 순수 로직) 통과.

---

## 2. 범위 (In Scope)

### 2.1 CombatSystem — 스킬 코어 (C++, 메인)
- `ESkillType { Active, Passive, Ultimate }`, `ESkillEffectType { DamageSingle, DamageAoe, SelfBuff, DashDamage }`
- `FSkillDefinition` 구조체: `SkillId(FName)`, `DisplayName`, `Type`, `EffectType`, `Cooldown(초)`, `DamageCoeff(ATK 배수)`, `BuffMagnitude`, `BuffDuration`, `GaugeCostOrGain` 등.
- `USkillComponent : UActorComponent`
  - 보유 스킬 목록(`TArray<FSkillDefinition>`) + 런타임 상태(`LastCastTime` per skill, 궁극기 `CurrentGauge`).
  - `TickSkills(float Now)` — 5Hz 전투 틱에서 호출. 각 액티브에 대해 `IsReady(Now)` 면 자동 발동.
  - `bool IsReady(SkillId, Now)` / `float GetCooldownRemaining / GetCooldownRatio(SkillId, Now)` — UI 조회용.
  - 궁극기: `AddGauge(float)` (공격/피격 시 누적, 0~100), `bIsUltimateReady()`, 발동 시 리셋.
  - 패시브: `ApplyPassivesToStats(FDerivedStats& InOut)` — `RefreshDerivedStats` 에서 합산.
- 스킬 실행: 데미지는 `FCombatFormulas::ComputeDamage` 재사용. 대상 탐색은 `UBattleAIComponent::FindClosestEnemy` 패턴 재사용(AoE 는 범위 내 다수).

### 2.2 BattleAIComponent 연동
- 전투 틱(`UpdateBattle`)에서 `USkillComponent::TickSkills` 호출 → 사거리 내 타깃 있을 때 액티브 자동 발동. 기존 일반 공격 로직과 공존(스킬은 별도 쿨다운).
- 공격/피격 이벤트에서 궁극기 게이지 누적 훅.

### 2.3 CharacterSystem — 스킬 보유/패시브
- `AIdleCharacter` 에 `USkillComponent` 추가 + 전사 기본 7종 정의 주입(데이터 소스: §2.5).
- `RefreshDerivedStats` 에서 패시브 반영(`ApplyPassivesToStats`).

### 2.4 전사 스킬 V1 정의 (밸런스 1차 — 시뮬레이터 도착 시 재보정)
액티브(자동, 쿨다운):
| ID | 이름 | 효과 | 쿨다운 | 계수/수치 |
| --- | --- | --- | --- | --- |
| `heavy_strike` | 강타 | 단일 강타 | 4s | ATK ×2.5 |
| `whirlwind` | 회전베기 | 주변 적 광역 | 8s | ATK ×1.8 (범위 내 전체) |
| `shield_up` | 방패 올리기 | 자버프 DEF↑ | 12s | DEF +50%, 4s |
| `charge` | 돌진 | 최원거리 적 돌진+타격 | 10s | ATK ×2.0 |

패시브(영구):
| ID | 이름 | 효과 |
| --- | --- | --- |
| `weapon_mastery` | 무기 숙련 | ATK +15% |
| `toughness` | 강인함 | 최대 HP +20% |

궁극기(게이지식):
| ID | 이름 | 효과 | 게이지 |
| --- | --- | --- | --- |
| `berserkers_fury` | 광전사의 분노 | 대형 단일 타격(ATK ×6) + 4s 공격력 +30% | 공격당 +8, 피격당 +5, 100 도달 시 발동 후 0 리셋 |

### 2.5 데이터 소스
- V1 은 `StatFormulas` 처럼 **C++ 정적 정의**(`USkillComponent` 가 직업별 기본 스킬셋을 코드 테이블로 보유). `SkillDB` DataTable/서버 미러는 후속.

### 2.6 UI — 쿨다운/게이지 표시
- `IdleHUD` 확장: 하단에 액티브 4종 쿨다운(아이콘/바 + 남은 비율) + 궁극기 게이지 바. V1 은 텍스트/단순 바 허용(아이콘 에셋 후속).

### 2.7 서버 미러 (최소)
- `SkillDB` 시드(전사 7종 id/type/effectType/cooldown/계수/버프/게이지)를 `server/src/core/data/skills.ts` 에 최소 추가(읽기 전용 참조). 실제 스킬 실행은 클라 C++ 결정적 로직.

### 2.8 테스트
- 순수 로직 자동화 테스트: 쿨다운 `IsReady/GetCooldownRatio`, 궁극기 게이지 누적/임계/리셋, 패시브 `ApplyPassivesToStats` 합산.

---

## 3. 범위 외 (Out of Scope)
- 마법사/궁수 등 타 직업 스킬(M4 PR #10).
- 환생 전용 스킬 슬롯 해금(환생 시스템 PR).
- 스킬 레벨업/포인트 분배 UI(후속) — V1 은 7종 고정 해금.
- 스킬 아이콘/이펙트 아트, 궁극기 연출(후속, 아트 디렉션만).
- 보스 전용 궁극기 게이팅 — V1 은 게이지 100% 시 항상 발동 허용.

---

## 4. 7파트 작업 분배 — Codex 호출 계획
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| **캐릭터·전투 (메인)** | `USkillComponent` + `FSkillDefinition` + 자동발동/쿨다운/게이지/패시브 + BattleAI 연동 + AIdleCharacter 주입 + 테스트 | ✅ 메인 (`character`) |
| 밸런스 | 전사 7종 수치(쿨다운/계수/게이지율) 데이터 + 밸런스 문서 갱신 | ✅ 보조 (`balance`) |
| 디자이너 | HUD 쿨다운/게이지 표시 + 아트 디렉션(스킬 아이콘/이펙트 방향) | ✅ 보조 (`designer`) |
| 백엔드 | `SkillDB` 최소 시드(읽기 참조) | ✅ 최소 (`backend`) |
| QA | 스킬 자동발동/게이지/패시브 검증 시나리오 + IT | ✅ 보조 (`qa`) |
| 스토리/퀘스트 | 해당 없음(스킬 플레이버 텍스트만 story 보조 선택) | ◻ |

## 5. 호출 순서
1. `character`(메인) → 코어 스킬 시스템 + 연동 + 테스트
2. `balance` → 수치 데이터/문서 (메인 구조 위에)
3. `designer` → HUD 표시
4. `backend` → SkillDB 시드 (독립)
5. `qa` → 검증 시나리오

## 6. 워크플로우 v3
[1] PM 기획(본 문서)+PR → [2] Codex 개발(+PM 산출 게시) → [3] Claude 리뷰+fix → [4] Codex 리뷰+fix(+PM 산출 게시) → [5] Claude 검증 → [N] CI 통과+PM 종합+머지. 기능 슬라이스이므로 Codex 포함 풀 워크플로우(사용자 명시).

## 7. 일정 (잠정)
- Codex 메인 ~40분 + 보조 합동 ~30분 → 리뷰 라운드 1~2.

## 8. 리스크
| 리스크 | 완화 |
| --- | --- |
| AoE/돌진 대상 탐색이 기존 단일 타깃 패턴과 다름 | `FindClosestEnemy` 를 범위 질의로 일반화, 단순 거리 기반 V1 |
| 궁극기 "보스 전용" 게이팅 미구현 | V1 은 게이지 100% 발동 허용으로 단순화, 게이팅 후속 명시 |
| HUD 아이콘 에셋 부재 | V1 텍스트/단순 바, 아이콘 후속 |
| 스킬 수치 미검증 | 1차 가이드, M2 밸런스 시뮬레이터 도착 시 재보정(밸런스 문서 명시) |

## 9. 후속 PR 예고
- 타 직업 스킬 트리(마법사/궁수), 스킬 레벨업/포인트 분배 UI, 스킬 아이콘/이펙트 아트, 궁극기 보스 게이팅, SkillDB 서버 검증.
