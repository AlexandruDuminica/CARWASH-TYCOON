#include "CarWash.h"
#include "BasicService.h"
#include "DeluxeService.h"
#include "WaxService.h"
#include "EcoService.h"
#include "Customer.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
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

    goals_.add(std::make_unique<ProfitGoal>(
        500.0, "Castiga cel putin 500 EUR"));
    goals_.add(std::make_unique<CarsServedGoal>(
        50, "Spala cel putin 50 de masini"));
    goals_.add(std::make_unique<RatingGoal>(
        4.0, "Pastreaza satisfactia medie peste 4.0"));

    pricing_ = std::make_unique<BalancedPricing>();

    currentReport_.beginDay(day_);
}

bool CarWash::sameCaseInsensitive(const std::string& a,
                                  const std::string& b) const {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        char ca = static_cast<char>(std::tolower(static_cast<unsigned char>(a[i])));
        char cb = static_cast<char>(std::tolower(static_cast<unsigned char>(b[i])));
        if (ca != cb) return false;
    }
    return true;
}

int CarWash::findService(const std::string& name) const {
    for (size_t i = 0; i < services_.size(); ++i) {
        if (services_[i] && sameCaseInsensitive(services_[i]->name(), name)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool CarWash::addService(const WashService& s) {
    if (services_.size() >= MAX_SERV) return false;
    services_.push_back(s.clone());
    return true;
}

bool CarWash::addBay(const WashBay& b) {
    if (bays_.size() >= MAX_BAYS) return false;
    bays_.push_back(std::make_unique<WashBay>(b));

    if (bays_.back()->id() % 2 == 0) {
        bays_.back()->addWax();
    } else {
        bays_.back()->addDeluxe();
    }

    return true;
}

int CarWash::bookCars(const std::string& serviceName, int cars) {
    int si = findService(serviceName);
    if (si < 0) {
        throw InvalidServiceException("Serviciul nu exista: " + serviceName);
    }
    if (cars <= 0) {
        throw BookingException("Numar de masini invalid");
    }
    const WashService& sp = *services_.at(static_cast<size_t>(si));

    int booked = 0;
    for (int c = 0; c < cars; ++c) {
        bool ok = false;
        for (auto& bay : bays_) {
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
    for (auto& s : services_) {
        if (s) s->applyFactor(factor);
    }
}

void CarWash::applyPricingStrategy() {
    if (pricing_) {
        pricing_->apply(*this);
    }
}

void CarWash::setPricingMode(const std::string& mode) {
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

    currentReport_.finalize(dailyCarsServed_, dailyLost_,
                            dailyAvgSat, dailyRevenue_);
    reports_.push_back(currentReport_);

    dailyCarsServed_ = 0;
    dailySatisfactionSum_ = 0.0;
    dailySatisfiedCustomers_ = 0;
    dailyLost_ = 0;
    dailyRevenue_ = 0.0;

    for (auto& b : bays_) {
        b->reset(openMin_);
    }
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
    for (int i = 0; i < extra; ++i) {
        queue_.increaseDemand();
    }
    queue_.generateRandomCustomers();

    std::vector<WashService*> servicePtrs;
    servicePtrs.reserve(services_.size());
    for (auto& s : services_)
        if (s) servicePtrs.push_back(s.get());

    int attempts = static_cast<int>(bays_.size()) * 4 * static_cast<int>(speedFactor_);
    int processed = 0;

    for (int k = 0; k < attempts; ++k) {
        if (queue_.empty()) break;

        auto customer = queue_.pop();
        if (!customer) break;

        double b = customer->budget();
        double imp = customer->impatience();
        (void)b;
        (void)imp;

        const WashService* chosen = customer->chooseService(servicePtrs);
        if (!chosen) {
            queue_.failOne();
            ++dailyLost_;
            reputation_.onLost();
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
                demand_.success();
            } else {
                queue_.failOne();
                ++dailyLost_;
                reputation_.onLost();
                demand_.fail();
            }
        } catch (const CarWashException&) {
            queue_.failOne();
            ++dailyLost_;
            reputation_.onLost();
            demand_.fail();
        }
    }

    int adj = demand_.adjust();
    if (adj > 0) queue_.increaseDemand();
    if (adj < 0) queue_.decreaseDemand();
    if (adj != 0) demand_.reset();

    if (nowMin_ >= closeMin_) {
        endCurrentDay();
    }

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
        std::cout << "  - " << *p;
        if (dynamic_cast<const DeluxeService*>(p.get())) {
            std::cout << " [premium]";
        }
        std::cout << "\n";
    }
}

void CarWash::showBays() const {
    std::cout << "BAIE (" << bays_.size() << "), create="
              << WashBay::totalBaysCreated() << "\n";
    for (const auto& b : bays_) {
        if (b) std::cout << "  " << *b << "\n";
    }
}

void CarWash::showStatus() const {
    std::cout << "=== STATUS ZIUA " << day_ << " ===\n";
    std::cout << "Bani: " << std::fixed << std::setprecision(2)
              << cash_ << " EUR\n";
    std::cout << "Timp: " << nowMin_ << "/" << closeMin_ << "\n";
    std::cout << "Inventar: " << inv_ << "\n";
    showQueue();
}

void CarWash::showGoals() const {
    goals_.print(std::cout, *this);
    if (goals_.allAchieved()) {
        std::cout << "Toate obiectivele au fost atinse!\n";
    }
}

void CarWash::showUpgrades() const {
    std::cout << "Upgrade-uri cumparate:\n";
    if (purchased_.empty()) {
        std::cout << "  (niciun upgrade)\n";
    } else {
        for (const auto& u : purchased_) {
            if (u) std::cout << "  - " << *u << "\n";
        }
    }

    std::cout << "Upgrade-uri disponibile (id -> descriere):\n";
    std::cout << "  1 -> " << BaySpeedUpgrade() << "\n";
    std::cout << "  2 -> " << ComfortUpgrade() << "\n";
    std::cout << "  3 -> " << MarketingUpgrade() << "\n";
}

void CarWash::showDashboard() const {
    std::cout << "=========== DASHBOARD ==========\n";
    std::cout << "Ziua: " << day_ << "\n";
    std::cout << "Cash: " << std::fixed << std::setprecision(2) << cash_ << " EUR\n";
    std::cout << "Cars served: " << totalCarsServed_ << "\n";
    std::cout << "Avg satisfaction: " << std::setprecision(2) << averageSatisfaction() << "\n";
    std::cout << "Reputation score: " << std::setprecision(2) << reputation_.score() << "\n";
    std::cout << "Queue: " << queue_ << "\n";
    std::cout << "SpeedFactor: " << speedFactor_
              << " | ComfortBonus: " << comfortBonus_ << "\n";
    goals_.print(std::cout, *this);
    if (goals_.allAchieved()) {
        std::cout << "Status obiective: COMPLETATE 100%\n";
    }
    std::cout << "================================\n";
}

void CarWash::showReports() const {
    if (reports_.empty()) {
        std::cout << "Nu exista inca rapoarte zilnice.\n";
        return;
    }
    std::cout << "=== Rapoarte zilnice ===\n";
    for (const auto& r : reports_) {
        std::cout << r << "\n";
    }
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
        << "  buyupgrade X   - cumpara upgrade (1..3)\n"
        << "  buysupply T Q  - cumpara produse (water|shampoo|wax) cantitate Q\n"
        << "  setpricing M   - seteaza strategia de preturi (aggressive|balanced|conservative)\n"
        << "  reports        - afiseaza rapoarte zilnice\n"
        << "  events         - afiseaza evenimentele zilei curente\n"
        << "  endrun         - termina simularea\n";
}

void CarWash::buyUpgrade(int id) {
    std::unique_ptr<Upgrade> u;
    if (id == 1)      u = std::make_unique<BaySpeedUpgrade>();
    else if (id == 2) u = std::make_unique<ComfortUpgrade>();
    else if (id == 3) u = std::make_unique<MarketingUpgrade>();
    else throw InvalidCommandException("Upgrade necunoscut");

    if (u->cost() > cash_) {
        throw CarWashException("Nu ai suficienti bani pentru upgrade");
    }
    cash_ -= u->cost();
    u->apply(*this);
    purchased_.push_back(std::move(u));
}

void CarWash::buySupplies(const std::string& type, int amount) {
    if (amount <= 0) {
        throw InvalidCommandException("Cantitate invalida pentru buysupply");
    }

    double pricePerUnit = 0.0;
    int water = 0, shampoo = 0, wax = 0;

    if (type == "water") {
        pricePerUnit = 0.02;
        water = amount;
    } else if (type == "shampoo") {
        pricePerUnit = 0.05;
        shampoo = amount;
    } else if (type == "wax") {
        pricePerUnit = 0.08;
        wax = amount;
    } else {
        throw InvalidCommandException("Tip necunoscut de produs: " + type);
    }

    const double totalCost = pricePerUnit * static_cast<double>(amount);
    if (totalCost > cash_) {
        throw CarWashException("Nu ai suficienti bani pentru a cumpara aceste produse");
    }

    cash_ -= totalCost;
    inv_.addSupplies(water, shampoo, wax);

    logEvent("Cumparat " + std::to_string(amount) + " unitati " + type +
             " pentru " + std::to_string(totalCost) + " EUR");
}

void CarWash::logEvent(const std::string& msg) const {
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
        showReports();

        // folosim si cumpararea de produse in mod CI
        try {
            buySupplies("water", 100);
        } catch (const CarWashException&) {
        }

        setPricingMode("balanced");
        nextCommand();

        try {
            buyUpgrade(1);
        } catch (const CarWashException&) {
        }

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
            } else if (cmd == "buyupgrade") {
                int id = 0;
                iss >> id;
                if (id <= 0) {
                    throw InvalidCommandException("Folosire: buyupgrade <id>");
                }
                buyUpgrade(id);
                showDashboard();
            } else if (cmd == "buysupply") {
                std::string type;
                int amount = 0;
                iss >> type >> amount;
                if (type.empty() || amount <= 0) {
                    throw InvalidCommandException(
                        "Folosire: buysupply <water|shampoo|wax> <cantitate>");
                }
                buySupplies(type, amount);
                showStatus();
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
