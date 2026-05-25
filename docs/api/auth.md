# Auth API

Base path: `/v1/auth`

## POST `/register`

요청:

```json
{ "email": "hero@example.com", "password": "Password123!", "nickname": "전사" }
```

응답:

```json
{ "ok": true, "data": { "user": { "id": "uuid", "email": "hero@example.com", "nickname": "전사" }, "accessToken": "...", "refreshToken": "...", "tokenType": "Bearer", "expiresIn": 900 } }
```

```bash
curl -X POST http://localhost:3000/v1/auth/register -H "content-type: application/json" -d '{"email":"hero@example.com","password":"Password123!","nickname":"전사"}'
```

## POST `/login`

요청:

```json
{ "email": "hero@example.com", "password": "Password123!" }
```

응답은 register와 동일하다. 비밀번호 불일치 시 `401 AUTH_ERROR`.

## POST `/refresh`

요청:

```json
{ "refreshToken": "..." }
```

refresh rotation을 수행하고 새 access/refresh 토큰을 반환한다. 폐기된 토큰은 `401 AUTH_ERROR`.

## POST `/logout`

요청:

```json
{ "refreshToken": "..." }
```

응답:

```json
{ "ok": true, "data": { "loggedOut": true, "refreshTtlSeconds": 2592000 } }
```

Rate limit: 인증 API는 IP 기준 5/min.
