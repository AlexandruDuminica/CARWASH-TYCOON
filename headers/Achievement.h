#pragma once

#include <ostream>
#include <string>

class CarWash;

/**
 * @brief Defines how rare an achievement is, typically used for UI/ordering.
 */
enum class AchievementRarity { Common, Rare, Epic, Legendary };

/**
 * @brief Logical grouping for achievements (used for filtering and reporting).
 */
enum class AchievementCategory { Operations, Finance, Reputation, Management, Hidden };

/**
 * @brief Reward payload granted when an achievement is unlocked.
 *
 * Rewards are applied to the game state (CarWash) when an achievement unlocks.
 */
struct AchievementReward {
    double cash{0.0}; /**< Cash reward in EUR. */
    int demand{0}; /**< Demand boost (implementation-defined impact). */
    double speed{0.0}; /**< Speed factor increment (throughput). */
    double comfort{0.0}; /**< Comfort bonus increment (satisfaction). */
};

/**
 * @brief Event types that can be emitted by gameplay and observed by achievements.
 */
enum class AchievementEventType {
    Served, /**< A customer/car was served. */
    Lost, /**< A customer was lost (could not be served). */
    DayEnd, /**< A day ended; daily aggregates are available. */
    BuySupplies, /**< Supplies were purchased from the shop. */
    BuyUpgrade, /**< An upgrade was purchased. */
    StructureChanged /**< Services/bays structure changed. */
};

/**
 * @brief Unified event payload provided to achievements.
 *
 * The achievement system uses a single event struct to avoid coupling each achievement
 * to a large number of different callback signatures. Fields are populated depending
 * on the AchievementEventType.
 */
struct AchievementEvent {
    AchievementEventType type{AchievementEventType::Served};

    int cars{0}; /**< Served cars for Served events. */
    double satisfaction{0.0}; /**< Satisfaction for Served events. */
    double revenue{0.0}; /**< Revenue for Served events. */

    int day{0}; /**< Current day index for DayEnd events. */
    int dailyServed{0}; /**< Total cars served during the day for DayEnd events. */
    int dailyLost{0}; /**< Total lost customers during the day for DayEnd events. */
    double dailyRevenue{0.0}; /**< Total revenue during the day for DayEnd events. */
    double dailyAvgSat{0.0}; /**< Average satisfaction during the day for DayEnd events. */

    std::string item{}; /**< Item identifier for BuySupplies events. */
    int packs{0}; /**< Packs purchased for BuySupplies events. */
    double cost{0.0}; /**< Total cost for BuySupplies/BuyUpgrade events. */

    int upgradeId{0}; /**< Upgrade id for BuyUpgrade events. */
};

/**
 * @brief Abstract base class for a single achievement.
 *
 * Achievements are stateful observers of gameplay events. Each achievement defines
 * its own logic inside onEvent(), updates progress, and unlocks itself when reaching
 * the target threshold. Unlocking can apply a reward to the CarWash game state.
 */
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
    /**
     * @brief Constructs an achievement with metadata, target, and reward.
     *
     * @param id Stable identifier used for saving / indexing.
     * @param name Display name shown in UI (may be hidden if hidden=true and locked).
     * @param desc Description shown in UI (may be hidden if hidden=true and locked).
     * @param cat Achievement category.
     * @param rar Achievement rarity.
     * @param target Progress threshold required to unlock.
     * @param reward Reward payload applied when unlocking.
     * @param hidden If true, name/description are obscured until unlocked.
     */
    Achievement(std::string id, std::string name, std::string desc,
                AchievementCategory cat, AchievementRarity rar,
                int target, const AchievementReward &reward, bool hidden = false)
        : id_(std::move(id)),
          name_(std::move(name)),
          desc_(std::move(desc)),
          category_(cat),
          rarity_(rar),
          hidden_(hidden),
          target_(target),
          reward_(reward) {
    }

    /**
     * @brief Virtual destructor for polymorphic deletion.
     */
    virtual ~Achievement() = default;

    /**
     * @brief Returns the stable identifier of the achievement.
     */
    const std::string &id() const noexcept { return id_; }

    /**
     * @brief Returns whether the achievement has been unlocked.
     */
    bool unlocked() const noexcept { return unlocked_; }

    /**
     * @brief Returns whether the achievement is hidden until unlocked.
     */
    bool hidden() const noexcept { return hidden_; }

    /**
     * @brief Returns current progress value.
     */
    int progress() const noexcept { return progress_; }

    /**
     * @brief Returns target progress required to unlock.
     */
    int target() const noexcept { return target_; }

    /**
     * @brief Returns achievement category.
     */
    AchievementCategory category() const noexcept { return category_; }

    /**
     * @brief Returns achievement rarity.
     */
    AchievementRarity rarity() const noexcept { return rarity_; }

    /**
     * @brief Returns a UI-safe name (masked if hidden and not unlocked).
     */
    std::string displayName() const {
        if (hidden_ && !unlocked_) return "???";
        return name_;
    }

    /**
     * @brief Returns a UI-safe description (masked if hidden and not unlocked).
     */
    std::string displayDesc() const {
        if (hidden_ && !unlocked_) return "Hidden achievement";
        return desc_;
    }

    /**
     * @brief Event handler invoked by the achievement manager.
     *
     * Derived classes implement their condition/progress logic here.
     *
     * @param game Mutable game state, enabling reward application on unlock.
     * @param ev Event payload describing what just happened.
     */
    virtual void onEvent(CarWash &game, const AchievementEvent &ev) = 0;

    /**
     * @brief Marks the achievement as unlocked and applies reward (once).
     *
     * If the achievement is already unlocked, this function has no effect.
     *
     * @param game Game state to which the reward is applied.
     */
    void unlock(CarWash &game);

    /**
     * @brief Prints achievement details for UI/console output.
     *
     * Output respects hidden status via displayName()/displayDesc().
     *
     * @param os Output stream.
     */
    void print(std::ostream &os) const;
};

/**
 * @brief Stream output helper for achievements.
 *
 * @param os Output stream.
 * @param a Achievement to print.
 * @return Reference to the output stream.
 */
std::ostream &operator<<(std::ostream &os, const Achievement &a);
