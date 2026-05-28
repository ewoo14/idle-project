<!-- markdownlint-disable MD013 MD022 MD028 MD032 MD060 -->

# 아트 디렉션 — idle-project

> 본 문서는 비주얼 톤·아이덴티티의 1차 기준선입니다. 디테일한 캐릭터 시트·UI 키트는 디자이너 슬라이스 PR (PR #2 이후) 에서 추가됩니다.

---

## 1. 비주얼 컨셉

### 1.1 한 줄 (2026-05-25 갱신, PR #11)
> *"3D 스타일라이즈드 애니메 풍 사이드뷰 — 셀쉐이딩 + 표정 풍부한 미소년/미소녀 + 게임감 있는 카메라"*

> **변경 이력**: PR #1 의 도트 풍 원안 → 사용자 명시 (2026-05-25) 로 3D 애니메 풍 전환 (PR #11). 도트 사이즈 / 8-12 프레임 / 트위닝 금지 등 도트 표준은 **모두 폐기**. 셀쉐이딩 3D + Mixamo 풍부한 애니메이션 + ARKit blend shape 표정 채택.

### 1.2 키워드
- **친근함** — 일본 애니메 풍 둥근 얼굴 + 큰 눈
- **표정** — Idle / Battle / Smile / Hit / Death / LevelUp 등 상황별 자연 표정
- **환상적** — Niagara 파티클, 동적 라이팅으로 매 사냥이 화려
- **셀쉐이딩** — 외곽선 강조 + 평면 그림자 (Toon shading)

### 1.3 참고
- VRoid Studio 의 표준 미소년/미소녀, 원신 (Genshin Impact), 블루 아카이브, AFK Arena (UI 정돈), Octopath Traveler (사이드뷰 카메라)

---

## 2. 캐릭터 / 몬스터

### 2.1 비율 (3D 갱신)
- **8헤드 (1:7~8)** — VRoid 표준
- 사이즈: SK Mesh 약 **180 cm (UE unit 180)** — 표준 키
- (구) 픽셀 64×96 px 폐기

### 2.2 컬러
- 메인 캐릭터는 **3색 팔레트** 로 인식성 확보 (헤어 / 의상 / 액세서리)
- 셀쉐이딩 외곽선 — 어두운 색 (#1A1B2E 또는 #000)
- 몬스터는 직업 컬러와 보색 우선

### 2.3 애니메이션 (3D 갱신)
- **Skeletal Mesh + AnimBlueprint State Machine**
- 자산 출처: **Mixamo 무료** (Idle / Walking / Sword Slash / Hit React / Death) + 추후 자체 제작
- 트랜지션: blend 시간 0.15~0.3s (자연 보간)
- (구) 8-12 프레임 도트 표준 폐기

### 2.4 표정 시스템 (3D 신규)
- **Blend Shapes (Morph Targets)** — VRoid 의 ARKit 호환 표정 (`Joy`, `Angry`, `Sorrow`, `Fun`, `Surprised`, `Neutral`)
- `EFacialExpression` enum: None / Idle / Battle / Smile / Hit / Death / LevelUp
- `UFacialExpressionComponent::SetExpression(EFacialExpression, Duration)` API
- 자동 트리거:
  - 전투 시작 → Battle (Angry)
  - 슬라임 사망 → Smile (Joy, 1초)
  - 피격 → Hit (Sorrow, 0.5초)
  - 사망 → Death (Sorrow + 영구)
  - 레벨업 → LevelUp (Surprised + Joy, 1.5초)
  - 평상시 → Idle (Neutral)

---

## 3. UI / UX 스타일

### 3.1 톤
- 라운드 4~8px 코너, 옅은 그림자
- 음영 대신 라인 강조 (1.5px 다크 라인)
- 아이콘은 외곽선 + 단색 채움

### 3.2 컬러 토큰 (초안)

| 토큰 | HEX | 용도 |
| --- | --- | --- |
| `--bg-primary` | `#1A1B2E` | 메인 배경 |
| `--bg-panel` | `#252845` | 패널 |
| `--text-primary` | `#F5F5F0` | 일반 텍스트 |
| `--text-muted` | `#A0A4B8` | 보조 텍스트 |
| `--accent-gold` | `#F4C44A` | 강조, 골드 |
| `--accent-blue` | `#6BB7F0` | MP, 정보 |
| `--accent-red` | `#E5556B` | HP, 위험 |
| `--rarity-common` | `#B0B5C0` | 일반 / Common |
| `--rarity-rare` | `#5B8BC0` | 희귀 / Rare |
| `--rarity-epic` | `#A05BC0` | 영웅 / Epic |
| `--rarity-unique` | `#5BC07A` | 유니크 / Unique |
| `--rarity-legendary` | `#F0A040` | 전설 / Legendary |
| `--rarity-transcendent` | `#40D0E8` | 초월 / Transcendent |
| `--rarity-mythic` | gradient(`#FF7B00 → #00B4FF`) | 신화 / Mythic |

레어도 표시는 `Common → Rare → Epic → Unique → Legendary → Transcendent → Mythic` 7단계를 기준으로 한다. `Uncommon` 표시는 더 이상 사용하지 않으며, 기존 초록 계열은 `Unique` 토큰으로 재배정한다. `Transcendent`는 prestige 시스템의 `초월` 패널과 같은 한국어를 쓰지만, 장비/룬 행과 상점 결과처럼 아이템 맥락 안에서만 노출해 환생·초월 진행 패널과 혼동되지 않게 한다.

Skill HUD V1은 `--bg-panel` 위에 `--accent-blue`로 쿨다운 진행도를, `--accent-gold`로 READY 상태와 궁극기 준비 상태를 표시한다. 하단 중앙 4슬롯 + 상단 궁극기 게이지 구조로 배치하고 1080p/1440p/4K에서 캔버스 높이 기준으로 크기를 스케일해 기존 HP/EXP/Gold/장비 HUD와 겹치지 않게 유지한다.

Skill Rank HUD V1은 기존 Skill HUD 슬롯 내부에 `랭크 n/MaxRank`를 보조 텍스트로 추가하고, 슬롯 우상단 24px 정사각 `+` 버튼으로 강화 가능 상태를 표시한다. 남은 스킬 포인트는 궁극기 게이지 위 우측에 `스킬 포인트 N`으로 노출하며, 포인트가 1 이상이면 `--accent-gold`, 0이면 `--text-muted`를 사용한다. `CanRankUp(SkillId)`가 true인 슬롯만 `+` 버튼을 `--accent-gold` CTA로 그리고 클릭 대상에 포함한다. V1은 별도 스킬 트리 화면 없이 HUD 내 즉시 강화만 제공하며, 1080p/1440p/4K에서 기존 4슬롯 중앙 정렬을 유지하도록 슬롯 폭 160px, 높이 60px 기준으로 Canvas 높이 1.0~2.0 스케일을 적용한다.

Offline Reward Modal V1은 `--bg-primary` 56% 딤 오버레이 위에 `--bg-panel` 96% 패널을 중앙 배치한다. 보상 존재 시에만 표시하며 제목은 `오프라인 보상`, 본문은 `경과 시간 H:MM`, `골드 +N`, `EXP +N`, CTA는 `수령`으로 고정한다. 패널 폭은 화면 폭의 약 42%를 사용하되 1080p/1440p/4K에서 420~640px 스케일 범위로 제한하고, `--accent-gold` 2px 테두리와 수령 버튼으로 보상성/클릭 대상을 강조한다.

Quest Log V1은 Q 단축키로 우측 상단에 열리는 `--bg-panel` 94% 패널이다. 제목은 `퀘스트`, 닫기 힌트는 `Q 닫기`, 행 타입은 `메인`/`일일`, 진행 문구는 `진행 n / target`, CTA는 `수령`/`진행`/`완료`로 고정한다. 진행 바는 `--accent-blue`, 수령 가능한 퀘스트의 좌측 강조선과 버튼은 `--accent-gold`를 사용하며, 1080p/1440p/4K에서 Canvas 높이 기준 1.0~2.0 스케일로 기존 HP/EXP/Gold HUD와 스킬 HUD를 가리지 않도록 우측 28px 여백을 유지한다.

Rebirth HUD V1은 기존 Canvas HUD 레이어에 우측 고정 패널로 배치한다. `--bg-panel` 91% 패널과 2px 상태 테두리를 사용하고, `환생 가능` 상태와 `환생 진행` 버튼은 `--accent-gold`, 미충족 상태는 `--text-muted`, `보스 격파 필요`은 `--accent-red`, 레벨 조건 달성은 `--accent-blue`로 구분한다. 문구는 `환생`, `환생 가능`, `보스 격파`, `영구 보너스`, `이번 환생 보상`, `환생 진행`을 기준으로 고정하며, 현재 환생 횟수와 영구 보너스 포인트, `PreviewRebirthReward()` 기반 보상 `+N 포인트`를 한눈에 확인하게 한다. 1080p/1440p/4K에서는 Canvas 높이 기반 1.0~2.0 스케일을 적용하고 폭은 화면의 약 24%를 쓰되 320~440px 범위로 제한해 HP/EXP/Gold, 스킬 HUD, 오프라인 보상 모달과 겹치지 않게 유지한다.

Gold Shop HUD V1은 기존 Canvas HUD 우측 상단 정보열에 `골드 상점` 패널로 배치한다. `--bg-panel` 91% 패널과 2px 상태 테두리를 사용하고, 충분한 골드와 뽑기 CTA는 `--accent-gold`, 골드 부족 상태는 `--accent-red`, 보조 상태 문구는 `--accent-blue` 또는 `--text-muted`를 사용한다. 뽑기 결과는 7단계 희귀도별 rarity 토큰으로 좌측 강조선과 텍스트를 표시하며 실패/차단 결과는 `--accent-red`로 구분한다. Unique는 초록 단색, Transcendent는 청록 단색, Mythic은 `RarityMythicStart` 중심 색과 하단 보조 라인 `RarityMythicEnd`를 함께 사용한다. 패널은 1080p 기준 우측 28px, 상단 92px에 배치하고 폭은 화면의 약 22%를 쓰되 300~380px 범위로 제한한다. 1080p/1440p/4K에서는 Canvas 높이 기반 1.0~2.0 스케일을 적용해 환생 패널(304px 기준 Y), 강화 패널(526px 기준 Y), 스킬 HUD와 겹치지 않게 유지한다.

Enhancement HUD 50-Level V1은 기존 우측 526px 기준 강화 패널 배치를
유지하되, 장착 행의 레벨을 `+N / 50` 형식으로 표시한다. 다음 비용은
희귀도 배수를 반영해 `--text-muted`로, 성공률은 강화 가능 상태에서
`--accent-blue`로 표시하며, 골드 부족 상태는 비용만 `--accent-red`로
전환한다. +50 장비는 레벨과 좌측 강조선을 `--accent-gold`로 유지하고
비용/성공률/버튼 상태 모두 `MAX` 또는 현지화된 최대 문구를 사용한다.
1080p/1440p/4K에서는 패널 폭을 화면의 약 25%이되 360~460px 범위로
제한하고 Canvas 높이 기반 1.0~2.0 스케일을 적용해 골드 상점, 환생
패널, 하단 스킬 HUD와 겹치지 않게 한다.

Item Affix HUD V1은 좌측 하단 장비 요약의 무기 줄 아래에 치명/공속/마공
요약을 한 줄로 추가한다. 0 affix 장비는 기존처럼 별도 줄을 표시하지 않고,
희귀도 색상은 7단계 `RarityToColor` 토큰을 그대로 사용한다. 텍스트는
`치명 +3% / 공속 +0.10 / 마공 +12` 순서로 고정하며 1080p/1440p/4K에서
Canvas 높이 기반 스케일을 유지해 하단 스킬 HUD와 겹치지 않게 한다.

Class Selection HUD V1은 좌측 상단 Canvas HUD 레이어에 `시작 직업` 패널로 배치하고 `전사`/`마법사`/`궁수`/`도적`/`성직자` 5개 선택지를 고정 노출한다. 선택지는 `--bg-primary` 행 위에 현재 선택 직업만 `--accent-gold` 좌측 강조선과 `SELECTED` 상태를 사용하고, 비선택 행은 `--text-muted` 강조선과 `SELECT` 상태로 구분한다. 주 스탯 요약은 전사 `STR/CON`, 마법사 `INT/WIS`, 궁수 `DEX/LUK`, 도적 `DEX/LUK`, 성직자 `WIS/INT`로 표시한다. 5개 행은 34px 기준 압축 행으로 구성하고 1080p/1440p/4K에서는 Canvas 높이 기반 1.0~2.0 스케일로 좌측 28px 여백과 322px 기준 폭을 유지한다. 펫 패널은 클래스 선택 패널 아래 360px 기준 Y, 시즌 패스는 574px 기준 Y로 내려 좌측 정보열끼리 겹치지 않게 하며 하단 스킬 HUD 및 우측 퀘스트/환생 HUD와도 충돌하지 않게 한다.

Pet / Season Pass HUD V1은 기존 Canvas HUD 위에 좌측 세로 정보열로 고정한다. `펫` 패널은 직업 선택 패널 아래 322px 기준 폭으로 배치하고 보유 펫 2종(`Dog`, `Bird`), 현재 장착 상태, `골드 +%` / `드롭 +%` 요약을 함께 보여준다. 장착 중인 펫은 `--accent-gold` 좌측 강조선과 `장착됨` 상태로 표시하고, 미장착 펫은 `장착` 버튼만 `--accent-gold` CTA로 둔다. `시즌 패스` 패널은 펫 패널 아래 420px 기준 폭으로 배치하며, 전체 토큰 진행 바는 `--accent-blue`, 수령 가능한 티어는 `--accent-gold`, 잠긴 티어는 `--text-muted`로 구분한다. 10개 무료 티어는 한 화면에서 스캔 가능하도록 압축 행으로 표시하고 각 행은 `티어`, 필요 토큰, 보상, `받기`/`수령 완료`/`잠김` 상태를 고정한다. 1080p/1440p/4K에서는 Canvas 높이 기반 1.0~2.0 스케일을 적용해 하단 스킬 HUD, 우측 퀘스트/환생 패널과 겹치지 않게 한다.

Chapter 2 Stage HUD V1은 기존 상단 중앙 Stage HUD를 유지하되 `챕터 N`을
제목 행에 별도 보조 라벨로 추가하고, 진행 행은
`스테이지 N-M current/target` 좌표를 유지한다. 챕터 진입/클리어 피드백은
Stage HUD 바로 아래 220px 기준 폭의 얇은 토스트로 표시하며
`--accent-gold` 텍스트와 `--bg-primary` 반투명 배경을 사용한다.
1080p/1440p/4K에서는 Canvas 높이 기반 1.0~2.0 스케일을 적용하고,
피드백 토스트는 상단 HUD 영역 안에서만 페이드되어 하단 스킬 HUD, 우측
환생/퀘스트, 좌측 클래스/펫/시즌 패널과 겹치지 않게 한다.

Floating Damage Text V1은 Canvas로 머리 위 월드 위치(+120uu Z)를 스크린 투영한 뒤 1.0초 동안 32px 상승하며 페이드아웃한다. 물리 피해는 `--text-primary`, 마법 피해는 `--accent-blue`, 치명타는 `--accent-gold`와 1.25x 크기 및 `!` suffix를 사용하며, 1080p/1440p/4K 가독성을 위해 Canvas 높이 기준 1.0~2.0 스케일을 유지한다.

### 3.3 타이포

| 용도 | 폰트 | 비고 |
| --- | --- | --- |
| 본문 (한글) | **Pretendard 400/500/700** | 가독성 |
| 본문 (영문) | **Inter 400/600** | 폴백 |
| 수치 / 데미지 폰트 | **갤멧체** 또는 **DungGeunMo** | 도트 풍 강조 |

### 3.4 모션
- UI: 부드러운 ease-out (200ms), 슬라이드/페이드 위주
- 전투 데미지 텍스트: 위로 튀어오르며 페이드 (350ms)
- 보상 모달: 카드 뒤집기 (450ms) + 파티클

---

## 4. 사운드 (개략)

- BGM: 챕터별 분리, 평화/전투/보스 3종
- SFX: 공격(직업별), 피격, 레벨업, 강화 성공/실패, 보상
- 톤: 8비트 + 오케스트라 미니멀 혼합 (예: Octopath Traveler 분위기)

상세는 사운드 슬라이스 PR (M5 이후) 에서.

---

## 5. 표현 가이드라인

- **금지**: 메이플스토리 IP 의 캐릭터/몬스터/지명/스킬명 직접 차용. 영감만 허용.
- **선호**: 동화풍, 따뜻한 색감, 작은 디테일 (먼지, 풀, 새 그림자)
- **회피**: 과도한 고어, 어두운 호러 톤 (12세 이상 등급 유지)

---

## 6. 작업 산출물 표준

- 캐릭터: PSD (레이어 분리) + PNG export (4x 스케일)
- UI: Figma 컴포넌트 → PNG export + JSON 토큰 (`docs/planning/ui-tokens.json`)
- 애님: Aseprite 원본 + GIF preview
- 모든 에셋은 Git LFS 추적 (`.gitattributes` 참고)

자세한 작업 표준은 디자이너 슬라이스 PR 에서 확장.
