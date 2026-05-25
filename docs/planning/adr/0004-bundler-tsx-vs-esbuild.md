# ADR-0004 실행/빌드 방식: tsx 개발 + tsc 빌드

## 상태

채택

## 맥락

개발 서버는 빠른 재시작이 필요하고, CI/이미지는 타입 검증이 확실해야 한다. 후보는 tsx, esbuild 번들, tsc 빌드였다.

## 결정

개발은 `tsx watch`, 빌드는 `tsc`를 사용한다. 별도 번들은 도입하지 않는다.

## 이유

tsx는 ESM TypeScript 실행이 단순하고 로컬 개발 피드백이 빠르다. 서버 코드는 번들 크기보다 타입 안정성과 stack trace가 중요하므로 CI와 Docker 빌드에서는 `tsc` 산출물을 사용한다. esbuild 번들은 이후 cold start 또는 배포 크기가 문제가 될 때 재검토한다.
