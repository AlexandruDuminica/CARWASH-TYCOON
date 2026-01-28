#pragma once

#include <memory>
#include <vector>
#include <string>
#include <ostream>

#include "Achievement.h"

class CarWash;

class AchievementManager {
    std::vector<std::unique_ptr<Achievement> > list_;
    int totalSuppliesPacks_{0};
    int totalUpgrades_{0};
    int perfectDays_{0};

    void dispatch(CarWash &game, const AchievementEvent &ev);

public:
    AchievementManager();

    void onServed(CarWash &game, int cars, double satisfaction, double revenue);

    void onLost(CarWash &game);

    void onDayEnd(CarWash &game, int day, int served, int lost, double revenue, double avgSat);

    void onBuySupplies(CarWash &game, const std::string &item, int packs, double cost);

    void onBuyUpgrade(CarWash &game, int upgradeId, double cost);

    void onStructureChanged(CarWash &game);

    int totalSuppliesPacks() const noexcept { return totalSuppliesPacks_; }
    int totalUpgrades() const noexcept { return totalUpgrades_; }
    int perfectDays() const noexcept { return perfectDays_; }

    void print(std::ostream &os) const;
};
