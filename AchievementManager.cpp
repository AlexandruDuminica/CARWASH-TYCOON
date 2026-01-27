#include "AchievementManager.h"
#include "CarWash.h"

#include <algorithm>
#include <iomanip>

namespace {
    static std::string rarityToStr(AchievementRarity r) {
        switch (r) {
            case AchievementRarity::Common: return "Common";
            case AchievementRarity::Rare: return "Rare";
            case AchievementRarity::Epic: return "Epic";
            case AchievementRarity::Legendary: return "Legendary";
        }
        return "Common";
    }

    static std::string catToStr(AchievementCategory c) {
        switch (c) {
            case AchievementCategory::Operations: return "Operations";
            case AchievementCategory::Finance: return "Finance";
            case AchievementCategory::Reputation: return "Reputation";
            case AchievementCategory::Management: return "Management";
            case AchievementCategory::Hidden: return "Hidden";
        }
        return "Operations";
    }
} // namespace

void Achievement::unlock(CarWash &game) {
    if (unlocked_) return;
    unlocked_ = true;

    if (reward_.cash != 0.0) game.adjustCash(reward_.cash);
    if (reward_.demand != 0) game.increaseBaseDemand(reward_.demand);
    if (reward_.speed != 0.0) game.increaseSpeedFactor(reward_.speed);
    if (reward_.comfort != 0.0) game.increaseComfortBonus(reward_.comfort);

    game.logEvent("ACHIEVEMENT UNLOCKED: " + name_);
}

void Achievement::print(std::ostream &os) const {
    os << (unlocked_ ? "[UNLOCKED] " : "[LOCKED] ");
    os << displayName() << " | " << displayDesc();
    os << " | " << catToStr(category()) << " | " << rarityToStr(rarity());
    os << " | hidden=" << (hidden() ? "1" : "0");
    os << " | " << progress() << "/" << target();
}

std::ostream &operator<<(std::ostream &os, const Achievement &a) {
    a.print(os);
    return os;
}

namespace {
    struct ThresholdAch final : Achievement {
        enum class Metric {
            CarsServedTotal, CashTotal, ReputationScore, BaysCount, ServicesCount, UpgradesBought, SuppliesPacks,
            PerfectDays
        };

        Metric metric_;
        int threshold_{1};

        ThresholdAch(std::string id, std::string name, std::string desc,
                     AchievementCategory cat, AchievementRarity rar,
                     Metric m, int threshold, const AchievementReward &reward, bool hidden = false)
            : Achievement(std::move(id), std::move(name), std::move(desc), cat, rar, threshold, reward, hidden),
              metric_(m),
              threshold_(threshold) {
        }

        void onEvent(CarWash &game, const AchievementEvent &) override {
            if (unlocked_) return;

            int value = 0;
            switch (metric_) {
                case Metric::CarsServedTotal: value = game.totalCarsServed();
                    break;
                case Metric::CashTotal: value = static_cast<int>(game.totalCash());
                    break;
                case Metric::ReputationScore: value = static_cast<int>(game.reputationScore() * 100.0);
                    break;
                case Metric::BaysCount: value = game.bayCount();
                    break;
                case Metric::ServicesCount: value = game.serviceCount();
                    break;
                case Metric::UpgradesBought: value = game.upgradesBought();
                    break;
                case Metric::SuppliesPacks: value = game.suppliesPacksBought();
                    break;
                case Metric::PerfectDays: value = game.perfectDaysCount();
                    break;
            }

            progress_ = std::min(value, threshold_);
            if (value >= threshold_) unlock(game);
        }
    };

    struct PerfectDayAch final : Achievement {
        int minServed_{0};

        PerfectDayAch(std::string id, std::string name, std::string desc,
                      AchievementCategory cat, AchievementRarity rar,
                      int minServed, const AchievementReward &reward)
            : Achievement(std::move(id), std::move(name), std::move(desc), cat, rar, 1, reward, false),
              minServed_(minServed) {
        }

        void onEvent(CarWash &game, const AchievementEvent &ev) override {
            if (unlocked_) return;
            if (ev.type != AchievementEventType::DayEnd) return;

            const bool ok = (ev.dailyLost == 0 && ev.dailyServed >= minServed_);
            progress_ = ok ? 1 : 0;
            if (ok) unlock(game);
        }
    };

    struct RevenueDayAch final : Achievement {
        double minRevenue_{0.0};

        RevenueDayAch(std::string id, std::string name, std::string desc,
                      AchievementCategory cat, AchievementRarity rar,
                      double minRevenue, const AchievementReward &reward, bool hidden = false)
            : Achievement(std::move(id), std::move(name), std::move(desc), cat, rar, 1, reward, hidden),
              minRevenue_(minRevenue) {
        }

        void onEvent(CarWash &game, const AchievementEvent &ev) override {
            if (unlocked_) return;
            if (ev.type != AchievementEventType::DayEnd) return;

            const bool ok = (ev.dailyRevenue >= minRevenue_);
            progress_ = ok ? 1 : 0;
            if (ok) unlock(game);
        }
    };

    struct AvgSatDayAch final : Achievement {
        double minAvg_{0.0};

        AvgSatDayAch(std::string id, std::string name, std::string desc,
                     AchievementCategory cat, AchievementRarity rar,
                     double minAvg, const AchievementReward &reward, bool hidden = false)
            : Achievement(std::move(id), std::move(name), std::move(desc), cat, rar, 1, reward, hidden),
              minAvg_(minAvg) {
        }

        void onEvent(CarWash &game, const AchievementEvent &ev) override {
            if (unlocked_) return;
            if (ev.type != AchievementEventType::DayEnd) return;

            const bool ok = (ev.dailyServed > 0 && ev.dailyAvgSat >= minAvg_);
            progress_ = ok ? 1 : 0;
            if (ok) unlock(game);
        }
    };
} // namespace

AchievementManager::AchievementManager() {
    list_.push_back(std::make_unique<ThresholdAch>(
        "ops_first_wash", "First Wash", "Serve your first customer",
        AchievementCategory::Operations, AchievementRarity::Common,
        ThresholdAch::Metric::CarsServedTotal, 1,
        AchievementReward{20.0, 0, 0.0, 0.0}));

    list_.push_back(std::make_unique<ThresholdAch>(
        "ops_50", "Busy Day", "Serve 50 total cars",
        AchievementCategory::Operations, AchievementRarity::Common,
        ThresholdAch::Metric::CarsServedTotal, 50,
        AchievementReward{50.0, 1, 0.0, 0.0}));

    list_.push_back(std::make_unique<ThresholdAch>(
        "ops_200", "Carwash Machine", "Serve 200 total cars",
        AchievementCategory::Operations, AchievementRarity::Rare,
        ThresholdAch::Metric::CarsServedTotal, 200,
        AchievementReward{150.0, 2, 0.05, 0.0}));

    list_.push_back(std::make_unique<ThresholdAch>(
        "fin_200cash", "Positive Cashflow", "Reach 200 EUR cash",
        AchievementCategory::Finance, AchievementRarity::Common,
        ThresholdAch::Metric::CashTotal, 200,
        AchievementReward{30.0, 0, 0.0, 0.0}));

    list_.push_back(std::make_unique<ThresholdAch>(
        "fin_1000cash", "Stacking Bills", "Reach 1000 EUR cash",
        AchievementCategory::Finance, AchievementRarity::Rare,
        ThresholdAch::Metric::CashTotal, 1000,
        AchievementReward{200.0, 1, 0.0, 0.0}));

    list_.push_back(std::make_unique<ThresholdAch>(
        "rep_400", "Trusted Brand", "Reputation score reaches 4.00",
        AchievementCategory::Reputation, AchievementRarity::Rare,
        ThresholdAch::Metric::ReputationScore, 400,
        AchievementReward{0.0, 2, 0.0, 0.05}));

    list_.push_back(std::make_unique<ThresholdAch>(
        "rep_470", "Local Legend", "Reputation score reaches 4.70",
        AchievementCategory::Reputation, AchievementRarity::Epic,
        ThresholdAch::Metric::ReputationScore, 470,
        AchievementReward{150.0, 3, 0.0, 0.10}));

    list_.push_back(std::make_unique<ThresholdAch>(
        "mgmt_supplies_5", "Restocked", "Buy 5 supply packs total",
        AchievementCategory::Management, AchievementRarity::Common,
        ThresholdAch::Metric::SuppliesPacks, 5,
        AchievementReward{0.0, 1, 0.0, 0.0}));

    list_.push_back(std::make_unique<ThresholdAch>(
        "mgmt_upg_1", "First Upgrade", "Buy your first upgrade",
        AchievementCategory::Management, AchievementRarity::Common,
        ThresholdAch::Metric::UpgradesBought, 1,
        AchievementReward{0.0, 0, 0.05, 0.0}));

    list_.push_back(std::make_unique<ThresholdAch>(
        "mgmt_upg_3", "Fully Invested", "Buy 3 upgrades total",
        AchievementCategory::Management, AchievementRarity::Rare,
        ThresholdAch::Metric::UpgradesBought, 3,
        AchievementReward{0.0, 1, 0.10, 0.05}));

    list_.push_back(std::make_unique<PerfectDayAch>(
        "ops_perfect_day", "Perfect Day", "End a day with 0 lost customers and 5+ served",
        AchievementCategory::Operations, AchievementRarity::Epic,
        5, AchievementReward{100.0, 2, 0.05, 0.05}));

    list_.push_back(std::make_unique<RevenueDayAch>(
        "fin_big_day", "Big Day", "End a day with 120+ EUR revenue",
        AchievementCategory::Finance, AchievementRarity::Rare,
        120.0, AchievementReward{80.0, 1, 0.0, 0.0}, false));

    list_.push_back(std::make_unique<AvgSatDayAch>(
        "hidden_perfectionist", "Perfectionist", "End a day with avg satisfaction >= 4.80",
        AchievementCategory::Hidden, AchievementRarity::Legendary,
        4.80, AchievementReward{200.0, 3, 0.05, 0.10}, true));
}

void AchievementManager::dispatch(CarWash &game, const AchievementEvent &ev) {
    for (auto &a: list_) a->onEvent(game, ev);
}

void AchievementManager::onServed(CarWash &game, int cars, double satisfaction, double revenue) {
    AchievementEvent ev;
    ev.type = AchievementEventType::Served;
    ev.cars = cars;
    ev.satisfaction = satisfaction;
    ev.revenue = revenue;
    dispatch(game, ev);
}

void AchievementManager::onLost(CarWash &game) {
    AchievementEvent ev;
    ev.type = AchievementEventType::Lost;
    dispatch(game, ev);
}

void AchievementManager::onDayEnd(CarWash &game, int day, int served, int lost, double revenue, double avgSat) {
    if (lost == 0 && served >= 5) perfectDays_++;

    AchievementEvent ev;
    ev.type = AchievementEventType::DayEnd;
    ev.day = day;
    ev.dailyServed = served;
    ev.dailyLost = lost;
    ev.dailyRevenue = revenue;
    ev.dailyAvgSat = avgSat;
    dispatch(game, ev);
}

void AchievementManager::onBuySupplies(CarWash &game, const std::string &item, int packs, double cost) {
    totalSuppliesPacks_ += packs;
    AchievementEvent ev;
    ev.type = AchievementEventType::BuySupplies;
    ev.item = item;
    ev.packs = packs;
    ev.cost = cost;
    dispatch(game, ev);
}

void AchievementManager::onBuyUpgrade(CarWash &game, int upgradeId, double cost) {
    totalUpgrades_ += 1;
    AchievementEvent ev;
    ev.type = AchievementEventType::BuyUpgrade;
    ev.upgradeId = upgradeId;
    ev.cost = cost;
    dispatch(game, ev);
}

void AchievementManager::onStructureChanged(CarWash &game) {
    AchievementEvent ev;
    ev.type = AchievementEventType::StructureChanged;
    dispatch(game, ev);
}

void AchievementManager::print(std::ostream &os) const {
    os << "=== ACHIEVEMENTS ===\n";

    std::vector<const Achievement *> v;
    v.reserve(list_.size());
    for (const auto &p: list_) v.push_back(p.get());

    std::sort(v.begin(), v.end(), [](const Achievement *a, const Achievement *b) {
        if (a->unlocked() != b->unlocked()) return a->unlocked();
        if (a->category() != b->category()) return static_cast<int>(a->category()) < static_cast<int>(b->category());
        return a->displayName() < b->displayName();
    });

    for (const auto *a: v) os << *a << "\n";

    os << "Stats: supplies_packs=" << totalSuppliesPacks()
            << " upgrades=" << totalUpgrades()
            << " perfect_days=" << perfectDays_ << "\n";
}
