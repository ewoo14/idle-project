# 출석 보상 QA 검증 노트

> 작성 2026-05-30 · 검증: PM · SaveVer 22→23

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/core/formulas`) | attendance 마일스톤/보상 parity | **attendance 9/0** (전체 607/0) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation(클라) | Attendance(체크인/마일스톤/수령)/GameCore(SaveSystem v23) | **97/0** |
| UE Automation(UI) | Attendance/Localization(CsvIntegrity 18키)/UI | **40/0** |

## 2. 시나리오 커버리지

- **체크인**: 하루 1회(UTC date), 중복 거부, 날짜 변경 시 누적++.
- **마일스톤**: getReachedMilestones 경계(threshold-1/threshold, total 34→7), 무한 기하 임계(f32 fround, n=6→20).
- **수령**: ClaimMilestone(도달 수령·미달 거부·중복 거부·비순차), 보상 단일 지급(gold/essence/consumable).
- **parity**: 클라 AttendanceService ↔ 서버 attendance.ts 임계(f32 powf)/보상(3주기) 1:1.
- **세이브 v23**: TotalAttendance/LastAttendanceDate/ClaimedMilestones 라운드트립, SaveVer<23=0/빈(회귀안전).

## 3. 후속/비고

- getter lazy-ensure(#91 교훈) 선반영 — Init 미경유 컨텍스트 null 방지.
- 연속 출석(streak) 보너스·월간 캘린더·복귀 보상은 후속.
- 수치(임계/보상) 초기값 재튜닝. PR 본문 출석 패널 스크린샷.
