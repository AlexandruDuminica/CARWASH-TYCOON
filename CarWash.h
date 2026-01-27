#pragma once

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <limits>

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

    static constexpr int MAX_SERV = 20;
    static constexpr int MAX_BAYS = 20;

    [[nodiscard]] bool sameCaseInsensitive(const std::string& a,const std::string& b) const;
    [[nodiscard]] int  findService(const std::string& name) const;

    void endCurrentDay();
    void applyPricingStrategy();
    void setPricingMode(const std::string& mode);

public:
    CarWash(std::string n, Inventory inv, int openM, int closeM);

    [[nodiscard]] double avgSatisfactionToday() const noexcept { return reputation_.avgSatisfaction(); }
    [[nodiscard]] int servedSamplesToday() const noexcept { return reputation_.totalServedSamples(); }

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
    void showShop() const;
    void showAchievements() const;

    void buyUpgrade(int id);
    void buySupplies(const std::string& item, int packs = 1);

    void increaseSpeedFactor(double delta)   { speedFactor_     += delta; }
    void increaseComfortBonus(double delta)  { comfortBonus_    += delta; }
    void increaseBaseDemand(int delta)       { baseDemandBonus_ += delta; }

    void adjustCash(double delta) { cash_ += delta; if (cash_ < 0.0) cash_ = 0.0; }
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

    void logEvent(const std::string& msg) const;

    void run();
};

inline bool CarWash::addService(const WashService& s) {
    if (services_.size() >= MAX_SERV) return false;
    services_.push_back(s.clone());
    achievements_.onStructureChanged(*this);
    return true;
}

inline bool CarWash::addBay(const WashBay& b) {
    if (bays_.size() >= MAX_BAYS) return false;
    bays_.push_back(std::make_unique<WashBay>(b));
    if (bays_.back()->id() % 2 == 0) {
        bays_.back()->addWax();
    } else {
        bays_.back()->addDeluxe();
    }
    achievements_.onStructureChanged(*this);
    return true;
}

inline void CarWash::run() {
    std::cout << "=== CARWASH TYCOON ===\n";
    showHelp();
    showDashboard();

#ifdef GITHUB_ACTIONS
    try {
        showStatus();
        showServices();
        showBays();
        showGoals();
        showUpgrades();
        showReports();

        adjustCash(400.0);
        showShop();
        buySupplies("water", 1);
        buySupplies("shampoo", 1);
        showShop();

        setPricingMode("balanced");
        nextCommand();

        try {
            buyUpgrade(1);
        } catch (const CarWashException&) {
        }

        showAchievements();
        showDashboard();
    } catch (const CarWashException& ex) {
        std::cout << "Eroare: " << ex.what() << "\n";
    }

    std::cout << "=== FINAL (CI) ===\n";
#else
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        try {
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;

            if (cmd == "help") {
                showHelp();
            } else if (cmd == "status") {
                showStatus();
            } else if (cmd == "services") {
                showServices();
            } else if (cmd == "bays") {
                showBays();
            } else if (cmd == "queue") {
                showQueue();
            } else if (cmd == "next") {
                nextCommand();
            } else if (cmd == "endday") {
                endCurrentDay();
                showDashboard();
            } else if (cmd == "dashboard") {
                showDashboard();
            } else if (cmd == "goals") {
                showGoals();
            } else if (cmd == "upgrades") {
                showUpgrades();
            } else if (cmd == "achievements") {
                showAchievements();
            } else if (cmd == "buyupgrade") {
                int id = 0;
                iss >> id;
                if (id <= 0) {
                    throw InvalidCommandException("Folosire: buyupgrade <id>");
                }
                buyUpgrade(id);
                showDashboard();
            } else if (cmd == "setpricing") {
                std::string mode;
                iss >> mode;
                if (mode.empty()) {
                    throw InvalidCommandException(
                        "Folosire: setpricing <aggressive|balanced|conservative>");
                }
                setPricingMode(mode);
            } else if (cmd == "reports") {
                showReports();
            } else if (cmd == "events") {
                events_.print(std::cout);
            } else if (cmd == "shop") {
                showShop();
            } else if (cmd == "buysupplies") {
                std::string item;
                std::string packsStr;
                iss >> item >> packsStr;
                if (item.empty()) {
                    throw InvalidCommandException(
                        "Folosire: buysupplies <water|shampoo|wax> [packs]");
                }
                int packs = 1;
                if (!packsStr.empty()) {
                    try {
                        size_t pos = 0;
                        packs = std::stoi(packsStr, &pos);
                        if (pos != packsStr.size()) {
                            throw std::invalid_argument("junk");
                        }
                    } catch (const std::exception&) {
                        throw InvalidCommandException(
                            "Folosire: buysupplies <water|shampoo|wax> [packs]");
                    }
                }
                buySupplies(item, packs);
                showDashboard();
            } else if (cmd == "endrun") {
                break;
            } else if (cmd.empty()) {
                continue;
            } else {
                throw InvalidCommandException("Comanda necunoscuta: " + cmd);
            }
        } catch (const CarWashException& ex) {
            std::cout << "Eroare: " << ex.what() << "\n";
        }
    }

    std::cout << "=== FINAL ===\n";
    showDashboard();
#endif
}
