#pragma once

#include <memory>
#include <string>
#include <vector>

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

class CarWash {
    std::string name_;
    Inventory   inv_;
    std::vector<std::unique_ptr<WashService>> services_;
    std::vector<std::unique_ptr<WashBay>>     bays_;

    double cash_{0.0};
    int openMin_;
    int closeMin_;
    int nowMin_;
    int day_{1};

    CarQueue queue_;
    DemandManager demand_;
    GoalManager goals_;
    std::vector<std::unique_ptr<Upgrade>> purchased_;
    EventManager events_;
    ReputationManager reputation_;
    std::unique_ptr<PricingStrategy> pricing_;

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

    static constexpr int MAX_SERV = 20;
    static constexpr int MAX_BAYS = 20;

    bool sameCaseInsensitive(const std::string& a,const std::string& b) const;
    int  findService(const std::string& name) const;

    void endCurrentDay();
    void applyPricingStrategy();
    void setPricingMode(const std::string& mode);

public:
    CarWash(std::string n, Inventory inv, int openM, int closeM);

    bool addService(const WashService& s);
    bool addBay(const WashBay& b);

    int  bookCars(const std::string& serviceName, int cars);

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

    void buyUpgrade(int id);

    // hooks pentru Upgrade-uri si Event-uri
    void increaseSpeedFactor(double delta)   { speedFactor_     += delta; }
    void increaseComfortBonus(double delta)  { comfortBonus_    += delta; }
    void increaseBaseDemand(int delta)       { baseDemandBonus_ += delta; }

    // pentru Event / Pricing / Reputation / Goals
    void adjustCash(double delta) { cash_ += delta; if (cash_ < 0.0) cash_ = 0.0; }
    void adjustServicePrices(double factor);

    double totalCash() const noexcept { return cash_; }
    int totalCarsServed() const noexcept { return totalCarsServed_; }
    double averageSatisfaction() const noexcept {
        return totalSatisfiedCustomers_ > 0
               ? totalSatisfaction_ / totalSatisfiedCustomers_
               : 0.0;
    }

    double reputationScore() const noexcept { return reputation_.score(); }
    int currentDemand() const noexcept { return queue_.demand(); }

    void logEvent(const std::string& msg) const;

    void run();
};