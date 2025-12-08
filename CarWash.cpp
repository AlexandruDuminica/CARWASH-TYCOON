#include "CarWash.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include "BasicService.h"
#include "DeluxeService.h"
#include "WaxService.h"
#include "EcoService.h"

double CarWash::r2(double v) const {
    return std::round(v * 100.0) / 100.0;
}

bool CarWash::geMoney(double a, double b) const {
    return r2(a) >= r2(b);
}

bool CarWash::gtMoney(double a, double b) const {
    return r2(a) > r2(b);
}

bool CarWash::sameCaseInsensitive(const std::string& a, const std::string& b) const {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        char ca = static_cast<char>(std::tolower(static_cast<unsigned char>(a[i])));
        char cb = static_cast<char>(std::tolower(static_cast<unsigned char>(b[i])));
        if (ca != cb) return false;
    }
    return true;
}

int CarWash::findService(const std::string& name) const {
    for (std::size_t i = 0; i < services_.size(); ++i) {
        if (services_[i] && sameCaseInsensitive(services_[i]->name(), name)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void CarWash::trim(std::string& s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }
    std::size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) {
        ++i;
    }
    s.erase(0, i);
}

int CarWash::toIntOrZero(const std::string& t) {
    if (t.empty()) return 0;
    char* e = nullptr;
    long v = std::strtol(t.c_str(), &e, 10);
    if (e == t.c_str() || *e != '\0') return 0;
    if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) return 0;
    return static_cast<int>(v);
}

CarWash::CarWash(std::string n, Inventory inv, int openM, int closeM)
    : name_(std::move(n)),
      inv_(inv),
      openMin_(openM),
      closeMin_(closeM) {
    if (openM >= closeM) {
        throw CarWashException("Program invalid: ora de deschidere >= ora de inchidere");
    }
}

bool CarWash::addService(const WashService& s) {
    if (services_.size() >= MAX_SERV) return false;
    services_.push_back(s.clone()); // constructor virtual
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
        throw InvalidServiceException("Serviciul \"" + serviceName + "\" nu exista");
    }
    if (cars <= 0) {
        throw BookingException("Numarul de masini trebuie sa fie pozitiv");
    }

    const WashService& sp = *services_.at(static_cast<std::size_t>(si));

    int low = 0, high = cars, ok = 0;
    while (low <= high) {
        int mid = (low + high) / 2;
        Inventory probe = inv_;
        if (probe.takeIfCan(sp, mid)) {
            ok = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    if (ok == 0) {
        throw InventoryException("Nu exista suficiente resurse pentru nicio masina");
    }
    inv_.takeIfCan(sp, ok);

    int done = 0;
    for (int c = 0; c < ok; ++c) {
        int idx = -1;
        int bestTime = std::numeric_limits<int>::max() / 2;
        for (std::size_t i = 0; i < bays_.size(); ++i) {
            if (!bays_[i]) continue;
            if (!bays_[i]->canDo(serviceName)) continue;
            int a = bays_[i]->availAt();
            if (a < bestTime) {
                bestTime = a;
                idx = static_cast<int>(i);
            }
        }
        if (idx == -1) break;
        int finish = bays_[static_cast<std::size_t>(idx)]
                         ->bookOne(sp, bays_[static_cast<std::size_t>(idx)]->availAt());
        if (finish <= closeMin_) {
            cash_ += sp.price();
            ++done;
        } else {
            inv_.add(sp.needW(), sp.needS(), sp.needX());
            break;
        }
    }
    return done;
}

void CarWash::priceAll(double factor) {
    for (auto& p : services_) {
        if (p) p->applyFactor(factor);
    }
}

double CarWash::quoteBuy(int w, int s, int x) const {
    return w * priceW_ + s * priceS_ + x * priceX_;
}

bool CarWash::buy(int w, int s, int x) {
    if (w < 0 || s < 0 || x < 0) {
        throw InventoryException("Cantitati negative la cumparare");
    }
    double c = quoteBuy(w, s, x);
    if (gtMoney(c, cash_)) {
        throw InventoryException("Fonduri insuficiente pentru achizitie");
    }
    cash_ -= r2(c);
    inv_.add(w, s, x);
    return true;
}

bool CarWash::addMinutes(int m) {
    if (m <= 0) {
        throw CarWashException("Numarul de minute trebuie sa fie pozitiv");
    }
    double unit = costPerMinPerBay_ * static_cast<double>(!bays_.empty() ? bays_.size() : 1);
    double cost = m * unit;
    if (gtMoney(cost, cash_)) {
        throw CarWashException("Fonduri insuficiente pentru extinderea programului");
    }
    cash_ -= r2(cost);
    closeMin_ += m;
    return true;
}

bool CarWash::upgradeBay(int id, const std::string& kind) {
    WashBay* target = nullptr;
    for (auto& p : bays_) {
        if (p && p->id() == id) {
            target = p.get();
            break;
        }
    }
    if (!target) {
        throw CarWashException("Bay inexistent");
    }

    if (sameCaseInsensitive(kind, "Deluxe")) {
        if (!geMoney(cash_, costDeluxe_)) {
            throw CarWashException("Fonduri insuficiente pentru upgrade Deluxe");
        }
        cash_ -= r2(costDeluxe_);
        target->addDeluxe();
        return true;
    }
    if (sameCaseInsensitive(kind, "Wax")) {
        if (!geMoney(cash_, costWax_)) {
            throw CarWashException("Fonduri insuficiente pentru upgrade Wax");
        }
        cash_ -= r2(costWax_);
        target->addWax();
        return true;
    }
    throw InvalidCommandException("Tip de upgrade necunoscut: " + kind);
}

void CarWash::endDay() {
    for (auto& p : bays_) {
        if (p) p->reset(openMin_);
    }
    ++day_;
}

int CarWash::maxBuyW() const {
    return priceW_ > 0.0 ? static_cast<int>(std::floor(r2(cash_) / priceW_)) : 0;
}
int CarWash::maxBuyS() const {
    return priceS_ > 0.0 ? static_cast<int>(std::floor(r2(cash_) / priceS_)) : 0;
}
int CarWash::maxBuyX() const {
    return priceX_ > 0.0 ? static_cast<int>(std::floor(r2(cash_) / priceX_)) : 0;
}
int CarWash::maxAddMin() const {
    double u = costPerMinPerBay_ * static_cast<double>(!bays_.empty() ? bays_.size() : 1);
    return u > 0.0 ? static_cast<int>(std::floor(r2(cash_) / u)) : 0;
}

void CarWash::showServices() const {
    std::cout << "Services:\n";
    for (const auto& p : services_) {
        if (!p) continue;
        std::cout << "  - " << *p;

        // downcast cu sens pentru Deluxe
        if (dynamic_cast<const DeluxeService*>(p.get())) {
            std::cout << " [premium]";
        }

        std::cout << " | 5 cars: " << std::fixed << std::setprecision(2)
                  << p->finalPriceForCars(5) << " EUR\n";
    }
}

void CarWash::showBays() const {
    std::cout << "Bays (" << bays_.size() << "):\n";
    for (const auto& p : bays_) {
        if (!p) continue;
        int rem = closeMin_ - p->availAt();
        if (rem < 0) rem = 0;
        std::cout << "  " << *p << " | remaining=" << rem << "min\n";
    }
    std::cout << "Total bays created: " << WashBay::totalBaysCreated() << "\n";
}

void CarWash::showStatus() const {
    std::cout << "=== STATUS (Day " << day_ << ") ===\n";
    std::cout << "Inventory: " << inv_ << "\n";
    std::cout << "Cash: " << std::fixed << std::setprecision(2) << cash_ << " EUR\n";
    std::cout << "Hours: open=" << openMin_ << " -> close=" << closeMin_ << " (mins)\n";
    showBays();
}

void CarWash::showHelp() const {
    std::cout
        << "Commands:\n"
        << "  help\n"
        << "  status\n"
        << "  services\n"
        << "  bays\n"
        << "  bayscount\n"
        << "  book <Service> <k>\n"
        << "  shop (water " << priceW() << " EUR/L, shampoo " << priceS()
        << " EUR/ml, wax " << priceX() << " EUR/ml; max W=" << maxBuyW()
        << ", S=" << maxBuyS() << ", X=" << maxBuyX() << ")\n"
        << "  upgradehours <minutes> (cost " << costMinPerBay()
        << " EUR/min/bay; max " << maxAddMin() << ")\n"
        << "  upgradebay <id> <Deluxe|Wax> (Deluxe " << costDeluxe()
        << " EUR, Wax " << costWax() << " EUR)\n"
        << "  endday\n"
        << "  endrun\n";
}

void CarWash::run() {
    std::cout << "=== INITIAL STATE ===\n";
    showStatus();
    std::cout << "\n";

    priceAll(1.10);
    std::cout << "--- After 10% weekend price increase ---\n";
    showServices();
    std::cout << "\n";
    std::cout << "Type `help` for commands.\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        trim(line);
        if (line.empty()) {
            continue;
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
            } else if (cmd == "bayscount") {
                std::cout << "Total bays: " << bays_.size() << "\n";
            } else if (cmd == "book") {
                std::string svc;
                int k = 0;
                iss >> svc >> k;
                if (svc.empty()) {
                    throw InvalidCommandException("Usage: book <Service> <k>");
                }
                int got = bookCars(svc, k);
                std::cout << "Booked " << got << " cars for " << svc << ".\n";
            } else if (cmd == "shop") {
                std::string t;
                int w = 0, s = 0, x = 0;

                std::cout << "Water L (max " << maxBuyW() << "): ";
                if (!std::getline(std::cin, t)) break;
                trim(t);
                w = toIntOrZero(t);

                std::cout << "Shampoo ml (max " << maxBuyS() << "): ";
                if (!std::getline(std::cin, t)) break;
                trim(t);
                s = toIntOrZero(t);

                std::cout << "Wax ml (max " << maxBuyX() << "): ";
                if (!std::getline(std::cin, t)) break;
                trim(t);
                x = toIntOrZero(t);

                double cost = quoteBuy(w, s, x);
                std::cout << "Cost: " << std::fixed << std::setprecision(2) << cost
                          << " EUR. Confirm (y/n)? ";
                if (!std::getline(std::cin, t)) break;
                trim(t);
                if (!t.empty() && (t[0] == 'y' || t[0] == 'Y')) {
                    buy(w, s, x);
                    std::cout << "Purchased.\n";
                } else {
                    std::cout << "Cancelled.\n";
                }
            } else if (cmd == "upgradehours") {
                int m = 0;
                iss >> m;
                if (m <= 0) {
                    throw InvalidCommandException("Usage: upgradehours <positive_minutes>");
                }
                if (m > maxAddMin()) {
                    std::cout << "You can afford at most " << maxAddMin() << " minutes.\n";
                    continue;
                }
                addMinutes(m);
                std::cout << "Closing time extended by " << m << " minutes.\n";
            } else if (cmd == "upgradebay") {
                int id = 0;
                std::string kind;
                iss >> id >> kind;
                if (id <= 0 || kind.empty()) {
                    throw InvalidCommandException("Usage: upgradebay <id> <Deluxe|Wax>");
                }
                upgradeBay(id, kind);
                std::cout << "Bay " << id << " upgraded: " << kind << ".\n";
            } else if (cmd == "endday") {
                endDay();
                showStatus();
                std::cout << "\n";
            } else if (cmd == "endrun") {
                break;
            } else {
                throw InvalidCommandException("Comanda necunoscuta: " + cmd);
            }
        } catch (const CarWashException& ex) {
            std::cout << "Eroare: " << ex.what() << "\n";
        }
    }

    std::cout << "\n=== FINAL STATE ===\n";
    showStatus();
}
