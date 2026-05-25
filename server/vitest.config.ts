import { defineConfig } from "vitest/config";

export default defineConfig({
  test: {
    environment: "node",
    globals: true,
    include: ["src/**/*.test.ts", "tests/**/*.test.ts"],
    coverage: {
      provider: "v8",
      reporter: ["text", "lcov"],
      include: [
        "src/core/formulas/**/*.ts",
        "src/modules/**/*.service.ts",
        "src/plugins/rate-limit.ts",
      ],
      exclude: ["src/**/*.schema.ts"],
      thresholds: {
        lines: 80,
        statements: 80,
        branches: 70,
        functions: 80,
      },
    },
  },
});
