오프라인 보상 V1 의 서버 공식 + 라우트를 구현하라. 기획서: docs/planning/slices/16-offline-rewards-v1.md (먼저 읽고 따른다). 기존 패턴 준수: server/src/core/formulas/ (combat.ts/level.ts + *.test.ts Vitest), server/src/modules/ (auth/save/character — repo+route 패턴), server/src/db/schema.ts.

구현 범위 (이번 backend 호출):
1. server/src/core/formulas/offline.ts:
   - 상수: OFFLINE_CAP_SECONDS = 12*3600, OFFLINE_EFFICIENCY = 0.75.
   - computeOfflineRewards(input: { level:number, lastSeenUnixSec:number, nowUnixSec:number, rebirthCount?:number }): { cappedSeconds:number, gold:number, exp:number, timeBonusMultiplier:number }
     * elapsed=max(0, now-lastSeen), capped=min(elapsed, CAP).
     * 초당 기준 골드/EXP: level 기반 1차식(기존 level.ts/combat.ts 재사용 가능하면 재사용, 아니면 단순 baseGoldPerSec(level)/baseExpPerSec(level)). × OFFLINE_EFFICIENCY × timeBonus.
     * timeBonus 함수 분리(곡선 교체 용이): 1 + 환생보너스(0.05*rebirthCount) + 시간곡선(1차값, V1 평탄 또는 소폭). 명확히 함수화.
   - gold/exp 는 정수 반올림.
2. Vitest offline.test.ts: 상한(>12h→12h), 0경과→0, 효율 적용, 환생보너스, 단조 증가, 곡선.
3. server/src/modules/offline/: GET /v1/offline/preview + POST /v1/offline/claim (인증 사용자, lastSeen 기준 산출/수령+재화 반영+lastSeen 갱신). save 모듈/스키마의 lastSeen 활용(없으면 schema 에 최소 추가 + drizzle 마이그레이션). 기존 라우트 등록 방식(main.ts/플러그인) 따름.
4. 통합/라우트 테스트는 가능 범위에서(기존 integration 패턴).

제약: TypeScript 5.5, 한글 주석, 기존 에러/응답 컨벤션. 클라(UE)·모달·아트는 범위 외(후속 보조). 가능하면 `npm test`(vitest) + `tsc -p tsconfig.json` + lint 로 검증. push 금지. 커밋 prefix: codex(backend):.
