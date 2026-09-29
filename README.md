# Instance Completion Rewards

`mod-instance-completion-rewards` is a standalone module for the
Playerbots-compatible AzerothCore WotLK server. It awards a configurable item
and exact quantity once when the final encounter of a qualifying dungeon or
raid is completed.

The module is intentionally independent of AutoBalance. Keep
`AutoBalance.reward.enable = 0` when enabling this module to avoid double
rewards.

## Default reward policy

- The module is disabled until explicitly enabled.
- WotLK dungeons award 30 Emblems of Triumph (`47241`).
- WotLK raids award 30 Emblems of Frost (`49426`).
- Recipients must be level 80 and present when the run completes.
- Headless Playerbots and game masters are excluded.
- At least one eligible recipient must be present.

All policy values are configurable in
`conf/mod_instance_completion_rewards.conf.dist`.

## Completion behavior

The module uses AzerothCore's configured final-encounter marker. It requires a
new update to the instance encounter mask, so an earlier boss or repeated final
credit in the same instance cannot award another completion reward. Both
creature-kill and spell-credit endings are supported.

Instances without a valid final marker in AzerothCore's `instance_encounters`
data cannot produce a completion reward.

## Installation

1. Place the repository under the AzerothCore source tree's `modules`
   directory.
2. Configure and build AzerothCore normally.
3. Copy `conf/mod_instance_completion_rewards.conf.dist` into the installed
   module configuration directory as
   `mod_instance_completion_rewards.conf`.
4. Set `InstanceCompletionRewards.Enable = 1` and review the reward policy.
5. Keep `AutoBalance.reward.enable = 0`.
6. Restart the worldserver after installing the newly built module. Later
   configuration-only changes can use AzerothCore's normal config reload.

Do not install or rebuild while the live realm must remain uninterrupted.

## Verification

Before production use:

- Compile against the exact Playerbots-compatible core used by the server.
- Verify an earlier boss awards nothing.
- Complete a WotLK dungeon and raid and confirm each eligible human receives
  exactly the configured amount.
- Verify a spell-credit ending such as Halls of Reflection.
- Confirm repeated completion credit does not duplicate the award.
- Confirm excluded bots, game masters, and under-level characters receive
  nothing.
- Confirm a full inventory produces a server warning and no silent duplicate.

The detailed behavior contract is in
`specs/configurable-completion-emblem-rewards.md`.
