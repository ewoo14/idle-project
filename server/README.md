# server/ — Node.js 백엔드

> 본 디렉터리는 **PR #7 (M3 백엔드 V1)** 시점에 스캐폴드됩니다. 현재는 자리만.

## 예정 스택

- Node.js 22 LTS
- TypeScript 5.5
- Fastify 5 (HTTP)
- Drizzle ORM (PostgreSQL)
- Redis 7 (세션/캐시)
- vitest (테스트)
- pino (로그)

## 예정 구조

```
server/
├── src/
│   ├── modules/
│   │   ├── auth/
│   │   ├── save/
│   │   ├── leaderboard/
│   │   ├── event/
│   │   └── admin/
│   ├── core/
│   │   ├── db.ts
│   │   ├── redis.ts
│   │   ├── logger.ts
│   │   ├── errors.ts
│   │   └── formulas/         # 클라이언트와 미러
│   ├── plugins/
│   └── server.ts
├── migrations/                # Drizzle SQL
├── tests/                     # vitest
├── Dockerfile
├── tsconfig.json
└── package.json
```

## 개발 (PR #7 이후)

```powershell
cd server
npm ci
cp .env.example .env
npm run db:migrate
npm run dev
```

## API 명세

`docs/api/` 참고.
