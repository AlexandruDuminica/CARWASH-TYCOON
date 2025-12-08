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
    WashService& sp = *services_.at(static_cast<size_t>(si));

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

void CarWash::simulateHour() {
    nowMin_ += 60;

    // crestere cerere din upgrade-uri
    int extra = baseDemandBonus_ > 0 ? baseDemandBonus_ : 0;
    for (int i = 0; i < extra; ++i) {
        queue_.increaseDemand();
    }
    queue_.generateRandomCustomers();

    std::vector<WashService*> servicePtrs;
    servicePtrs.reserve(services_.size());
    for (auto& s : services_)
        if (s) servicePtrs.push_back(s.get());

    int attempts = static_cast<int>(bays_.size()) * 4 * speedFactor_;
    int processed = 0;

    for (int k = 0; k < attempts; ++k) {
        if (queue_.empty()) break;

        auto customer = queue_.pop();
        if (!customer) break;

        const WashService* chosen = customer->chooseService(servicePtrs);
        if (!chosen) {
            queue_.failOne();
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
                processed++;
                demand_.success();
            } else {
                queue_.failOne();
                demand_.fail();
            }
        } catch (const CarWashException&) {
            queue_.failOne();
            demand_.fail();
        }
    }

    int adj = demand_.adjust();
    if (adj > 0) queue_.increaseDemand();
    if (adj < 0) queue_.decreaseDemand();
    if (adj != 0) demand_.reset();

    if (nowMin_ >= closeMin_) {
        for (auto& b : bays_) {
            b->reset(openMin_);
        }
        nowMin_ = openMin_;
        ++day_;
        goals_.checkAll(*this);
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
    std::cout << goals_;
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
    std::cout << "Cash: " << std::fixed << std::setprecision(2) << cash_ << " EUR\n";
    std::cout << "Cars served: " << totalCarsServed_ << "\n";
    std::cout << "Avg satisfaction: " << std::setprecision(2) << averageSatisfaction() << "\n";
    std::cout << "Queue: " << queue_ << "\n";
    std::cout << "SpeedFactor: " << speedFactor_
              << " | ComfortBonus: " << comfortBonus_ << "\n";
    std::cout << goals_;
    std::cout << "================================\n";
}

void CarWash::showHelp() const {
    std::cout
        << "Comenzi:\n"
        << "  help         - afiseaza acest mesaj\n"
        << "  status       - stare detaliata\n"
        << "  services     - lista servicii\n"
        << "  bays         - lista bai\n"
        << "  queue        - info coada\n"
        << "  next         - simuleaza o ora\n"
        << "  dashboard    - afiseaza rezumat tycoon\n"
        << "  goals        - afiseaza obiective\n"
        << "  upgrades     - lista upgrade-uri\n"
        << "  buyupgrade X - cumpara upgrade (1..3)\n"
        << "  endrun       - termina simularea\n";
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

void CarWash::run() {
    std::cout << "=== CARWASH TYCOON ===\n";
    showHelp();
    showDashboard();

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

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
            } else if (cmd == "dashboard") {
                showDashboard();
            } else if (cmd == "goals") {
                showGoals();
            } else if (cmd == "upgrades") {
                showUpgrades();
            } else if (cmd == "buyupgrade") {
                int id = 0; iss >> id;
                if (id <= 0) throw InvalidCommandException("Folosire: buyupgrade <id>");
                buyUpgrade(id);
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
}
