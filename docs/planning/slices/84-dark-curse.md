# PR #84 기획서 — Dark 속성 심화: 저주(Curse) 상태이상

> **PM 자율 진행 (Claude 에이전트 구현팀, Codex 6/1 복구까지)**. #66 Dark 속성 + #30 상태이상 프레임워크 활용 — **저주(Curse) = 받는 피해 증폭 디버프**(DoT/슬로우와 다른 새 범주)를 추가하고 Dark 스킬이 부여. 콘텐츠 풍부화([[project-content-richness]]). client + server 5-team.

## 1. 목표 / DoD
Dark 속성에 고유 전투 정체성 부여. 저주 = 대상이 받는 피해 증폭(처치 가속).

### DoD
1. **저주 상태**: `ESkillStatusEffect::Curse = 4`(StatusElementTypes.h). Poison/Burn(DoT)·Freeze(슬로우)와 다른 **피해 증폭** 범주. `ApplyStatus(Curse, Duration, Magnitude, Now)`.
2. **피해 증폭 적용**: `CombatComponent::TakeDamageTyped`에서 저주 활성 시 받는 피해 `×(1 + GetActiveStatusMagnitude(Curse))`(단일 지점, 다른 상태와 독립). 저주는 DoT 틱 없음(지속만, TickStatuses에서 만료 처리).
3. **부여**: Dark 속성(ESkillElement::Dark) 스킬이 적중 시 저주 부여(Fire→Burn 패턴 동일). Dark 스킬이 없으면 thematically 적절한 스킬 2~3종을 Dark+Curse로 지정(테스트 가능하게).
4. **서버 미러**: combat.ts 저주 피해 증폭 + skills.ts Dark→Curse 부여 정의. 클라↔서버 parity.
5. **UI**: 저주 상태 아이콘(Dark 색) HUD 표시. ko/en CsvIntegrity.
6. **세이브 변경 없음**(전투 런타임 상태). 기존 Poison/Burn/Freeze·전투 회귀 없음.
7. **테스트**: 클라 Automation(저주 피해증폭/만료/부여/다른 상태 독립/회귀) + 서버 vitest(combat 증폭/skills 부여) + parity. CI GREEN + **표준 jumbo 빌드 + 광범위 Automation PM 검증**(tools/ci/ue-automation.ps1).

## 2. 작업 분배 (Claude 서브에이전트, claude(<role>):)
| 파트 | 작업 |
| --- | --- |
| character (메인) | ESkillStatusEffect::Curse, TakeDamageTyped 피해 증폭, TickStatuses 만료, Dark 스킬 Curse 부여(스킬 데이터), 서버 combat.ts/skills.ts 미러, 클라 Automation |
| backend | combat.ts 저주 증폭 + skills.ts Dark→Curse + vitest parity (character 흡수 가능) |
| designer | 저주 상태 아이콘(Dark 색) HUD + ko/en CsvIntegrity |
| balance | 저주 Magnitude(증폭%)·지속·Dark 스킬 밸런스 + 인플레 점검 |
| qa | 저주 증폭/만료/부여/다른 상태 독립/회귀 + parity |

## 3. 범위 외
저주 스택, 몬스터→플레이어 저주(역방향), Dark 전용 스킬 신규 트리, 저주 외 Dark 디버프.

## 4. 워크플로우 v3
Claude 서브에이전트 구현 → PM 리뷰/통합. [5] **`tools/ci/ue-automation.ps1`(전체 IdleProject)로 PM 검증**(#83 게이트). 머지 전 CI 그린.

## 5. 리스크
| 리스크 | 완화 |
| --- | --- |
| 피해 증폭 이중 적용 | TakeDamageTyped 단일 지점, 다른 상태와 독립 |
| Dark 스킬 부재로 미발동 | 적절 스킬 2~3종 Dark+Curse 지정(테스트 가능) |
| 증폭 인플레 | Magnitude 캡/지속 제한 + balance |
| 서버↔클라 drift | combat.ts/skills.ts 미러 + parity |
| jumbo ODR | 신규 익명 헬퍼 동명 grep + PM 표준 jumbo 빌드 |
