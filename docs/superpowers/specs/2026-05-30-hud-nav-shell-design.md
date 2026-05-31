# HUD 내비게이션 셸 — 적응형 정보구조 정비 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인 대기(브레인스토밍 → 사용자 검토 후 plan 단계화)
- 분류: 프론트엔드(UE5 HUD). **클라 전용 — 서버/세이브/parity 무관(SaveVer 29 유지).**
- 선행: 속성 UX #109 까지 누적된 ~28개 패널. 브랜치 `feat/hud-nav-shell`.
- 방향: "밸런스/UX 정비" → "HUD 정보구조/내비게이션" → "전면 내비게이션 셸"(사용자 클릭 확정).

## 1. 배경 / 문제

현 HUD 는 `IdleHUD.cpp` **단일 10,486줄** 모놀리스. `DrawHUD()`(IdleHUD.cpp:4512)가 매 프레임 **~28개 패널을 무조건 전부 그림.** 각 패널은 Canvas/서비스 null 일 때만 early-return 할 뿐, **거의 모든 패널이 항상 화면에 떠 있음.**

- **중앙 내비게이션 부재**: 패널 전환용 enum/탭/활성패널 개념 없음. 토글 가능한 것은 `bQuestLogVisible`·`bStatInfoVisible` 2개뿐(IdleHUD.h:1383~1384).
- **고정 앵커 충돌**: 예) 우상단(X=우측끝)에 Shop(Y=92)·Rune(Y=282)·QuestLog(Y=92), 좌상단(X=28)에 Guild(Y=120)·Pet(Y=300)·Mission(Y=360) — 서로 포개짐.
- 결과: "모든 기능이 항상 떠 있는" 디버그 대시보드형 → 신규/기존 유저 모두 길찾기 불가, 시각적 혼잡.

## 2. 목표 / 비목표

### 목표
1. **적응형 내비게이션 셸**: 화면 종횡비로 PC(레이아웃 B)/모바일(레이아웃 A) 자동 분기.
2. **2단 정보구조**: 7개 카테고리(1단) → 카테고리 내 패널 서브탭(2단).
3. **활성 패널 1개만 표시**: 나머지 메뉴 패널은 숨김. 기본 상태 = 패널 0개(순수 방치 전투 뷰).
4. **상시 HUD 요소 보존**: 전투/진행 관련 상시 표시물은 셸 밖에서 항상 노출.
5. **안전 리팩터**: 기존 `Draw*Panel` 내부 로직 보존, 위치만 도킹/오버레이 영역으로 인자화.
6. 회귀: 전체 IdleProject Automation GREEN 유지.

### 비목표
- 서버/세이브/전투 로직 변경(표시·배치만). **SaveVer 29 무변경.**
- 패널 **내용** 재설계(개별 패널의 기능/공식은 그대로).
- 내비 상태(마지막 본 카테고리/패널) 영속화 — 런치 시 닫힘으로 시작(세이브 무변경, 사용자 확정).
- UMG/Slate 위젯 전환 — 현 immediate-mode Canvas 유지(기존 Draw 함수 재사용).
- 신규 콘텐츠/패널 추가.

## 3. 적응형 레이아웃

### 3-1. 분기 규칙
- `AspectRatio = Canvas->SizeX / Canvas->SizeY`.
- `AspectRatio >= 1.3` → **PC 레이아웃 B**(가로/데스크톱), 그 외 → **모바일 레이아웃 A**(세로).
- 플랫폼 매크로(`PLATFORM_DESKTOP`/`PLATFORM_ANDROID`/`PLATFORM_IOS`)로 보정 가능(종횡비 우선, 매크로는 폴백/강제용).
- 매 프레임 재평가(창 리사이즈·회전 대응).

### 3-2. 레이아웃 B (PC, 가로)
- 좌측 세로 **카테고리 레일**: 7개 아이콘+라벨, 활성 카테고리 하이라이트.
- 우측 **고정폭 도킹 패널**: 상단 서브탭 행 + 그 아래 활성 패널 내용. 폭 = `Clamp(SizeX*0.34, 380, 560)` 가량.
- 좌측-중앙: **전투 씬 항상 노출**(레일과 도킹 사이).

### 3-3. 레이아웃 A (모바일, 세로)
- 하단 **카테고리 탭바**: 7개 탭(아이콘 우선, 라벨 작게). 엄지 도달 영역.
- 활성 패널 = **중앙 반투명 오버레이**(상단 서브탭 + 닫기 X). 전투 씬이 뒤로 비침.
- 바깥 영역 탭 또는 스와이프다운으로 닫기.

## 4. 정보구조 (7 카테고리 · 2단)

| 카테고리 | 패널(서브탭) |
| --- | --- |
| **전투** | 타워 · 던전 · 주간보스 |
| **성장** | 스탯분배 · 스탯정보 · 마스터리 |
| **환생** | 환생 · 초월 · 환생퍼크 |
| **장비** | 강화 · 잠재 · 룬 · 룬코덱 · 상점 |
| **수집** | 펫 · 칭호 · 업적 · 보물상자 |
| **일일** | 퀘스트 · 미션 · 출석 · 소비 · 시즌패스 |
| **소셜** | 길드 · 리더보드 |

- 서브탭이 5개(장비/일일)면 도킹/오버레이 상단에서 줄바꿈 허용.
- 카테고리/패널 라벨은 로컬라이즈(ko·en) 신규 키.

## 5. 상시 표시 (셸 밖, 항상 노출)

- 상단 재화 바(골드/보석/진행 챕터-스테이지) · 스테이지 인디케이터(`DrawStageIndicator`) · 보스 바(`DrawBossBar`) · 스킬 슬롯 바(`DrawSkillHud`) · 궁극기 게이지 · 속성 범례 · 플로팅 데미지(`DrawFloatingDamageTexts`) · 상태이상(`DrawStatusIndicators`) · 진행저장/클라우드 동기화 인디케이터.
- **모달(셸 위 덮음, 최우선)**: 오프라인 보상(`DrawOfflineRewardModal`) · 클래스 선택(최초 1회, `DrawClassSelectionPanel`) · 보스 특수공격 경고(`DrawBossSpecialWarning`).

## 6. 상호작용 규칙

- 기본 상태: `ActiveCategory = None`, `ActivePanel = None` → 패널 0개 = 순수 방치 전투 뷰.
- 카테고리 선택 → 해당 카테고리의 **첫 패널**(또는 직전 세션 내 마지막 본 패널, 메모리 한정·비영속) 표시.
- 같은 카테고리 재클릭 또는 닫기(X) → 닫힘(`ActiveCategory = None`).
- 서브탭 클릭 → 같은 카테고리 내 패널 전환.
- 항상 활성 패널 **1개**. 모달이 떠 있으면 셸 입력 차단.

## 7. 구현 접근 (안전 리팩터)

### 7-1. 신규 타입 (IdleHUD.h)
```cpp
enum class EHudCategory : uint8 { None, Combat, Growth, Rebirth, Gear, Collection, Daily, Social };
enum class EHudPanel : uint8 { None, Tower, Dungeon, WeeklyBoss, StatAlloc, StatInfo, Mastery,
    Rebirth, Transcend, RebirthPerk, Enhance, Potential, Rune, RuneCodex, Shop,
    Pet, Title, Achievement, TreasureBox, Quest, Mission, Attendance, Consumable, SeasonPass,
    Guild, Leaderboard };
```
- 카테고리→패널 순서 레지스트리(정적 테이블): `TArray<EHudPanel> PanelsForCategory(EHudCategory)`.
- AIdleHUD 멤버: `EHudCategory ActiveCategory = None;` `EHudPanel ActivePanel = None;`(+ 카테고리별 마지막 패널 캐시, 비영속).

### 7-2. DrawHUD 재구성
```
DrawHUD():
  상시 요소(전투/스테이지/보스/스킬/궁극기/속성범례/플로팅/상태/동기화)
  DrawNavShell()           // 종횡비 분기 → 레일(B) 또는 탭바(A) + 서브탭
  DrawActivePanel()        // ActivePanel 하나만 도킹/오버레이 영역에 Draw
  모달(오프라인/클래스선택/보스경고)
```
- 기존 `Draw*Panel()` 들은 **DrawActivePanel() 의 switch 에서 활성일 때만 호출.** 시그니처에 도킹 영역 `(float X, float Y, float W, float H)` 추가하거나, 멤버 `CurrentPanelRect` 를 참조하도록 변경 → 고정 앵커 상수 제거.

### 7-3. 히트박스
- 신규: 카테고리 레일/탭바(`NavCategory_<id>`), 서브탭(`NavPanel_<id>`), 닫기(`NavClose`).
- `NotifyHitBoxClick`(IdleHUD.cpp:4564)에 prefix 분기 추가 → `ActiveCategory`/`ActivePanel` 설정.

### 7-4. 단계(슬라이스 내 순서)
1. 셸 골격: enum/레지스트리/상태 + `DrawNavShell`(B/A) + 상시 요소 분리 + 빈 도킹 영역.
2. 패널 이주: 각 `Draw*Panel` 고정 앵커 → 도킹/오버레이 영역 인자화, switch 배선. (가장 큰 작업 — 패널 단위로 점진, 회귀 확인.)
3. 모바일 분기(A) 마감 + 로컬라이즈 키 + 회귀 테스트.

## 8. 세이브 / parity 영향

- **세이브 무변경(SaveVer 29 유지).** 내비 상태는 런타임 전용·비영속.
- 서버 무관(전투/공식/parity 변경 없음).
- 하위호환: 저장 데이터·전투 byte 동일 → 회귀 0.

## 9. 리스크 / 완화

- **10k줄 단일 파일 대수술** → 패널 단위 점진 이주 + 각 단계 Automation 회귀. ODR/jumbo 주의([[reference-ue-headless-verify]]): 신규 익명 헬퍼 동명 grep + PM 표준 jumbo 빌드.
- 고정 앵커 상수 산재 → 도킹 영역 인자화로 단일 좌표 출처화(중복 제거 부수 효과).
- 종횡비 분기 경계(정사각 근처) 깜빡임 → 임계값에 히스테리시스(예 1.25/1.35) 고려.
- PIE 시각 최종 확인(B/A 양 레이아웃) 권장 — 헤드리스로는 픽셀 미검증.

## 10. 테스트 / 검증

- 단위/Automation: 레지스트리(카테고리→패널 매핑), 분기 규칙(종횡비→레이아웃), 활성 패널 전환 상태머신, 닫기/토글.
- 회귀: 기존 패널 ViewModel 빌더 테스트 유지(내용 무변경).
- 수동(PIE): PC 가로/모바일 세로 각각 — 카테고리 7종 진입, 서브탭 전환, 상시 요소 노출, 모달 우선순위, 전투 씬 가시성.
- 게이트: `tools/ci/ue-automation.ps1` 표준 jumbo 빌드 + 전체 IdleProject Automation GREEN.
