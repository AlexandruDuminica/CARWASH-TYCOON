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

    int totalCarsServed_{0};
    double totalSatisfaction_{0.0};
    int totalSatisfiedCustomers_{0};

    double speedFactor_{1.0};
    double comfortBonus_{0.0};
    int baseDemandBonus_{0};

    static constexpr int MAX_SERV = 20;
    static constexpr int MAX_BAYS = 20;

    bool sameCaseInsensitive(const std::string& a,const std::string& b) const;
    int  findService(const std::string& name) const;

    // finalizează o zi (fie la sfârșit de program, fie manual)
    void endCurrentDay();

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
    void showHelp() const;

    void buyUpgrade(int id);

    // hooks pentru Upgrade-uri
    void increaseSpeedFactor(double delta)   { speedFactor_     += delta; }
    void increaseComfortBonus(double delta)  { comfortBonus_    += delta; }
    void increaseBaseDemand(int delta)       { baseDemandBonus_ += delta; }

    // accesori pentru Goals
    double totalCash() const noexcept { return cash_; }
    int totalCarsServed() const noexcept { return totalCarsServed_; }
    double averageSatisfaction() const noexcept {
        return totalSatisfiedCustomers_ > 0
               ? totalSatisfaction_ / totalSatisfiedCustomers_
               : 0.0;
    }

    void run();
};
