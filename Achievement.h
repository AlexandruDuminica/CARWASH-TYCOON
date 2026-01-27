#pragma once

#include <string>
#include <ostream>

class CarWash;

enum class AchievementRarity { Common, Rare, Epic, Legendary };
enum class AchievementCategory { Operations, Finance, Reputation, Management, Hidden };

struct AchievementReward {
    double cash{0.0};
    int demand{0};
    double speed{0.0};
    double comfort{0.0};
};

enum class AchievementEventType {
    Served,
    Lost,
    DayEnd,
    BuySupplies,
    BuyUpgrade,
    StructureChanged
};

struct AchievementEvent {
    AchievementEventType type{AchievementEventType::Served};

    int cars{0};
    double satisfaction{0.0};
    double revenue{0.0};

    int day{0};
    int dailyServed{0};
    int dailyLost{0};
    double dailyRevenue{0.0};
    double dailyAvgSat{0.0};

    std::string item{};
    int packs{0};
    double cost{0.0};

    int upgradeId{0};
};

class Achievement {
protected:
    std::string id_;
    std::string name_;
    std::string desc_;
    AchievementCategory category_{AchievementCategory::Operations};
    AchievementRarity rarity_{AchievementRarity::Common};
    bool hidden_{false};
    bool unlocked_{false};

    int progress_{0};
    int target_{1};
    AchievementReward reward_{};

public:
    Achievement(std::string id, std::string name, std::string desc,
                AchievementCategory cat, AchievementRarity rar,
                int target, const AchievementReward& reward, bool hidden = false)
        : id_(std::move(id)),
          name_(std::move(name)),
          desc_(std::move(desc)),
          category_(cat),
          rarity_(rar),
          hidden_(hidden),
          target_(target),
          reward_(reward) {}

    virtual ~Achievement() = default;

    const std::string& id() const noexcept { return id_; }
    bool unlocked() const noexcept { return unlocked_; }
    bool hidden() const noexcept { return hidden_; }
    int progress() const noexcept { return progress_; }
    int target() const noexcept { return target_; }

    AchievementCategory category() const noexcept { return category_; }
    AchievementRarity rarity() const noexcept { return rarity_; }

    std::string displayName() const {
        if (hidden_ && !unlocked_) return "???";
        return name_;
    }

    std::string displayDesc() const {
        if (hidden_ && !unlocked_) return "Hidden achievement";
        return desc_;
    }

    virtual void onEvent(CarWash& game, const AchievementEvent& ev) = 0;

    void unlock(CarWash& game);

    void print(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const Achievement& a);
