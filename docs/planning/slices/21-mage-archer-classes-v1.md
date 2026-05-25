# PR #21 기획서 — 마법사 + 궁수 직업 V1 (M5 PR #10)

> 전사(V1) 위에 **마법사·궁수** 추가. 직업별 스킬 트리 7종 + ClassId 기반 로딩 + 직업 스토리 분기. 스탯 성장곡선은 이미 `StatFormulas`(Warrior/Mage/Archer)에 존재 — 이번엔 **스킬셋 + 스킬 로딩 + 직업 선택 + 스토리**. GDD §3.1(마법사 INT·WIS 폭발 마법, 궁수 DEX·LUK 크리티컬).

## 1. 목표 / DoD
직업을 마법사/궁수로 선택하면 해당 직업 스탯 성장 + 직업 전용 스킬 7종(액티브4/패시브2/궁극기1)이 자동 전투에 적용되고, 직업별 스토리 인트로가 분기된다.

### DoD 검증
1. ClassId(Warrior/Mage/Archer)에 따라 AIdleCharacter 가 해당 스킬셋 로드 + 스탯 성장 적용.
2. 마법사 7스킬(INT 기반 마법 DPS) / 궁수 7스킬(DEX·크리 중거리) 자동 발동·쿨다운·게이지.
3. 직업 선택 경로(UI 또는 INI/시작 선택) 로 직업 전환 가능.
4. 직업별 스토리 인트로 분기(스토리 바이블 §3 직업 섹션 또는 텍스트 키).
5. 서버 SkillDB 미러에 마법사/궁수 스킬 반영. 서버 Vitest + UE 빌드/Automation GREEN.

## 2. 범위 (In Scope)
### 2.1 캐릭터·전투 (메인, C++)
- `USkillComponent`: `LoadDefaultMageSkills()` + `LoadDefaultArcherSkills()` (기존 Warrior 패턴). 직업별 7종(active4/passive2/ultimate1).
  - **마법사**: 파이어볼(단일 마법), 체인라이트닝(AoE), 마나실드(자버프), 메테오(원거리 강타), 패시브(주문력+/마나+), 궁극기(대마법).
  - **궁수**: 정조준(단일 크리), 멀티샷(AoE), 집중(자버프 크리율↑), 관통사격(돌진형), 패시브(크리율+/공속+), 궁극기(화살비).
- `AIdleCharacter`: `BeginPlay` 에서 `DefaultClassId` 에 따라 적합한 LoadDefault*Skills 호출(현재 Warrior 고정 → 분기).
- 마법사 데미지는 MagicAtk, 궁수는 크리(LUK/DEX) 반영 — CombatFormulas 활용(필요 시 크리 계산 V1 단순).
- Automation: 직업별 스킬 로드/자동발동, ClassId 분기.

### 2.2 직업 선택 (디자이너/캐릭터)
- 직업 선택 경로 V1: 시작 시 직업 선택 패널(전사/마법사/궁수) 또는 INI `DefaultClassId`. 선택 → ClassId set → 스킬/스탯 반영.

### 2.3 스토리 분기 (스토리)
- 스토리 바이블 §3(주인공) 또는 신규 §3.x 에 직업별 배경/인트로 1문단씩 + 텍스트 키(StoryText.csv). 챕터1 진입 시 직업별 한 줄 분기.

### 2.4 서버 (백엔드)
- `core/data/skills.ts` SkillDB 미러에 마법사/궁수 스킬 추가(classId 별). 클래스 메타(이미 rebirth/character 에 classId 있으면 활용).

### 2.5 밸런스
- 마법사/궁수 스킬 수치(쿨다운/계수) + 직업 밸런스(마법사 고DPS·종이방어 / 궁수 크리) 1차값 문서.

### 2.6 테스트
- 서버 Vitest(스킬 미러 parity) + 클라 Automation(직업별 스킬/분기).

## 3. 범위 외
- 도적/성직자(4·5직업, 후속), 전직(클래스 체인지) 시스템, 직업별 전용 장비, 직업 변경 비용.
- 직업별 고유 애니/이펙트 아트(후속, 방향만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | Mage/Archer 스킬셋 + ClassId 로딩 분기 + Automation | ✅ 메인 (`character`) |
| 밸런스 | 직업 스킬 수치/밸런스 + 문서 | ✅ 보조 (`balance`) |
| 스토리 | 직업별 스토리 분기/텍스트 | ✅ 보조 (`story`) |
| 백엔드 | SkillDB 미러 마법사/궁수 | ✅ 보조 (`backend`) |
| 디자이너 | 직업 선택 UI | ✅ 보조 (`designer`) |
| QA | 직업별 스킬/분기 시나리오 | ✅ 보조 (`qa`) |

## 5. 호출 순서
1. `character`(메인) → 스킬셋 + 로딩 분기 + 테스트
2. `balance` → 수치  3. `story` → 분기  4. `backend` → 미러  5. `designer` → 선택 UI  6. `qa`

## 6. 워크플로우 v3
[1] 기획+PR → [2] Codex 개발(+게시) → [3] Claude TM → [4] Codex TM+fix → [5] Claude 검증 → [N] **CI 그린 확정** + PM 종합 + 머지. 사용자 PIE 차후 일괄([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]).

## 7. 리스크
| 리스크 | 완화 |
| --- | --- |
| 직업별 데미지 계산(마법/크리) 분기 | CombatFormulas 확장 최소화, V1 단순 계수 |
| 서버↔클라 스킬 parity(3직업) | DefinitionParity 테스트 확장 |
| 직업 밸런스 미검증 | 1차값, 시뮬레이터 후속 |

## 8. 후속
- 도적/성직자, 전직, 직업 전용 장비/스토리 챕터 분기 확대, 직업별 아트.
