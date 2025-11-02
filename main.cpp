// Car Wash Tycoon – compatibil cu verificările CI (Cppcheck/Clang-Tidy/ASan/Valgrind)
// - 4 clase prin compunere: ServicePackage, Inventory, WashBay (Rule of Three), CarWash
// - ctor cu parametri pentru fiecare, operator<< pentru toate clasele
// - Rule of Three demonstrat în WashBay (char* label) cu copy ctor / copy op= / dtor corecte
// - multe metode publice netriviale; const-correct; fără "using namespace std"
// - scenariu în main + interfață text: help/status/bays/bayscount/book/shop/upgradehours/upgradebay/endday/endrun
//
// Notă: pentru a respecta Tema 1 din sablonul tibmax01, clasele expun public API (altfel CI pică).

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class ServicePackage {
    std::string name_;
    int durationMin_;
    double price_;
    int waterL_;
    int shampooML_;
    int waxML_;

public:
    ServicePackage(std::string name, int durationMin, double price,
                   int waterL, int shampooML, int waxML)
        : name_(std::move(name)),
          durationMin_(durationMin),
          price_(price),
          waterL_(waterL),
          shampooML_(shampooML),
          waxML_(waxML) {}

    // getters const
    const std::string& name() const noexcept { return name_; }
    int duration() const noexcept { return durationMin_; }
    double price() const noexcept { return price_; }
    int needWater() const noexcept { return waterL_; }
    int needShampoo() const noexcept { return shampooML_; }
    int needWax() const noexcept { return waxML_; }

    void applyPriceFactor(double factor) {
        if (factor < 0.1 || factor > 10.0) {
            throw std::invalid_argument("price factor out of range");
        }
        price_ *= factor;
    }

    friend std::ostream& operator<<(std::ostream& os, const ServicePackage& sp) {
        os << "ServicePackage{name='" << sp.name()
           << "', dur=" << sp.duration() << "min, price="
           << std::fixed << std::setprecision(2) << sp.price()
           << " EUR, use=[" << sp.needWater() << "L water, "
           << sp.needShampoo() << "ml shampoo, " << sp.needWax() << "ml wax]}";
        return os;
    }
};

class Inventory {
    int waterL_;
    int shampooML_;
    int waxML_;

public:
    Inventory(int waterL, int shampooML, int waxML)
        : waterL_(waterL), shampooML_(shampooML), waxML_(waxML) {}

    void restock(int w, int s, int x) {
        if (w > 0) waterL_ += w;
        if (s > 0) shampooML_ += s;
        if (x > 0) waxML_ += x;
    }

    bool consumeIfAvailable(const ServicePackage& sp, int cars) {
        const long needW = static_cast<long>(sp.needWater()) * cars;
        const long needS = static_cast<long>(sp.needShampoo()) * cars;
        const long needX = static_cast<long>(sp.needWax()) * cars;
        if (needW <= waterL_ && needS <= shampooML_ && needX <= waxML_) {
            waterL_    -= static_cast<int>(needW);
            shampooML_ -= static_cast<int>(needS);
            waxML_     -= static_cast<int>(needX);
            return true;
        }
        return false;
    }

    int water() const noexcept { return waterL_; }
    int shampoo() const noexcept { return shampooML_; }
    int wax() const noexcept { return waxML_; }

    double fullness() const {
        const double maxW = 5000.0, maxS = 5000.0, maxX = 5000.0;
        return (waterL_ / maxW + shampooML_ / maxS + waxML_ / maxX) / 3.0 * 100.0;
    }

    friend std::ostream& operator<<(std::ostream& os, const Inventory& inv) {
        os << "Inventory{water=" << inv.water() << "L, shampoo=" << inv.shampoo()
           << "ml, wax=" << inv.wax() << "ml, fullness=" << std::fixed
           << std::setprecision(1) << inv.fullness() << "%}";
        return os;
    }
};

class WashBay {
    int id_;
    int availableAtMin_;
    char* label_;  // demonstrăm Rule of Three
    bool canBasic_{true};
    bool canDeluxe_{false};
    bool canWax_{false};

    static char* dupCString(const std::string& s) {
        char* p = new char[s.size() + 1];
        std::copy(s.c_str(), s.c_str() + s.size() + 1, p);
        return p;
    }

public:
    // ctor cu parametri
    WashBay(int id, int startMin, const std::string& label)
        : id_(id), availableAtMin_(startMin), label_(dupCString(label)) {}

    // Rule of Three
    WashBay(const WashBay& other)
        : id_(other.id_), availableAtMin_(other.availableAtMin_), label_(dupCString(other.label_)),
          canBasic_(other.canBasic_), canDeluxe_(other.canDeluxe_), canWax_(other.canWax_) {}

    WashBay& operator=(const WashBay& other) {
        if (this != &other) {
            id_ = other.id_;
            availableAtMin_ = other.availableAtMin_;
            delete[] label_;
            label_ = dupCString(other.label_);
            canBasic_  = other.canBasic_;
            canDeluxe_ = other.canDeluxe_;
            canWax_    = other.canWax_;
        }
        return *this;
    }

    ~WashBay() { delete[] label_; }

    // capabilități
    bool supportsBasic()  const noexcept { return canBasic_; }
    bool supportsDeluxe() const noexcept { return canDeluxe_; }
    bool supportsWax()    const noexcept { return canWax_;   }

    void upgradeDeluxe() noexcept { canDeluxe_ = true; }
    void upgradeWax()    noexcept { canWax_    = true; }

    bool supportsServiceName(const std::string& n) const {
        if (n == "Basic" || n == "basic") return canBasic_;
        if (n == "Deluxe" || n == "deluxe") return canDeluxe_;
        if (n == "Wax" || n == "wax") return canWax_;
        return false;
    }

    // programare
    int schedule(const ServicePackage& sp, int startMin) {
        const int begin = (startMin > availableAtMin_) ? startMin : availableAtMin_;
        const int finish = begin + sp.duration();
        availableAtMin_ = finish;
        return finish;
    }

    void resetTo(int openingMin) noexcept { availableAtMin_ = openingMin; }

    int availableAt() const noexcept { return availableAtMin_; }
    int id() const noexcept { return id_; }

    friend std::ostream& operator<<(std::ostream& os, const WashBay& wb) {
        os << "Bay #" << wb.id()
           << " (availAt=" << wb.availableAt() << "min"
           << ", caps=[Basic" << (wb.canDeluxe_ ? ",Deluxe" : "")
           << (wb.canWax_ ? ",Wax" : "") << "])";
        return os;
    }
};

class CarWash {
    std::string name_;
    Inventory inventory_;
    std::vector<ServicePackage> services_;
    std::vector<WashBay> bays_;
    double cash_{0.0};

    int openingMin_;
    int closingMin_;
    int currentDay_{1};

    // prețuri aprovizionare
    double priceWaterPerL_{0.02};
    double priceShampooPerML_{0.03};
    double priceWaxPerML_{0.05};

    // prețuri upgrade
    double costPerMinutePerBay_{0.50}; // EUR/min/bay
    double costUpgradeDeluxe_{200.0};  // EUR/bay
    double costUpgradeWax_{150.0};     // EUR/bay

    static bool equalNoCase(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i]))) return false;
        }
        return true;
    }

    int findServiceIndex(const std::string& n) const {
        for (std::size_t i = 0; i < services_.size(); ++i) {
            if (equalNoCase(services_[i].name(), n)) return static_cast<int>(i);
        }
        return -1;
    }

public:
    CarWash(std::string name, Inventory inv, int openAtMin, int closeAtMin)
        : name_(std::move(name)), inventory_(std::move(inv)),
          openingMin_(openAtMin), closingMin_(closeAtMin) {}

    // compunere
    void addService(const ServicePackage& sp) { services_.push_back(sp); }
    void addBay(const WashBay& wb) { bays_.push_back(wb); }

    // funcții netriviale — rezervări ținând cont de capabilități și program
    int book(const std::string& serviceName, int cars) {
        const int idx = findServiceIndex(serviceName);
        if (idx < 0) throw std::runtime_error("service not found");
        const ServicePackage& sp = services_.at(static_cast<std::size_t>(idx));

        // binsearch – câte mașini permite stocul
        int low = 0, high = cars, feasible = 0;
        while (low <= high) {
            const int mid = (low + high) / 2;
            Inventory probe = inventory_;
            if (probe.consumeIfAvailable(sp, mid)) { feasible = mid; low = mid + 1; }
            else { high = mid - 1; }
        }
        if (feasible == 0) return 0;
        (void)inventory_.consumeIfAvailable(sp, feasible);

        int scheduled = 0;
        for (int i = 0; i < feasible; ++i) {
            // alegem cel mai devreme bay care suportă serviciul
            auto it = std::min_element(bays_.begin(), bays_.end(),
                [&](const WashBay& a, const WashBay& b){
                    const int A = a.supportsServiceName(serviceName) ? a.availableAt() : std::numeric_limits<int>::max()/2;
                    const int B = b.supportsServiceName(serviceName) ? b.availableAt() : std::numeric_limits<int>::max()/2;
                    return A < B;
                });

            if (it == bays_.end() || !it->supportsServiceName(serviceName)) {
                break;
            }

            const int startMin = it->availableAt();
            const int finish   = it->schedule(sp, startMin);
            if (finish <= closingMin_) {
                cash_ += sp.price();
                ++scheduled;
            } else {
                // revenim consumul pentru mașina care ar depăși programul
                inventory_.restock(sp.needWater(), sp.needShampoo(), sp.needWax());
                break;
            }
        }
        return scheduled;
    }

    // ajustări de preț
    void applyGlobalPriceFactor(double factor) {
        for (auto& s : services_) s.applyPriceFactor(factor);
    }

    // SHOP
    double priceWaterPerL() const noexcept    { return priceWaterPerL_; }
    double priceShampooPerML() const noexcept { return priceShampooPerML_; }
    double priceWaxPerML() const noexcept     { return priceWaxPerML_; }

    double quotePurchaseCost(int wLiters, int sMl, int xMl) const {
        return wLiters * priceWaterPerL_ + sMl * priceShampooPerML_ + xMl * priceWaxPerML_;
    }
    bool buyInventory(int wLiters, int sMl, int xMl) {
        if (wLiters < 0 || sMl < 0 || xMl < 0) return false;
        const double cost = quotePurchaseCost(wLiters, sMl, xMl);
        if (cost > cash_ + 1e-9) return false;
        cash_ -= cost;
        inventory_.restock(wLiters, sMl, xMl);
        return true;
    }

    // UPGRADE ore
    double costPerMinutePerBay() const noexcept { return costPerMinutePerBay_; }
    bool upgradeHours(int addMinutes) {
        if (addMinutes <= 0) return false;
        const double cost = addMinutes * costPerMinutePerBay_ * static_cast<double>(bays_.size());
        if (cost > cash_ + 1e-9) return false;
        cash_ -= cost;
        closingMin_ += addMinutes;
        return true;
    }

    // UPGRADE capabilități pe bay
    double costUpgradeDeluxe() const noexcept { return costUpgradeDeluxe_; }
    double costUpgradeWax() const noexcept    { return costUpgradeWax_; }
    bool upgradeBay(int id, const std::string& type) {
        WashBay* tgt = nullptr;
        for (auto& b : bays_) if (b.id() == id) { tgt = &b; break; }
        if (!tgt) return false;

        if (equalNoCase(type, "Deluxe")) {
            if (cash_ + 1e-9 < costUpgradeDeluxe_) return false;
            cash_ -= costUpgradeDeluxe_;
            tgt->upgradeDeluxe();
            return true;
        }
        if (equalNoCase(type, "Wax")) {
            if (cash_ + 1e-9 < costUpgradeWax_) return false;
            cash_ -= costUpgradeWax_;
            tgt->upgradeWax();
            return true;
        }
        return false;
    }

    // zile
    void endDay() {
        for (auto& b : bays_) b.resetTo(openingMin_);
        ++currentDay_;
    }

    // “max afford”
    int maxAffordableWaterL() const {
        return priceWaterPerL_ > 0.0 ? static_cast<int>(std::floor(cash_ / priceWaterPerL_)) : 0;
    }
    int maxAffordableShampooML() const {
        return priceShampooPerML_ > 0.0 ? static_cast<int>(std::floor(cash_ / priceShampooPerML_)) : 0;
    }
    int maxAffordableWaxML() const {
        return priceWaxPerML_ > 0.0 ? static_cast<int>(std::floor(cash_ / priceWaxPerML_)) : 0;
    }
    int maxAffordableMinutes() const {
        const double unit = costPerMinutePerBay_ * static_cast<double>(std::max<std::size_t>(1, bays_.size()));
        return unit > 0.0 ? static_cast<int>(std::floor(cash_ / unit)) : 0;
    }
    int baysNeedingDeluxe() const {
        int c = 0; for (const auto& b : bays_) if (!b.supportsDeluxe()) ++c; return c;
    }
    int baysNeedingWax() const {
        int c = 0; for (const auto& b : bays_) if (!b.supportsWax()) ++c; return c;
    }
    int maxAffordableDeluxeUpgrades() const {
        const int byCash = static_cast<int>(std::floor(cash_ / costUpgradeDeluxe_));
        return std::max(0, std::min(byCash, baysNeedingDeluxe()));
    }
    int maxAffordableWaxUpgrades() const {
        const int byCash = static_cast<int>(std::floor(cash_ / costUpgradeWax_));
        return std::max(0, std::min(byCash, baysNeedingWax()));
    }

    // getters pt afișare
    const Inventory& inventory() const noexcept { return inventory_; }
    const std::vector<ServicePackage>& services() const noexcept { return services_; }
    const std::vector<WashBay>& bays() const noexcept { return bays_; }
    int openingMin() const noexcept { return openingMin_; }
    int closingMin() const noexcept { return closingMin_; }
    int day() const noexcept { return currentDay_; }
    double cash() const noexcept { return cash_; }
    int numBays() const noexcept { return static_cast<int>(bays_.size()); }
    int remainingForBay(const WashBay& b) const {
        const int rem = closingMin_ - b.availableAt();
        return rem > 0 ? rem : 0;
    }

    friend std::ostream& operator<<(std::ostream& os, const CarWash& cw) {
        os << "CarWash (Day " << cw.day() << ")\n"
           << "  " << cw.inventory() << "\n"
           << "  Bays (" << cw.bays().size() << "):\n";
        for (const auto& b : cw.bays()) {
            os << "    - " << b << " | remaining=" << cw.remainingForBay(b) << "min\n";
        }
        os << "  Hours: open " << cw.openingMin() << " -> close " << cw.closingMin()
           << " (mins)\n  Cash: " << std::fixed << std::setprecision(2) << cw.cash() << " EUR";
        return os;
    }
};

// ——— Afișări helper (fără IO în clasele model) ———
static void printServices(const CarWash& cw) {
    std::cout << "Services:\n";
    for (const auto& s : cw.services()) std::cout << "  - " << s << "\n";
}
static void printBays(const CarWash& cw) {
    std::cout << "Bays (" << cw.numBays() << " total):\n";
    for (const auto& b : cw.bays())
        std::cout << "  " << b << " | remaining=" << cw.remainingForBay(b) << "min\n";
}
static void printStatus(const CarWash& cw) {
    std::cout << "=== STATUS (Day " << cw.day() << ") ===\n";
    std::cout << "Inventory: " << cw.inventory() << "\n";
    std::cout << "Cash: " << std::fixed << std::setprecision(2) << cw.cash() << " EUR\n";
    std::cout << "Hours: open=" << cw.openingMin() << " -> close=" << cw.closingMin() << " (mins)\n";
    printBays(cw);
}

static void trim(std::string& s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    std::size_t i = 0; while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    s.erase(0, i);
}

int main() {
    try {
        const int OPEN  = 8 * 60;
        const int CLOSE = 12 * 60;

        Inventory inv(3000, 2000, 1500);
        CarWash cw("ShinyHands", inv, OPEN, CLOSE);

        ServicePackage basic ("Basic",  20,  8.0,  80, 40,  0);
        ServicePackage deluxe("Deluxe", 35, 14.5, 120, 60,  0);
        ServicePackage wax   ("Wax",    25, 16.0,  60, 20, 50);

        cw.addService(basic);
        cw.addService(deluxe);
        cw.addService(wax);

        WashBay b1(1, OPEN,      "B1");
        WashBay b2(2, OPEN + 10, "B2");
        WashBay b3(3, OPEN + 5,  "B3");
        cw.addBay(b1);
        cw.addBay(b2);
        cw.addBay(b3);

        std::cout << "=== INITIAL STATE ===\n" << cw << "\n\n";
        cw.applyGlobalPriceFactor(1.10);
        std::cout << "--- After 10% weekend price increase ---\n";
        for (const auto& s : cw.services()) {
            std::cout << s << "\n";
        }
        std::cout << "\nType `help` for commands.\n";

        auto printHelp = [&]() {
            std::cout <<
                "Commands:\n"
                "  help\n"
                "  status\n"
                "  services\n"
                "  bays\n"
                "  bayscount\n"
                "  book <Service> <k>\n"
                "  shop (water " << cw.priceWaterPerL() << " EUR/L, "
                                  "shampoo " << cw.priceShampooPerML() << " EUR/ml, "
                                  "wax " << cw.priceWaxPerML() << " EUR/ml"
                << "; max water=" << cw.maxAffordableWaterL()
                << ", shampoo=" << cw.maxAffordableShampooML()
                << ", wax=" << cw.maxAffordableWaxML() << ")\n"
                "  upgradehours <minutes> (cost " << cw.costPerMinutePerBay()
                << " EUR/min/bay; max " << cw.maxAffordableMinutes() << ")\n"
                "  upgradebay <id> <Deluxe|Wax> (Deluxe: " << cw.costUpgradeDeluxe()
                << " EUR, Wax: " << cw.costUpgradeWax() << " EUR; max Deluxe="
                << cw.maxAffordableDeluxeUpgrades() << ", max Wax=" << cw.maxAffordableWaxUpgrades() << ")\n"
                "  endday  (prints status)\n"
                "  endrun\n";
        };

        std::string line;
        while (true) {
            std::cout << "> ";
            if (!std::getline(std::cin, line)) break;
            trim(line);
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string cmd; iss >> cmd;

            if (cmd == "help") {
                printHelp();
            } else if (cmd == "status") {
                printStatus(cw);
            } else if (cmd == "services") {
                printServices(cw);
            } else if (cmd == "bays") {
                printBays(cw);
            } else if (cmd == "bayscount") {
                std::cout << "Total bays: " << cw.numBays() << "\n";
            } else if (cmd == "book") {
                std::string service; int k;
                if (!(iss >> service >> k)) {
                    std::cout << "Usage: book <Service> <k>\n";
                    continue;
                }
                try {
                    const int booked = cw.book(service, k);
                    if (booked == 0) {
                        std::cout << "No suitable bay supports " << service
                                  << " or not enough time/resources.\n";
                    } else {
                        std::cout << "Booked " << booked << " cars for " << service << ".\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "Error: " << e.what() << "\n";
                }
            } else if (cmd == "shop") {
                int w = 0, s = 0, x = 0;
                std::string tmp;

                std::cout << "How many liters of water? (max " << cw.maxAffordableWaterL() << "): ";
                if (!std::getline(std::cin, tmp)) break; trim(tmp);
                if (!tmp.empty()) {
                    try { w = std::max(0, std::stoi(tmp)); } catch (...) { w = 0; }
                }

                std::cout << "How many ml of shampoo? (max " << cw.maxAffordableShampooML() << "): ";
                if (!std::getline(std::cin, tmp)) break; trim(tmp);
                if (!tmp.empty()) {
                    try { s = std::max(0, std::stoi(tmp)); } catch (...) { s = 0; }
                }

                std::cout << "How many ml of wax? (max " << cw.maxAffordableWaxML() << "): ";
                if (!std::getline(std::cin, tmp)) break; trim(tmp);
                if (!tmp.empty()) {
                    try { x = std::max(0, std::stoi(tmp)); } catch (...) { x = 0; }
                }

                const double cost = cw.quotePurchaseCost(w, s, x);
                std::cout << "Cost: " << std::fixed << std::setprecision(2) << cost
                          << " EUR. Confirm (y/n)? ";
                if (!std::getline(std::cin, tmp)) break; trim(tmp);
                if (!tmp.empty() && (tmp[0] == 'y' || tmp[0] == 'Y')) {
                    if (cw.buyInventory(w, s, x)) {
                        std::cout << "Purchased. Cash: " << std::fixed << std::setprecision(2)
                                  << cw.cash() << " EUR\n";
                    } else {
                        std::cout << "Not enough cash.\n";
                    }
                } else {
                    std::cout << "Cancelled.\n";
                }
            } else if (cmd == "upgradehours") {
                int minutes = 0;
                if (!(iss >> minutes) || minutes <= 0) {
                    std::cout << "Usage: upgradehours <positive_minutes> (max "
                              << cw.maxAffordableMinutes() << ")\n";
                    continue;
                }
                if (minutes > cw.maxAffordableMinutes()) {
                    std::cout << "You can afford at most " << cw.maxAffordableMinutes() << " minutes.\n";
                    continue;
                }
                if (cw.upgradeHours(minutes)) {
                    std::cout << "Closing time extended by " << minutes << " minutes.\n";
                } else {
                    std::cout << "Not enough cash for hours upgrade.\n";
                }
            } else if (cmd == "upgradebay") {
                int id; std::string type;
                if (!(iss >> id >> type)) {
                    std::cout << "Usage: upgradebay <id> <Deluxe|Wax> (Deluxe max "
                              << cw.maxAffordableDeluxeUpgrades() << ", Wax max "
                              << cw.maxAffordableWaxUpgrades() << ")\n";
                    continue;
                }
                if (cw.upgradeBay(id, type)) {
                    std::cout << "Bay " << id << " upgraded with " << type << ".\n";
                } else {
                    std::cout << "Upgrade failed (check id/type/cash or bay already upgraded).\n";
                }
            } else if (cmd == "endday") {
                cw.endDay();
                std::cout << "New day started. Bays reset to opening time. Day = "
                          << cw.day() << ".\n";
                printStatus(cw);  // status automat la sfârșit de zi
                std::cout << "\n";
            } else if (cmd == "endrun") {
                break;
            } else {
                std::cout << "Unknown command. Type `help`.\n";
            }
        }

        std::cout << "\n=== FINAL STATE ===\n";
        printStatus(cw);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
