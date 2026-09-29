// Copyright (c) 2026 DaBoysZeroth contributors
// SPDX-License-Identifier: MIT

#include "Config.h"
#include "LFGMgr.h"
#include "Log.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldSession.h"

#include <vector>

namespace InstanceCompletionRewards
{
struct Settings
{
    bool Enabled = false;
    uint32 DungeonItem = 47241;
    uint32 DungeonCount = 30;
    uint32 RaidItem = 49426;
    uint32 RaidCount = 30;
    uint32 MinimumPlayers = 1;
    uint32 MinimumPlayerLevel = 80;
    uint32 MinimumExpansion = 2;
    bool RewardBots = false;
    bool RewardGameMasters = false;
};

Settings gSettings;

void LoadSettings()
{
    gSettings.Enabled = sConfigMgr->GetOption<bool>("InstanceCompletionRewards.Enable", false);
    gSettings.DungeonItem = sConfigMgr->GetOption<uint32>("InstanceCompletionRewards.Dungeon.Item", 47241);
    gSettings.DungeonCount = sConfigMgr->GetOption<uint32>("InstanceCompletionRewards.Dungeon.Count", 30);
    gSettings.RaidItem = sConfigMgr->GetOption<uint32>("InstanceCompletionRewards.Raid.Item", 49426);
    gSettings.RaidCount = sConfigMgr->GetOption<uint32>("InstanceCompletionRewards.Raid.Count", 30);
    gSettings.MinimumPlayers = sConfigMgr->GetOption<uint32>("InstanceCompletionRewards.MinimumPlayers", 1);
    gSettings.MinimumPlayerLevel = sConfigMgr->GetOption<uint32>("InstanceCompletionRewards.MinimumPlayerLevel", 80);
    gSettings.MinimumExpansion = sConfigMgr->GetOption<uint32>("InstanceCompletionRewards.MinimumExpansion", 2);
    gSettings.RewardBots = sConfigMgr->GetOption<bool>("InstanceCompletionRewards.RewardBots", false);
    gSettings.RewardGameMasters = sConfigMgr->GetOption<bool>("InstanceCompletionRewards.RewardGameMasters", false);

    LOG_INFO("module", "Instance Completion Rewards: {}. Dungeon item/count: {}/{}, raid item/count: {}/{}.",
        gSettings.Enabled ? "enabled" : "disabled",
        gSettings.DungeonItem,
        gSettings.DungeonCount,
        gSettings.RaidItem,
        gSettings.RaidCount);
}

bool IsBot(Player const* player)
{
    WorldSession const* session = player ? player->GetSession() : nullptr;
    return session && session->IsHeadless();
}

bool IsEligible(Player const* player)
{
    if (!player || player->GetLevel() < gSettings.MinimumPlayerLevel)
        return false;

    if (!gSettings.RewardGameMasters && player->IsGameMaster())
        return false;

    if (!gSettings.RewardBots && IsBot(player))
        return false;

    return true;
}

class ConfigScript : public WorldScript
{
public:
    ConfigScript()
        : WorldScript("InstanceCompletionRewardsConfigScript", { WORLDHOOK_ON_AFTER_CONFIG_LOAD })
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        LoadSettings();
    }
};

class CompletionScript : public GlobalScript
{
public:
    CompletionScript()
        : GlobalScript("InstanceCompletionRewardsCompletionScript", {
            GLOBALHOOK_ON_AFTER_UPDATE_ENCOUNTER_STATE
        })
    {
    }

    void OnAfterUpdateEncounterState(
        Map* map,
        EncounterCreditType /*type*/,
        uint32 /*creditEntry*/,
        Unit* /*source*/,
        Difficulty /*difficulty*/,
        DungeonEncounterList const* /*encounters*/,
        uint32 dungeonCompleted,
        bool updated) override
    {
        if (!gSettings.Enabled || !map || !map->IsDungeon() || !dungeonCompleted || !updated)
            return;

        lfg::LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(dungeonCompleted);
        if (!dungeon || dungeon->expansion < gSettings.MinimumExpansion)
            return;

        bool const isRaid = map->IsRaid();
        uint32 const item = isRaid ? gSettings.RaidItem : gSettings.DungeonItem;
        uint32 const count = isRaid ? gSettings.RaidCount : gSettings.DungeonCount;
        if (!item || !count)
            return;

        std::vector<Player*> recipients;
        Map::PlayerList const& playerList = map->GetPlayers();

        for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
        {
            Player* player = itr->GetSource();
            if (IsEligible(player))
                recipients.push_back(player);
        }

        if (recipients.size() < gSettings.MinimumPlayers)
            return;

        for (Player* player : recipients)
        {
            if (!player->AddItem(item, count))
            {
                LOG_WARN("module", "Instance Completion Rewards: could not add item {} x{} to player {} after completing dungeon identifier {}.",
                    item,
                    count,
                    player->GetName(),
                    dungeonCompleted);
            }
        }
    }
};
} // namespace InstanceCompletionRewards

void AddSC_mod_instance_completion_rewards()
{
    new InstanceCompletionRewards::ConfigScript();
    new InstanceCompletionRewards::CompletionScript();
}
