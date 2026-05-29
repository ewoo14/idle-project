# 룬 리롤 + 전송 QA 검증 노트 (룬 시스템 확장4)

> 작성 2026-05-30 · 검증: PM · **세이브 변경 없음(SaveVer 19 유지)**

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/core/formulas`) | rune 비용/확률 parity 함수 | **rune 25/0** (전체 565/0) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation(클라) | Rune(리롤/등급/전송/parity)/GameCore(회귀) | **106/0** |
| UE Automation(UI) | Rune/Localization(CsvIntegrity)/UI | **67/0** |

## 2. 시나리오 커버리지

- **세트 리롤**: 룬 정수 차감, ERuneSet 랜덤 재설정(Offense~Fortune), 현재와 같아도 소비.
- **등급 상승 시도**: chance=1 강제 성공(+1단계)/chance=0 강제 실패(불변)/Mythic 거부(시도 불성립·자원 무차감)/자원 부족 거부. RNG 클라 권위, 서버 함수 parity.
- **전송**: Dst=max(Dst,Src), Src 삭제(보유 수 감소), 장착 슬롯 인덱스 보정(밀린 인덱스 감소·src 슬롯 해제), 동일/무효 인덱스 거부, 룬 정수 비용.
- **parity**: 클라 RuneFormula ↔ 서버 rune.ts 비용/확률(fround float) 1:1 앵커.

## 3. PM 적발·수정 (실버그)

- `TransferEnhancement`에서 `EnsureSlotCount()`가 인덱스 보정 **전**에 호출돼, src 삭제로 범위 밖이 된 후속 장착 인덱스가 보정 전에 INDEX_NONE으로 소실되던 버그 → EnsureSlotCount를 보정 루프 **뒤로 이동**. UE Automation이 1차 적발(transfer 슬롯 정합 단언) → fix 후 GREEN.

## 4. 한계 / 후속

- 등급 상승 RNG는 클라 권위(로컬 세이브) — 서버는 비용/확률 parity 미러(라우트/DB 없음). 치트 표면은 기존 강화(#71)와 동일 신뢰 경계.
- 직업룬 등급 상승·RuneType 리롤은 후속(동일 패턴). 수치는 초기값 재튜닝 대상. PR 본문 룬 패널 스크린샷.
