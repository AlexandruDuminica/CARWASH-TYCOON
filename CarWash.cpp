#include "CarWash.h"
#include "BasicService.h"
#include "DeluxeService.h"
#include "WaxService.h"
#include "EcoService.h"
#include "Customer.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

CarWash::CarWash(std::string n, Inventory inv, int openM, int closeM)
    : name_(std::move(n)),
      inv_(inv),
      openMin_(openM),
      closeMin_(closeM),
      nowMin_(openM) {
    if (openM >= closeM) {
        throw CarWashException("Ore program invalide");
    }

    goals_.add(std::make_unique<ProfitGoal>(500.0, "Castiga cel putin 500 EUR"));
    goals_.add(std::make_unique<CarsServedGoal>(50, "Spala cel putin 50 de masini"));
    goals_.add(std::make_unique<RatingGoal>(4.0, "Pastreaza satisfactia medie peste 4.0"));

    pricing_ = std::make_unique<BalancedPricing>();
    currentReport_.beginDay(day_);
}

bool CarWash::sameCaseInsensitive(const std::string &a, const std::string &b) const {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        char ca = static_cast<char>(std::tolower(static_cast<unsigned char>(a[i])));
        char cb = static_cast<char>(std::tolower(static_cast<unsigned char>(b[i])));
        if (ca != cb) return false;
    }
    return true;
}

int CarWash::findService(const std::string &name) const {
    for (size_t i = 0; i < services_.size(); ++i) {
        if (services_[i] && sameCaseInsensitive(services_[i]->name(), name)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool CarWash::addService(const WashService &s) {
    if (services_.size() >= MAX_SERV) return false;
    services_.push_back(s.clone());
    if (nanoCoatingEnabled_) applyNanoCoatingToWaxServices();
    achievements_.onStructureChanged(*this);
    return true;
}

bool CarWash::addBay(const WashBay &b) {
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

void CarWash::applyNanoCoatingToWaxServices() {
    for (auto &s: services_) {
        if (!s) continue;
        if (auto *wax = dynamic_cast<WaxService *>(s.get())) {
            wax->enableNanoCoating(true);
        }
    }
}

void CarWash::enableNanoCoating() {
    if (nanoCoatingEnabled_) return;
    nanoCoatingEnabled_ = true;
    applyNanoCoatingToWaxServices();
    logEvent("Nano Coating enabled for Wax services");
}

int CarWash::bookCars(const std::string &serviceName, int cars) {
    int si = findService(serviceName);
    if (si < 0) {
        throw InvalidServiceException("Serviciul nu exista: " + serviceName);
    }
    if (cars <= 0) {
        throw BookingException("Numar de masini invalid");
    }
    const WashService &sp = *services_.at(static_cast<size_t>(si));

    int booked = 0;
    for (int c = 0; c < cars; ++c) {
        bool ok = false;
        for (auto &bay: bays_) {
            if (!bay->canDo(sp.name())) continue;
            int finish = bay->availAt() + sp.duration();
            if (finish <= closeMin_ && inv_.takeIfCan(sp, 1)) {
                bay->bookOne(sp, bay->availAt());
                cash_ += sp.price();
                ++booked;
                ok = true;
                break;
            }
        }
        if (!ok) break;
    }
    return booked;
}

void CarWash::adjustServicePrices(double factor) {
    if (factor <= 0.0) return;
    for (auto &s: services_) {
        if (s) s->applyFactor(factor);
    }
}

void CarWash::applyPricingStrategy() {
    if (pricing_) pricing_->apply(*this);
}

void CarWash::setPricingMode(const std::string &mode) {
    if (mode == "aggressive") {
        pricing_ = std::make_unique<AggressivePricing>();
    } else if (mode == "conservative") {
        pricing_ = std::make_unique<ConservativePricing>();
    } else {
        pricing_ = std::make_unique<BalancedPricing>();
    }
    logEvent("Schimbare strategie preturi: " + pricing_->name());
}

void CarWash::endCurrentDay() {
    double dailyAvgSat = dailySatisfiedCustomers_ > 0
                             ? dailySatisfactionSum_ / dailySatisfiedCustomers_
                             : 0.0;

    achievements_.onDayEnd(*this, day_, dailyCarsServed_, dailyLost_, dailyRevenue_, dailyAvgSat);

    currentReport_.finalize(dailyCarsServed_, dailyLost_, dailyAvgSat, dailyRevenue_);
    reports_.push_back(currentReport_);

    dailyCarsServed_ = 0;
    dailySatisfactionSum_ = 0.0;
    dailySatisfiedCustomers_ = 0;
    dailyLost_ = 0;
    dailyRevenue_ = 0.0;

    for (auto &b: bays_) b->reset(openMin_);
    nowMin_ = openMin_;

    goals_.checkAll(*this);

    events_.startNewDay(*this);
    applyPricingStrategy();

    ++day_;
    currentReport_.beginDay(day_);

    std::cout << "--- Ziua a fost incheiata. Ziua curenta: " << day_ << " ---\n";
}

void CarWash::simulateHour() {
    nowMin_ += 60;

    int extra = baseDemandBonus_ > 0 ? baseDemandBonus_ : 0;
    for (int i = 0; i < extra; ++i) queue_.increaseDemand();
    queue_.generateRandomCustomers();

    std::vector<WashService *> servicePtrs;
    servicePtrs.reserve(services_.size());
    for (auto &s: services_) if (s) servicePtrs.push_back(s.get());

    int attempts = static_cast<int>(bays_.size()) * static_cast<int>(4 * speedFactor_);
    int processed = 0;

    for (int k = 0; k < attempts; ++k) {
        if (queue_.empty()) break;

        auto customer = queue_.pop();
        if (!customer) break;

        const WashService *chosen = customer->chooseService(servicePtrs);
        if (!chosen) {
            queue_.failOne();
            ++dailyLost_;
            reputation_.onLost();
            achievements_.onLost(*this);
            demand_.fail();
            continue;
        }

        try {
            int got = bookCars(chosen->name(), 1);
            if (got == 1) {
                customer->onServed(*chosen, chosen->price(), 0);
                double sat = customer->satisfaction() + comfortBonus_;
                if (sat > 5.0) sat = 5.0;
                if (sat < 0.0) sat = 0.0;

                totalCarsServed_++;
                totalSatisfaction_ += sat;
                totalSatisfiedCustomers_++;

                dailyCarsServed_++;
                dailySatisfactionSum_ += sat;
                dailySatisfiedCustomers_++;
                dailyRevenue_ += chosen->price();
                currentReport_.addServiceSale(chosen->name(), chosen->price());

                processed++;
                reputation_.onServed(sat);
                achievements_.onServed(*this, 1, sat, chosen->price());
                demand_.success();
            } else {
                queue_.failOne();
                ++dailyLost_;
                reputation_.onLost();
                achievements_.onLost(*this);
                demand_.fail();
            }
        } catch (const CarWashException &) {
            queue_.failOne();
            ++dailyLost_;
            reputation_.onLost();
            achievements_.onLost(*this);
            demand_.fail();
        }
    }

    int adj = demand_.adjust();
    if (adj > 0) queue_.increaseDemand();
    if (adj < 0) queue_.decreaseDemand();
    if (adj != 0) demand_.reset();

    if (nowMin_ >= closeMin_) endCurrentDay();

    std::cout << "Ora simulata: procesate=" << processed
            << " ramase=" << queue_.size()
            << " pierdute=" << queue_.lostCustomers()
            << " cerere/h=" << queue_.demand() << "\n";
}

void CarWash::showQueue() const {
    std::cout << queue_ << "\n";
}

void CarWash::nextCommand() {
    simulateHour();
    showDashboard();
}

void CarWash::showServices() const {
    std::cout << "SERVICII:\n";
    for (const auto& p : services_) {
        if (!p) continue;

        std::string extra;
        if (const auto* wax = dynamic_cast<const WaxService*>(p.get())) {
            extra = wax->nanoCoatingEnabled() ? " nano=ON" : " nano=OFF";
        }

        std::cout << "  - " << *p
                  << " kind=" << WashService::kindToString(p->kind())
                  << (p->isPremium() ? " [premium]" : "")
                  << extra
                  << "\n";
    }
}

void CarWash::showBays() const {
    std::cout << "BAIE (" << bays_.size() << "), create=" << WashBay::totalBaysCreated() << "\n";
    for (const auto &b: bays_) if (b) std::cout << "  " << *b << "\n";
}

void CarWash::showStatus() const {
    std::cout << "=== STATUS ZIUA " << day_ << " ===\n";
    std::cout << "Bani: " << std::fixed << std::setprecision(2) << cash_ << " EUR\n";
    std::cout << "Timp: " << nowMin_ << "/" << closeMin_ << "\n";
    std::cout << "Inventar: " << inv_ << "\n";
    showQueue();
}

void CarWash::showGoals() const {
    goals_.print(std::cout, *this);
    if (goals_.allAchieved()) std::cout << "Toate obiectivele au fost atinse!\n";
}

void CarWash::showUpgrades() const {
    std::cout << "Upgrade-uri cumparate:\n";
    if (purchased_.empty()) {
        std::cout << "  (niciun upgrade)\n";
    } else {
        for (const auto &u: purchased_) if (u) std::cout << "  - " << *u << "\n";
    }

    std::cout << "Upgrade-uri disponibile (id -> descriere):\n";
    std::cout << "  1 -> " << BaySpeedUpgrade() << "\n";
    std::cout << "  2 -> " << ComfortUpgrade() << "\n";
    std::cout << "  3 -> " << MarketingUpgrade() << "\n";
    std::cout << "  4 -> " << NanoCoatingUpgrade() << "\n";
}

void CarWash::showDashboard() const {
    std::cout << "=========== DASHBOARD ==========\n";
    std::cout << "Ziua: " << day_ << "\n";
    std::cout << "Cash: " << std::fixed << std::setprecision(2) << cash_ << " EUR\n";
    std::cout << "Cars served: " << totalCarsServed_ << "\n";
    std::cout << "Avg satisfaction: " << std::setprecision(2) << averageSatisfaction() << "\n";
    std::cout << "Today satisfaction: " << std::setprecision(2) << avgSatisfactionToday()
            << " (n=" << servedSamplesToday() << ")\n";
    std::cout << "Reputation score: " << std::setprecision(2) << reputation_.score() << "\n";
    std::cout << "Queue: " << queue_ << "\n";
    std::cout << "SpeedFactor: " << speedFactor_
            << " | ComfortBonus: " << comfortBonus_ << "\n";
    goals_.print(std::cout, *this);
    if (goals_.allAchieved()) std::cout << "Status obiective: COMPLETATE 100%\n";
    std::cout << "================================\n";
}

void CarWash::showReports() const {
    if (reports_.empty()) {
        std::cout << "Nu exista inca rapoarte zilnice.\n";
        return;
    }
    std::cout << "=== Rapoarte zilnice ===\n";
    for (const auto &r: reports_) std::cout << r << "\n";
}

void CarWash::showShop() const {
    std::cout << "=== SUPPLY SHOP ===\n";
    std::cout << "Ai: " << std::fixed << std::setprecision(2) << cash_ << " EUR\n";
    std::cout << "Inventar curent: " << inv_ << "\n\n";
    std::cout << "Oferte:\n";
    std::cout << "  water   : 20 EUR / pack -> +200 water\n";
    std::cout << "  shampoo : 25 EUR / pack -> +50 shampoo\n";
    std::cout << "  wax     : 30 EUR / pack -> +25 wax\n";
    std::cout << "Cumperi cu: buysupplies <water|shampoo|wax> [packs]\n";
}

void CarWash::buySupplies(const std::string &item, int packs) {
    if (packs <= 0) throw InvalidCommandException("Folosire: buysupplies <water|shampoo|wax> [packs]");

    constexpr int kWaterPackQty = 200;
    constexpr int kShampooPackQty = 50;
    constexpr int kWaxPackQty = 25;

    constexpr double kWaterPackCost = 20.0;
    constexpr double kShampooPackCost = 25.0;
    constexpr double kWaxPackCost = 30.0;

    if (sameCaseInsensitive(item, "water")) {
        const double totalCost = kWaterPackCost * packs;
        if (cash_ < totalCost) throw CarWashException("Nu ai suficienti bani pentru water");
        cash_ -= totalCost;
        inv_.addWater(kWaterPackQty * packs);
        totalSuppliesPacksBought_ += packs;
        achievements_.onBuySupplies(*this, "water", packs, totalCost);
        logEvent("Cumparare supplies: water x" + std::to_string(packs));
    } else if (sameCaseInsensitive(item, "shampoo")) {
        const double totalCost = kShampooPackCost * packs;
        if (cash_ < totalCost) throw CarWashException("Nu ai suficienti bani pentru shampoo");
        cash_ -= totalCost;
        inv_.addShampoo(kShampooPackQty * packs);
        totalSuppliesPacksBought_ += packs;
        achievements_.onBuySupplies(*this, "shampoo", packs, totalCost);
        logEvent("Cumparare supplies: shampoo x" + std::to_string(packs));
    } else if (sameCaseInsensitive(item, "wax")) {
        const double totalCost = kWaxPackCost * packs;
        if (cash_ < totalCost) throw CarWashException("Nu ai suficienti bani pentru wax");
        cash_ -= totalCost;
        inv_.addWax(kWaxPackQty * packs);
        totalSuppliesPacksBought_ += packs;
        achievements_.onBuySupplies(*this, "wax", packs, totalCost);
        logEvent("Cumparare supplies: wax x" + std::to_string(packs));
    } else {
        throw InvalidCommandException("Resursa necunoscuta. Folosire: buysupplies <water|shampoo|wax> [packs]");
    }
}

void CarWash::showAchievements() const {
    achievements_.print(std::cout);
}

void CarWash::showStats() const {
    Statistics stats(reports_);
    stats.print(std::cout);
}

void CarWash::showHelp() const {
    std::cout
            << "Comenzi:\n"
            << "  help           - afiseaza acest mesaj\n"
            << "  status         - stare detaliata\n"
            << "  services       - lista servicii\n"
            << "  bays           - lista bai\n"
            << "  queue          - info coada\n"
            << "  next           - simuleaza o ora\n"
            << "  endday         - incheie manual ziua curenta\n"
            << "  dashboard      - afiseaza rezumat tycoon\n"
            << "  goals          - afiseaza obiective si progres\n"
            << "  upgrades       - lista upgrade-uri\n"
            << "  buyupgrade X   - cumpara upgrade (1..4)\n"
            << "  setpricing M   - seteaza strategia de preturi (aggressive|balanced|conservative)\n"
            << "  reports        - afiseaza rapoarte zilnice\n"
            << "  events         - afiseaza evenimentele zilei curente\n"
            << "  shop           - afiseaza oferta de supplies\n"
            << "  buysupplies R [packs] - cumpara supplies (water/shampoo/wax)\n"
            << "  achievements   - lista achievements\n"
            << "  stats          - analytics detaliat\n"
            << "  endrun         - termina simularea\n";
}

void CarWash::buyUpgrade(int id) {
    std::unique_ptr<Upgrade> u;
    if (id == 1) u = std::make_unique<BaySpeedUpgrade>();
    else if (id == 2) u = std::make_unique<ComfortUpgrade>();
    else if (id == 3) u = std::make_unique<MarketingUpgrade>();
    else if (id == 4) u = std::make_unique<NanoCoatingUpgrade>();
    else throw InvalidCommandException("Upgrade necunoscut");

    if (u->cost() > cash_) throw CarWashException("Nu ai suficienti bani pentru upgrade");

    const double cost = u->cost();
    cash_ -= cost;
    u->apply(*this);
    purchased_.push_back(std::move(u));
    upgradesBought_ += 1;
    achievements_.onBuyUpgrade(*this, id, cost);
}

void CarWash::logEvent(const std::string &msg) const {
    std::cout << "[LOG] " << msg << "\n";
}

void CarWash::run() {
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

        adjustCash(600.0);
        showShop();
        buySupplies("water", 2);
        buySupplies("shampoo", 2);
        buySupplies("wax", 1);
        showShop();

        buyUpgrade(4);
        showServices();

        setPricingMode("balanced");
        nextCommand();

        showReports();
        showAchievements();
        showStats();
        showDashboard();
    } catch (const CarWashException &ex) {
        std::cout << "Eroare: " << ex.what() << "\n";
    }
    std::cout << "=== FINAL (CI) ===\n";
#else
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        try {
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;

            if (cmd == "help") showHelp();
            else if (cmd == "status") showStatus();
            else if (cmd == "services") showServices();
            else if (cmd == "bays") showBays();
            else if (cmd == "queue") showQueue();
            else if (cmd == "next") nextCommand();
            else if (cmd == "endday") {
                endCurrentDay();
                showDashboard();
            } else if (cmd == "dashboard") showDashboard();
            else if (cmd == "goals") showGoals();
            else if (cmd == "upgrades") showUpgrades();
            else if (cmd == "achievements") showAchievements();
            else if (cmd == "stats") showStats();
            else if (cmd == "buyupgrade") {
                int uid = 0;
                iss >> uid;
                if (uid <= 0) throw InvalidCommandException("Folosire: buyupgrade <id>");
                buyUpgrade(uid);
                showDashboard();
            } else if (cmd == "setpricing") {
                std::string mode;
                iss >> mode;
                if (mode.empty()) throw InvalidCommandException(
                    "Folosire: setpricing <aggressive|balanced|conservative>");
                setPricingMode(mode);
            } else if (cmd == "reports") showReports();
            else if (cmd == "events") events_.print(std::cout);
            else if (cmd == "shop") showShop();
            else if (cmd == "buysupplies") {
                std::string item, packsStr;
                iss >> item >> packsStr;
                if (item.empty()) throw InvalidCommandException("Folosire: buysupplies <water|shampoo|wax> [packs]");
                int packs = 1;
                if (!packsStr.empty()) {
                    size_t pos = 0;
                    packs = std::stoi(packsStr, &pos);
                    if (pos != packsStr.size()) throw InvalidCommandException(
                        "Folosire: buysupplies <water|shampoo|wax> [packs]");
                }
                buySupplies(item, packs);
                showDashboard();
            } else if (cmd == "endrun") break;
            else if (cmd.empty()) continue;
            else throw InvalidCommandException("Comanda necunoscuta: " + cmd);
        } catch (const CarWashException &ex) {
            std::cout << "Eroare: " << ex.what() << "\n";
        }
    }
    std::cout << "=== FINAL ===\n";
    showDashboard();
#endif
}
