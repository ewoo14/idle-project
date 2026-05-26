# PR #28 기획서 — 데미지 플로팅 텍스트 + 전투 피드백 (심화)

> 기존 슬라이스 심화. PR #26(마법/크리)·#27(랭크)로 데미지가 차별화됐으나 화면 피드백이 없음. 피격 시 **데미지 숫자를 대상 위에 띄워**(크리/마법/물리 시각 구분 + 위로 떠오르며 페이드) 전투 체감을 높임. 클라 UI 전용.

## 1. 목표 / DoD
적/플레이어가 피격될 때 데미지 수치가 대상 머리 위에 떠올라 애니메이션(상승+페이드)되며, 크리티컬/마법/물리가 시각적으로 구분된다.

### DoD 검증
1. 데미지 적용 → 대상 화면 위치에 데미지 숫자 플로팅(상승+페이드, ~1s).
2. 크리티컬 = 강조(큰 글자/노랑), 마법 = 파랑/보라, 물리 = 흰색.
3. 데미지 이벤트가 크리 여부 + 타입(물리/마법)을 정확히 전달(데미지 적용부에서 메타 캡처).
4. UE 빌드/Automation(플로팅 모델/이벤트 메타) GREEN. 기존 전투 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 데미지 이벤트 메타 (캐릭터, C++)
- `EDamageKind { Physical, Magic }`.
- 데미지 적용부(SkillComponent::ApplyDamageSkill, BattleAIComponent::Attack)에서 **RollCrit 결과 bool 캡처** + magic/phys 판정 → 메타 전달.
- `UCombatComponent`: `OnDamageReceived(float Amount, bool bWasCrit, EDamageKind Kind)` 델리게이트(또는 TakeDamage 확장 + 기존 호출 호환). 피격 시 broadcast.
### 2.2 플로팅 텍스트 (디자이너, C++ HUD)
- IdleHUD: OnDamageReceived 구독 → 활성 플로팅 항목 리스트(대상 액터/월드위치/금액/크리/타입/생성시각). DrawHUD 에서 월드→스크린 투영 위치에 숫자 그리기 + 시간에 따라 상승(+Y) + 알파 페이드(~1s 후 제거).
- 스타일: 크리=큰 글자+노랑(+"!"/×배수 표기 선택), 마법=파랑, 물리=흰. design-system 토큰.
### 2.3 테스트
- 이벤트 메타(크리/타입) 정확성 + 플로팅 뷰모델(수명/위치/스타일 결정) 순수 로직 Automation.

## 3. 범위 외
- 회복/버프 숫자, 데미지 누적 표시(DPS 미터), 데미지 타입 추가(속성)(후속).
- 파티클/사운드 이펙트(사운드는 외부 의존).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | EDamageKind + 데미지 메타 캡처 + OnDamageReceived broadcast + Automation | ✅ 메인 (`character`) |
| 디자이너 | 플로팅 텍스트 렌더/애니/스타일 + 뷰모델 테스트 | ✅ 보조 (`designer`) |
| QA | 크리/마법/물리 플로팅 표시 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+designer/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| TakeDamage 시그니처 변경 호환 | 기존 시그니처 유지 + 오버로드/델리게이트로 메타 추가, 기존 호출/테스트 보존 |
| 월드→스크린 투영(플레이어 컨트롤러) | PlayerController ProjectWorldToScreen, 실패 시 스킵 |
| DrawHUD 플로팅 누적/수명 누수 | 만료 항목 제거 + 상한, 뷰모델 테스트 |

## 7. 후속
- 회복/버프 숫자, DPS 미터, 속성/상태이상 표시, 파티클·사운드(외부).
