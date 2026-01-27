#pragma once

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "Inventory.h"
#include "WashBay.h"
#include "WashService.h"
#include "CarQueue.h"
#include "DemandManager.h"
#include "CarWashExceptions.h"
#include "GoalManager.h"
#include "Upgrade.h"
#include "EventManager.h"
#include "ReputationManager.h"
#include "PricingStrategy.h"
#include "DailyReport.h"
#include "AchievementManager.h"
#include "Statistics.h"

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

    [[nodiscard]] bool sameCaseInsensitive(const std::string &a, const std::string &b) const;

    [[nodiscard]] int findService(const std::string &name) const;

    void endCurrentDay();

    void applyPricingStrategy();

    void setPricingMode(const std::string &mode);

    void applyNanoCoatingToWaxServices();

public:
    CarWash(std::string n, Inventory inv, int openM, int closeM);

    [[nodiscard]] double avgSatisfactionToday() const noexcept { return reputation_.avgSatisfaction(); }
    [[nodiscard]] int servedSamplesToday() const noexcept { return reputation_.totalServedSamples(); }

    bool addService(const WashService &s);

    bool addBay(const WashBay &b);

    int bookCars(const std::string &serviceName, int cars);

    void simulateHour();

    void showQueue() const;

    void nextCommand();

    void showServices() const;

    void showBays() const;

    void showStatus() const;

    void showGoals() const;

    void showUpgrades() const;

    void showDashboard() const;

    void showReports() const;

    void showHelp() const;

    void showShop() const;

    void showAchievements() const;

    void showStats() const;

    void buyUpgrade(int id);

    void buySupplies(const std::string &item, int packs = 1);

    void enableNanoCoating();

    void increaseSpeedFactor(double delta) { speedFactor_ += delta; }
    void increaseComfortBonus(double delta) { comfortBonus_ += delta; }
    void increaseBaseDemand(int delta) { baseDemandBonus_ += delta; }

    void adjustCash(double delta) {
        cash_ += delta;
        if (cash_ < 0.0) cash_ = 0.0;
    }

    void adjustServicePrices(double factor);

    [[nodiscard]] double totalCash() const noexcept { return cash_; }
    [[nodiscard]] int totalCarsServed() const noexcept { return totalCarsServed_; }

    [[nodiscard]] double averageSatisfaction() const noexcept {
        return totalSatisfiedCustomers_ > 0
                   ? totalSatisfaction_ / totalSatisfiedCustomers_
                   : 0.0;
    }

    [[nodiscard]] double reputationScore() const noexcept { return reputation_.score(); }
    [[nodiscard]] int currentDemand() const noexcept { return queue_.demand(); }

    [[nodiscard]] int bayCount() const noexcept { return static_cast<int>(bays_.size()); }
    [[nodiscard]] int serviceCount() const noexcept { return static_cast<int>(services_.size()); }
    [[nodiscard]] int upgradesBought() const noexcept { return upgradesBought_; }
    [[nodiscard]] int suppliesPacksBought() const noexcept { return totalSuppliesPacksBought_; }
    [[nodiscard]] int perfectDaysCount() const noexcept { return achievements_.perfectDays(); }

    void logEvent(const std::string &msg) const;

    void run();
};
