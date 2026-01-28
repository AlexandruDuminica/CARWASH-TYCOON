#pragma once

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "../headers//Inventory.h"
#include "WashBay.h"
#include "WashService.h"
#include "CarQueue.h"
#include "DemandManager.h"
#include "../headers//CarWashExceptions.h"
#include "GoalManager.h"
#include "../headers//Upgrade.h"
#include "EventManager.h"
#include "ReputationManager.h"
#include "PricingStrategy.h"
#include "DailyReport.h"
#include "AchievementManager.h"
#include "Statistics.h"

/**
 * @brief Core simulation engine for the CarWash Tycoon application.
 *
 * CarWash owns and coordinates the main gameplay subsystems:
 * - Service catalog (WashService polymorphic hierarchy)
 * - Wash bays (WashBay pool)
 * - Inventory (water/shampoo/wax consumption and restocking)
 * - Customer queue and demand modulation (CarQueue, DemandManager)
 * - Goals, events, reputation, pricing strategies
 * - Upgrades (including Nano Coating for wax services via RTTI)
 * - Daily reports, achievements and statistics
 *
 * The class provides both an interactive command loop and a CI/demo path
 * when GITHUB_ACTIONS is defined.
 */
class CarWash {
    std::string name_;
    Inventory inv_;
    std::vector<std::unique_ptr<WashService> > services_;
    std::vector<std::unique_ptr<WashBay> > bays_;

    double cash_{0.0};
    int openMin_;
    int closeMin_;
    int nowMin_;
    int day_{1};

    CarQueue queue_;
    DemandManager demand_;
    GoalManager goals_;
    std::vector<std::unique_ptr<Upgrade> > purchased_;
    EventManager events_;
    ReputationManager reputation_;
    std::unique_ptr<PricingStrategy> pricing_;
    AchievementManager achievements_;

    int totalCarsServed_{0};
    double totalSatisfaction_{0.0};
    int totalSatisfiedCustomers_{0};

    int dailyCarsServed_{0};
    double dailySatisfactionSum_{0.0};
    int dailySatisfiedCustomers_{0};
    int dailyLost_{0};
    double dailyRevenue_{0.0};

    DailyReport currentReport_;
    std::vector<DailyReport> reports_;

    double speedFactor_{1.0};
    double comfortBonus_{0.0};
    int baseDemandBonus_{0};

    int totalSuppliesPacksBought_{0};
    int upgradesBought_{0};

    bool nanoCoatingEnabled_{false};

    static constexpr int MAX_SERV = 20;
    static constexpr int MAX_BAYS = 20;

    /**
     * @brief Case-insensitive string equality.
     *
     * @param a Left operand.
     * @param b Right operand.
     * @return true if equal ignoring case; false otherwise.
     */
    [[nodiscard]] bool sameCaseInsensitive(const std::string &a, const std::string &b) const;

    /**
     * @brief Finds a service index by name (case-insensitive).
     *
     * @param name Service name to search for.
     * @return Index in services_ if found; -1 otherwise.
     */
    [[nodiscard]] int findService(const std::string &name) const;

    /**
     * @brief Finalizes the current day and rolls over to the next day.
     *
     * Computes daily aggregates, triggers achievement/goal/event hooks,
     * stores the DailyReport, resets per-day counters, and resets time/bays.
     */
    void endCurrentDay();

    /**
     * @brief Applies the current pricing strategy to the service catalog.
     */
    void applyPricingStrategy();

    /**
     * @brief Switches the active pricing strategy.
     *
     * @param mode Strategy key (e.g., "aggressive", "balanced", "conservative").
     */
    void setPricingMode(const std::string &mode);

    /**
     * @brief Applies Nano Coating to all WaxService instances using RTTI.
     *
     * Nano coating is a Wax-only capability; services are stored polymorphically,
     * so WaxService is detected via dynamic_cast and updated accordingly.
     */
    void applyNanoCoatingToWaxServices();

public:
    /**
     * @brief Constructs a CarWash simulation instance.
     *
     * @param n Car wash display name.
     * @param inv Initial inventory snapshot.
     * @param openM Opening time in minutes since midnight.
     * @param closeM Closing time in minutes since midnight.
     *
     * @throws CarWashException If opening time is not strictly before closing time.
     */
    CarWash(std::string n, Inventory inv, int openM, int closeM);

    /**
     * @brief Returns average satisfaction sample for the current day.
     *
     * This is backed by the ReputationManager rolling statistics.
     *
     * @return Average satisfaction in [0, 5] based on served samples.
     */
    [[nodiscard]] double avgSatisfactionToday() const noexcept { return reputation_.avgSatisfaction(); }

    /**
     * @brief Returns number of satisfaction samples collected today.
     *
     * @return Total served samples tracked by ReputationManager.
     */
    [[nodiscard]] int servedSamplesToday() const noexcept { return reputation_.totalServedSamples(); }

    /**
     * @brief Adds a wash service to the catalog (stores a polymorphic clone).
     *
     * @param s Service prototype to clone and store.
     * @return true if added; false if service limit reached.
     */
    bool addService(const WashService &s);

    /**
     * @brief Adds a wash bay to the simulation (stores a copy).
     *
     * @param b Bay prototype to copy and store.
     * @return true if added; false if bay limit reached.
     */
    bool addBay(const WashBay &b);

    /**
     * @brief Attempts to book multiple cars for a given service.
     *
     * Each car is scheduled on the first suitable bay that can execute the service,
     * finishes before close time, and has sufficient inventory.
     *
     * @param serviceName Service name.
     * @param cars Number of cars to book.
     * @return Number of cars actually booked.
     *
     * @throws InvalidServiceException If serviceName is not found.
     * @throws BookingException If cars <= 0.
     */
    int bookCars(const std::string &serviceName, int cars);

    /**
     * @brief Simulates one hour of gameplay.
     *
     * Advances time, generates customers, attempts to serve them based on bay capacity,
     * updates cash/satisfaction/reputation/achievements, adjusts demand, and ends the
     * day when closing time is reached.
     */
    void simulateHour();

    /**
     * @brief Prints the current customer queue.
     */
    void showQueue() const;

    /**
     * @brief Executes a single simulation step and prints the dashboard.
     */
    void nextCommand();

    /**
     * @brief Prints the available services (including premium flags).
     *
     * Wax services may also display Nano Coating status.
     */
    void showServices() const;

    /**
     * @brief Prints all wash bays and their state.
     */
    void showBays() const;

    /**
     * @brief Prints detailed status: cash, time, inventory, and queue.
     */
    void showStatus() const;

    /**
     * @brief Prints all goals and current progress.
     */
    void showGoals() const;

    /**
     * @brief Prints purchased upgrades and the upgrade catalog.
     */
    void showUpgrades() const;

    /**
     * @brief Prints a compact dashboard with core KPIs.
     */
    void showDashboard() const;

    /**
     * @brief Prints the stored daily reports.
     */
    void showReports() const;

    /**
     * @brief Prints the help/command menu.
     */
    void showHelp() const;

    /**
     * @brief Prints shop offers and current inventory/cash.
     */
    void showShop() const;

    /**
     * @brief Prints the achievements list/status.
     */
    void showAchievements() const;

    /**
     * @brief Prints advanced statistics computed from daily reports.
     */
    void showStats() const;

    /**
     * @brief Buys and applies an upgrade by id.
     *
     * @param id Upgrade id.
     *
     * @throws InvalidCommandException If id is not supported.
     * @throws CarWashException If insufficient funds.
     */
    void buyUpgrade(int id);

    /**
     * @brief Buys supply packs and updates inventory and achievements.
     *
     * @param item Supply key ("water", "shampoo", "wax").
     * @param packs Number of packs to purchase (default 1).
     *
     * @throws InvalidCommandException If packs <= 0 or item invalid.
     * @throws CarWashException If insufficient funds.
     */
    void buySupplies(const std::string &item, int packs = 1);

    /**
     * @brief Enables Nano Coating upgrade globally and applies it to existing wax services.
     */
    void enableNanoCoating();

    /**
     * @brief Increases service throughput multiplier.
     *
     * @param delta Amount to add to the speed factor.
     */
    void increaseSpeedFactor(double delta) { speedFactor_ += delta; }

    /**
     * @brief Increases satisfaction bonus applied to served customers.
     *
     * @param delta Amount to add to the comfort bonus.
     */
    void increaseComfortBonus(double delta) { comfortBonus_ += delta; }

    /**
     * @brief Increases base demand bonus applied per simulated hour.
     *
     * @param delta Demand increment.
     */
    void increaseBaseDemand(int delta) { baseDemandBonus_ += delta; }

    /**
     * @brief Adjusts cash balance by a delta value.
     *
     * Cash is clamped to a minimum of 0.0.
     *
     * @param delta Signed cash delta.
     */
    void adjustCash(double delta) {
        cash_ += delta;
        if (cash_ < 0.0) cash_ = 0.0;
    }

    /**
     * @brief Applies a multiplicative factor to all service prices.
     *
     * @param factor Must be > 0.0; otherwise ignored.
     */
    void adjustServicePrices(double factor);

    /**
     * @brief Returns current cash balance.
     */
    [[nodiscard]] double totalCash() const noexcept { return cash_; }

    /**
     * @brief Returns total number of cars served across all days.
     */
    [[nodiscard]] int totalCarsServed() const noexcept { return totalCarsServed_; }

    /**
     * @brief Returns lifetime average satisfaction across served customers.
     *
     * @return Average satisfaction or 0.0 if no satisfied customers were tracked.
     */
    [[nodiscard]] double averageSatisfaction() const noexcept {
        return totalSatisfiedCustomers_ > 0
                   ? totalSatisfaction_ / totalSatisfiedCustomers_
                   : 0.0;
    }

    /**
     * @brief Returns the current reputation score.
     */
    [[nodiscard]] double reputationScore() const noexcept { return reputation_.score(); }

    /**
     * @brief Returns the current demand level from the queue.
     */
    [[nodiscard]] int currentDemand() const noexcept { return queue_.demand(); }

    /**
     * @brief Returns number of bays currently owned.
     */
    [[nodiscard]] int bayCount() const noexcept { return static_cast<int>(bays_.size()); }

    /**
     * @brief Returns number of services currently available.
     */
    [[nodiscard]] int serviceCount() const noexcept { return static_cast<int>(services_.size()); }

    /**
     * @brief Returns number of upgrades purchased.
     */
    [[nodiscard]] int upgradesBought() const noexcept { return upgradesBought_; }

    /**
     * @brief Returns number of supply packs bought.
     */
    [[nodiscard]] int suppliesPacksBought() const noexcept { return totalSuppliesPacksBought_; }

    /**
     * @brief Returns the number of unlocked achievements (project-defined metric).
     */
    [[nodiscard]] int perfectDaysCount() const noexcept { return achievements_.perfectDays(); }

    /**
     * @brief Logs a message to the output stream (console).
     *
     * @param msg Message text.
     */
    void logEvent(const std::string &msg) const;

    /**
     * @brief Runs the simulation.
     *
     * In CI builds, runs a deterministic demo sequence (when GITHUB_ACTIONS is defined).
     * Otherwise, starts an interactive command loop reading from stdin.
     */
    void run();
};
