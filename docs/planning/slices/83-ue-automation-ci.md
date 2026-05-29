# PR #83 기획서 — UE5 Automation CI / 로컬 게이트 (인프라)

> **PM 자율 진행 (인프라/devops, PM 직접 — 스크립트/yml/문서)**. 사용자 지시 "인프라 우선". server-ci(GitHub-hosted)가 UE 빌드/Automation을 검증 못 하는 사각지대를 닫는다. client(infra) only.

## 1. 목표 / DoD
server-ci UE 미검증 사각지대로 잠재 red 누적(#73 골드 오버플로/#75 던전패널/#81 SaveVer)을 근본 차단.

### DoD
1. **로컬 게이트** `tools/ci/ue-automation.ps1`: 표준 jumbo(unity) 빌드(-DisableUnity 없이) + 광범위 Automation(기본 전체 IdleProject) + 파싱 → Result={Fail}/비정상 EXIT 시 비0 종료. 실패 테스트+진단 출력. (PM 머지 전 [5] 게이트로 사용)
2. **self-hosted 워크플로** `client-ci.yml`에 `ue5-automation` job(runs-on [self-hosted, windows, ue5], 라벨 `ue/automation` 게이트 → 러너 미등록 시 PR 비차단) 추가, 동일 스크립트 실행.
3. **문서** `docs/ops/ue-automation-ci.md`: 로컬 사용법 + 러너 등록/활성화 절차 + 한계.
4. 기존 server-ci/markdown/guard 회귀 없음. placeholder job 안내 갱신.

## 2. 작업 (PM 직접, Codex 한도 + 인프라 특성)
- 게이트 스크립트(PowerShell) + 워크플로(yml) + ops 문서. PM이 스크립트를 로컬 실행(표준 jumbo 빌드 + 전체 IdleProject Automation)으로 동작·전 저장소 그린 직접 검증.

## 3. 범위 외
- self-hosted 러너 실제 등록(개발책임자 1회 작업 — 문서로 안내). 자동 트리거 승격(러너 상시 가동 후). Automation 리포트 아티팩트 업로드.

## 4. 워크플로우
PM 직접 작성·검증 → server-ci(markdown/yml lint)/guard 그린 → 머지. UE job 자체는 러너 등록 전까지 라벨 게이트로 dormant.

## 5. 리스크
| 리스크 | 완화 |
| --- | --- |
| self-hosted job이 러너 없이 queued 블로킹 | 라벨 `ue/automation` 게이트(라벨 없으면 미실행) |
| 스크립트가 -DisableUnity 우회 | 스크립트가 표준 빌드 강제(adaptive unity 한계 문서화) |
| GitHub-hosted UE 불가 | self-hosted 유일 경로 명시(문서) |
