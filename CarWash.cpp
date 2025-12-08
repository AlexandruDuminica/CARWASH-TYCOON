#include "CarWash.h"
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
}

bool CarWash::sameCaseInsensitive(const std::string &a,
                                  const std::string &b) const {
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

bool CarWash::addService(const WashService &s) {
    if (services_.size() >= MAX_SERV) return false;
    services_.push_back(s.clone());
    return true;
}

bool CarWash::addBay(const WashBay &b) {
    if (bays_.size() >= MAX_BAYS) return false;
    bays_.push_back(std::make_unique<WashBay>(b));
    return true;
}

int CarWash::bookCars(const std::string &serviceName, int cars) {
    int si = findService(serviceName);
    if (si < 0) {
        throw InvalidServiceException("Serviciul nu exista: " + serviceName);
    }
    if (cars <= 0) {
        throw BookingException("Numar de masini invalid");
    }
    WashService &sp = *services_.at(static_cast<size_t>(si));

    int booked = 0;
    for (int c = 0; c < cars; ++c) {
        bool ok = false;
        for (auto &bay : bays_) {
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

void CarWash::priceAll(double f) {
    for (auto &p : services_) {
        if (p) p->applyFactor(f);
    }
}

void CarWash::simulateHour() {
    nowMin_ += 60;

    std::vector<std::string> names;
    names.reserve(services_.size());
    for (const auto &p : services_) {
        if (p) names.push_back(p->name());
    }
    queue_.generate(names);

    int processed = 0;
    int attempts = static_cast<int>(bays_.size()) * 4;
    for (int k = 0; k < attempts; ++k) {
        if (queue_.empty()) break;
        std::string svc = queue_.pop();
        try {
            int got = bookCars(svc, 1);
            if (got == 1) {
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
        for (auto &b : bays_) {
            b->reset(openMin_);
        }
        nowMin_ = openMin_;
        ++day_;
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
    showQueue();
}

void CarWash::showServices() const {
    std::cout << "SERVICII:\n";
    for (const auto &p : services_) {
        if (!p) continue;
        std::cout << "  - " << *p;
        if (dynamic_cast<DeluxeService*>(p.get())) {
            std::cout << " [premium]";
        }
        std::cout << "\n";
    }
}

void CarWash::showBays() const {
    std::cout << "BAIE (" << bays_.size() << "), create="
              << WashBay::totalBaysCreated() << "\n";
    for (const auto &b : bays_) {
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

void CarWash::showHelp() const {
    std::cout
        << "Comenzi:\n"
        << "  help\n"
        << "  status\n"
        << "  services\n"
        << "  bays\n"
        << "  queue\n"
        << "  next   (simuleaza o ora de clienti)\n"
        << "  endrun\n";
}

void CarWash::run() {
    std::cout << "=== CARWASH TYCOON ===\n";
    priceAll(1.10);
    showHelp();

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        try {
            if (line == "help") {
                showHelp();
            } else if (line == "status") {
                showStatus();
            } else if (line == "services") {
                showServices();
            } else if (line == "bays") {
                showBays();
            } else if (line == "queue") {
                showQueue();
            } else if (line == "next") {
                nextCommand();
            } else if (line == "endrun") {
                break;
            } else if (line.empty()) {
                continue;
            } else {
                throw InvalidCommandException("Comanda necunoscuta: " + line);
            }
        } catch (const CarWashException &ex) {
            std::cout << "Eroare: " << ex.what() << "\n";
        }
    }

    std::cout << "=== FINAL ===\n";
    showStatus();
}
