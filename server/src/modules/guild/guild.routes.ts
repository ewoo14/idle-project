import type { FastifyInstance } from "fastify";
import { pool } from "../../core/db.js";
import { rateLimitPolicies } from "../../plugins/rate-limit.js";
import { GuildRepoPg } from "./guild.repo.js";
import {
  guildAttendanceSchema,
  guildContributeSchema,
  guildCreateSchema,
  guildDonateSchema,
  guildGetSchema,
  guildJoinSchema,
  guildLeaveSchema,
  guildListSchema,
  guildRankSchema,
  guildRequestActionSchema,
  guildShopBuySchema,
  guildShopSchema,
  guildSnapshotSchema,
  guildUpdateSchema,
} from "./guild.schema.js";
import {
  type GuildJoinMode,
  type GuildRank,
  GuildService,
} from "./guild.service.js";

export async function guildRoutes(app: FastifyInstance) {
  const service = new GuildService(new GuildRepoPg(pool));

  // 길드 생성 (무소속, 생성자=길드장)
  app.post(
    "/",
    {
      preHandler: app.authenticate,
      schema: guildCreateSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const body = request.body as { characterId: string; name: string };
      const data = await service.create(
        request.user.sub,
        body.characterId,
        body.name,
      );
      return { ok: true, data };
    },
  );

  // 내 길드 스냅샷 (클라 캐시 소스)
  app.get(
    "/me",
    {
      preHandler: app.authenticate,
      schema: guildSnapshotSchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { characterId: string };
      const data = await service.snapshot(request.user.sub, query.characterId);
      return { ok: true, data };
    },
  );

  // 길드 목록/검색
  app.get(
    "/",
    {
      preHandler: app.authenticate,
      schema: guildListSchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as {
        q?: string;
        limit?: number;
        offset?: number;
      };
      const data = await service.list(query);
      return { ok: true, data };
    },
  );

  // 길드 단건 조회 (공개 정보)
  app.get(
    "/:id",
    {
      preHandler: app.authenticate,
      schema: guildGetSchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const params = request.params as { id: string };
      const data = await service.getById(params.id);
      return { ok: true, data };
    },
  );

  // 가입 (자유=즉시 / 승인=신청)
  app.post(
    "/:id/join",
    {
      preHandler: app.authenticate,
      schema: guildJoinSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string };
      const body = request.body as { characterId: string };
      const data = await service.join(
        request.user.sub,
        body.characterId,
        params.id,
      );
      return { ok: true, data };
    },
  );

  // 탈퇴 (길드장은 위임/해산 규칙)
  app.post(
    "/:id/leave",
    {
      preHandler: app.authenticate,
      schema: guildLeaveSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const body = request.body as { characterId: string };
      const data = await service.leave(request.user.sub, body.characterId);
      return { ok: true, data };
    },
  );

  // 가입 신청 승인 (길드장/부)
  app.post(
    "/:id/requests/:charId/approve",
    {
      preHandler: app.authenticate,
      schema: guildRequestActionSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string; charId: string };
      const body = request.body as { characterId: string };
      const data = await service.approve(
        request.user.sub,
        body.characterId,
        params.id,
        params.charId,
      );
      return { ok: true, data };
    },
  );

  // 가입 신청 거절 (길드장/부)
  app.post(
    "/:id/requests/:charId/reject",
    {
      preHandler: app.authenticate,
      schema: guildRequestActionSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string; charId: string };
      const body = request.body as { characterId: string };
      const data = await service.reject(
        request.user.sub,
        body.characterId,
        params.id,
        params.charId,
      );
      return { ok: true, data };
    },
  );

  // 설정 변경 (name/joinMode=길드장, notice=길드장/부)
  app.patch(
    "/:id",
    {
      preHandler: app.authenticate,
      schema: guildUpdateSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string };
      const body = request.body as {
        characterId: string;
        name?: string;
        notice?: string;
        joinMode?: GuildJoinMode;
      };
      const data = await service.updateSettings(
        request.user.sub,
        body.characterId,
        params.id,
        { name: body.name, notice: body.notice, joinMode: body.joinMode },
      );
      return { ok: true, data };
    },
  );

  // 계급 승강 (길드장만, 인원 해금·정원 검증)
  app.post(
    "/:id/members/:charId/rank",
    {
      preHandler: app.authenticate,
      schema: guildRankSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string; charId: string };
      const body = request.body as { characterId: string; rank: GuildRank };
      const data = await service.setRank(
        request.user.sub,
        body.characterId,
        params.id,
        params.charId,
        body.rank,
      );
      return { ok: true, data };
    },
  );

  // 일일 출석 기여 (1일 1회, +50)
  app.post(
    "/:id/attendance",
    {
      preHandler: app.authenticate,
      schema: guildAttendanceSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string };
      const body = request.body as { characterId: string };
      const data = await service.attendance(
        request.user.sub,
        body.characterId,
        params.id,
      );
      return { ok: true, data };
    },
  );

  // 재화 헌납 기여 (클라 권위 골드, 일일 상한)
  app.post(
    "/:id/donate",
    {
      preHandler: app.authenticate,
      schema: guildDonateSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string };
      const body = request.body as { characterId: string; gold: number };
      const data = await service.donate(
        request.user.sub,
        body.characterId,
        params.id,
        body.gold,
      );
      return { ok: true, data };
    },
  );

  // 전투/던전 자동 기여 델타 플러시 (주간 상한)
  app.post(
    "/:id/contribute",
    {
      preHandler: app.authenticate,
      schema: guildContributeSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string };
      const body = request.body as { characterId: string; amount: number };
      const data = await service.contribute(
        request.user.sub,
        body.characterId,
        params.id,
        body.amount,
      );
      return { ok: true, data };
    },
  );

  // 길드 상점 카탈로그 (멤버)
  app.get(
    "/:id/shop",
    {
      preHandler: app.authenticate,
      schema: guildShopSchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const params = request.params as { id: string };
      const query = request.query as { characterId: string };
      const data = await service.shop(
        request.user.sub,
        query.characterId,
        params.id,
      );
      return { ok: true, data };
    },
  );

  // 길드 상점 구매 (포인트 차감 + 보상 반환)
  app.post(
    "/:id/shop/:itemId/buy",
    {
      preHandler: app.authenticate,
      schema: guildShopBuySchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string; itemId: string };
      const body = request.body as { characterId: string };
      const data = await service.shopBuy(
        request.user.sub,
        body.characterId,
        params.id,
        params.itemId,
      );
      return { ok: true, data };
    },
  );
}
