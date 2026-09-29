# Configurable Completion Emblem Rewards

Status: Implemented locally; compile and in-game verification pending

## Problem and Outcome

Dungeon and raid emblem rewards are lower than desired for this private realm.
The operator needs an exact, configurable number of emblems awarded once at
the end of a run. The feature must remain independent of AutoBalance so module
updates and scaling choices do not affect the reward economy.

The initial local target is 30 Emblems of Triumph after a completed dungeon
and 30 Emblems of Frost after a completed raid.

## Completion Contract

The module listens to AzerothCore's
`GlobalScript::OnAfterUpdateEncounterState` hook. A completion reward is
eligible only when:

- the module is enabled;
- `dungeonCompleted` is nonzero, identifying the final encounter configured in
  the world database for that dungeon or raid difficulty;
- `updated` is true, meaning the encounter newly changed the instance's
  completed-encounter mask;
- the map is a dungeon or raid;
- the completed instance meets the configured minimum expansion;
- enough reward-eligible players are currently present.

The module does not filter by encounter credit type. Both creature-kill and
spell-credit final encounters are valid. This covers endings such as Halls of
Reflection.

Requiring `updated` prevents a repeated credit for the same final encounter in
the same instance from issuing a second reward. A reset or new instance has a
new completion lifecycle and may reward normally.

## Configuration

```ini
InstanceCompletionRewards.Enable = 0

InstanceCompletionRewards.Dungeon.Item = 47241
InstanceCompletionRewards.Dungeon.Count = 30

InstanceCompletionRewards.Raid.Item = 49426
InstanceCompletionRewards.Raid.Count = 30

InstanceCompletionRewards.MinimumPlayers = 1
InstanceCompletionRewards.MinimumPlayerLevel = 80
InstanceCompletionRewards.MinimumExpansion = 2
InstanceCompletionRewards.RewardBots = 0
InstanceCompletionRewards.RewardGameMasters = 0
```

Expansion values follow WotLK client data: `0` is Classic, `1` is The Burning
Crusade, and `2` is Wrath of the Lich King. The default therefore prevents
farming older, easier instances for current emblems while covering all WotLK
instances, including those whose minimum entry level is below 71.

A zero item entry or zero count disables that reward category. The module is
disabled by default so installing it cannot silently alter the server economy.

## Player Eligibility

A player is eligible when all of the following are true:

- the player is still present in the completed instance;
- the player meets `MinimumPlayerLevel`;
- the player is not a headless Playerbot unless `RewardBots` is enabled;
- the player is not a game master unless `RewardGameMasters` is enabled.

Only eligible reward recipients count toward `MinimumPlayers`.

## Reward Selection and Delivery

`Map::IsRaid()` selects the raid item and count; other instance maps select the
dungeon item and count. The module uses `Player::AddItem` to deliver the exact
configured amount. If inventory capacity prevents the award, the module logs a
warning. It does not mail the reward or retry automatically.

## In Scope

- Standalone AzerothCore module and configuration file.
- Exact, independently configurable dungeon and raid item entries and counts.
- Completion-only delivery with duplicate protection from the encounter mask.
- WotLK expansion gating by default.
- Configurable player minimum, player level, bot inclusion, and GM inclusion.
- Configuration reload support through AzerothCore's normal config reload.

## Out of Scope

- Changes to AutoBalance, boss loot tables, or Dungeon Finder reward quests.
- Database schema or data changes.
- Rewards for players who left the map or are offline.
- Mail fallback, delayed retry, or recovery after inventory failure.
- Inferring completion for instances missing a valid final-encounter marker.
- Live deployment, server rebuild, or realm restart as part of this task.

## Acceptance Criteria

1. Disabled configuration grants no completion reward.
2. A non-final encounter grants no completion reward.
3. The first final-encounter completion grants the exact configured amount to
   every eligible player present.
4. Repeated final credit in the same instance grants nothing further.
5. Kill-credit and spell-credit final encounters are both supported.
6. Dungeons and raids select their independently configured item and count.
7. Setting an item or count to zero disables that category.
8. Content below `MinimumExpansion` receives no reward.
9. Player level, bot, GM, and minimum-player rules behave as configured.
10. The module has no dependency on AutoBalance or level scaling.

## Verification

- Validate configuration and documentation consistency.
- Run whitespace and source-level checks locally.
- Later, with explicit operator approval, compile inside the compatible
  Playerbots AzerothCore tree.
- Later, test representative dungeon, raid, and spell-credit completions on a
  non-production or scheduled-maintenance realm.

## Unresolved Decisions

None for the initial implementation. Mail fallback can be specified separately
if inventory-full losses become an operational concern.
