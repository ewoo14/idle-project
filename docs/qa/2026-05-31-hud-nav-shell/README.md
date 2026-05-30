# HUD 내비게이션 셸 — PIE 시각 검증 스크린샷 (PR #110)

UE5 오프스크린 렌더(`-game -RenderOffScreen` + `HighResShot`)로 캡처. 임시 디버그(패널 강제 오픈·모달 일시 비표시·레이아웃 모드 강제)는 캡처 후 전부 되돌려 **소스/PR에 무영향**(git diff 0).

> 헤드리스 뷰포트가 1280×720으로 고정되어 실제 세로 해상도 렌더는 불가 → 모바일은 코드에서 모드 강제로 크롬 확인(가로로 늘어나 보이나 구성 동일). 실기기 세로 비율은 모바일 프리뷰에서 최종 확인 권장.

## Desktop (좌측 레일 + 우측 도킹)
| 파일 | 카테고리 → 패널 |
| --- | --- |
| `desktop-combat-tower.png` | 전투 → 무한의 탑 |
| `desktop-growth-stats.png` | 성장 → 스탯 분배 |
| `desktop-rebirth.png` | 환생 → 환생 |
| `desktop-gear-enhance.png` | 장비 → 강화 (+ 서브탭 강화/잠재/룬/룬도감/상점) |
| `desktop-collection.png` | 수집 → 펫/업적 |
| `desktop-daily-quest.png` | 일일 → 퀘스트 |
| `desktop-social-guild.png` | 소셜 → 길드 |

공통: 좌측 7카테고리 레일, 우측 도킹 패널 1개, 상시 요소(스테이지/보스/스킬바/속성범례/궁극기) 노출, 전투 씬 보임, 디밍·닫기 X 없음.

## Mobile (하단 탭바 + 중앙 오버레이)
| 파일 | 내용 |
| --- | --- |
| `mobile-overlay-enhance.png` | 하단 7카테고리 탭바 + 중앙 오버레이(강화) + 전체 디밍 + 닫기 X + 전투 씬 비침 |
