# PR #54 Codex Backend Notes

## Scope

- Updated save upload validation for the current client growth model.
- Kept the server-side anti-cheat floor checks for `level`, `rebirthCount`, and `gold`.
- No migration was required because save payloads are stored as JSONB and the character table already stores the authoritative floor fields used by this change.

## Backend Decisions

- Save `level` cap is now 1000. `LEVEL_CAP` and save schema use the same ceiling so `cumulativeExp(level)` keeps growing beyond the old 200 save cap.
- `maxEquipmentGrade` now allows 0~6 to match `EItemRarity` Common through Mythic.
- `transcendCount`, `towerHighestFloor`, and `skillPoints` are accepted as optional extension fields and rejected only when present with a non-integer or negative value. Unknown client payload fields remain preserved through `additionalProperties: true`.
- `totalExp` validation is a lower-bound check against `cumulativeExp(level)` with the existing max(1%, 1) tolerance. This keeps obvious under-reporting blocked while allowing rebirth/transcend history to carry total exp above the current level floor.

## Verification

- RED: `npm test -- save.test.ts` failed on grade 6, high level cap, totalExp upper-bound rejection, negative extension fields, and schema bound assertions.
- RED: `npm test -- level.test.ts save.test.ts` also failed on the old `LEVEL_CAP = 200` and capped cumulative exp behavior.
- GREEN: `npm test -- level.test.ts save.test.ts` passed after implementation.

## Follow-up for Character/UI/QA

- Character payload mapping should send `transcendCount`, `towerHighestFloor`, and `skillPoints` when available, while continuing to include required `level`, `rebirthCount`, and `maxEquipmentGrade`.
- UI sync indicators do not need special backend fields for this change; they can use the existing success/error envelope from Save API.
- QA should include Mythic grade upload, grade 7 rejection, level 1000 acceptance, level 1001 rejection, negative extension-field rejection, and server-progress regression rejection.

# PR #54 Codex Character Notes

## Scope

- Added client cloud-save upload/download plumbing on top of the existing v2 local save model.
- Kept local `UIdleSaveGame` schema at version 2 and upload the cloud API version as `3`, so local save compatibility from #52/#53 remains unchanged.
- Added a pure cloud merge policy and a payload mapper so cloud conflict decisions and JSON shape are Automation-covered without live backend dependency.

## Character Decisions

- `FCloudSaveMergePolicy` compares `rebirthCount`, then `level`, then `gold`, then `lastSeenUnixSec`. Server wins only when it is strictly ahead at the first differing field; exact ties keep local to avoid a pointless overwrite.
- Cloud payload includes the backend-required `level`, `rebirthCount`, `maxEquipmentGrade` plus optional `totalExp`, `gold`, `lastSeenUnixSec`, `transcendCount`, `towerHighestFloor`, and `skillPoints`.
- Full local save state is embedded under `clientSave` in the payload. The backend preserves it through `additionalProperties`, and the client uses it for v2 local-save round-trip recovery.
- `UApiClient` now supports `EnsureCharacter`, `UploadSave`, and `DownloadSave` against the actual backend routes: `/v1/characters/` and `/v1/save/`.
- `UIdleGameInstance` now performs best-effort `SyncFromCloud()` during init and `UploadToCloud()` after successful local saves. Any auth/network/payload failure moves sync state to `Offline` or `Conflict` without blocking local save/load.

## Automation Coverage

- RED: `Build.bat IdleProjectEditor Win64 Development` failed on missing `GameCore/CloudSaveMergePolicy.h` after adding the new SaveSystem tests.
- GREEN: `Build.bat IdleProjectEditor Win64 Development` exited 0.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds='Automation RunTests IdleProject.GameCore.SaveSystem; Quit'` exited 0; 13 SaveSystem tests found, including `CloudMergePolicy` and `CloudPayloadRoundTrip`, all `Result={Success}`.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds='Automation RunTests IdleProject; Quit'` exited 0; 147 tests found and the log ended with `TEST COMPLETE. EXIT CODE: 0`.
