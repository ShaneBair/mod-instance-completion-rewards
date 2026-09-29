# Instance Completion Rewards Module Guidance

## Repository Role

This repository is a standalone compiled AzerothCore module for the private
WotLK 3.3.5a Playerbots-compatible server described by `C:\src\AGENTS.md`.
It awards configurable emblem items once when a configured final dungeon or
raid encounter is completed.

Keep this feature independent from AutoBalance. It must not depend on
AutoBalance data, settings, or level scaling.

## Behavior Contract

- Use `GlobalScript::OnAfterUpdateEncounterState` as the completion signal.
- Require both a nonzero `dungeonCompleted` identifier and `updated == true`.
  Together these represent the first completion of the final configured
  encounter in an instance.
- Do not restrict the encounter credit type. Some valid final encounters use
  spell credit rather than creature-kill credit.
- Use the completion identifier to read expansion metadata from `LFGMgr`.
- Treat `Map::IsDungeon()` as covering both dungeon and raid instance maps;
  use `Map::IsRaid()` to select the reward category.
- Determine Playerbots through `WorldSession::IsHeadless()` on this target
  Playerbots-compatible core. Do not use account names or account IDs.
- Count only reward-eligible players toward the configured minimum.
- Keep rewards disabled by default and do not modify normal loot or LFG quest
  rewards.

## Operational Safety

- Do not enable, install, deploy, build, or restart the live realm without
  explicit user authorization.
- The module owns no database schema.
- Configuration belongs under the `InstanceCompletionRewards.*` prefix.
- AutoBalance's experimental reward feature must remain disabled when this
  module is enabled, or players could receive two independent rewards.

## Verification

- Check configuration names and documented defaults for consistency.
- Compile inside the exact Playerbots-compatible AzerothCore source tree before
  deployment.
- Test non-final bosses, kill-credit endings, spell-credit endings, duplicate
  completion credit, dungeon/raid selection, minimum-player behavior, bot/GM
  filtering, and full-inventory failure.
- Never claim gameplay or deployment verification based only on source review.
