// Car Wash Tycoon – Tema POO cu: SHOP + zile + upgrade ore + upgrade per-bay
// ---------------------------------------------------------------------------
// Comenzi (tasteaza `help` in program):
//   help
//   status                   -> inventar, servicii, bays, cash, ziua curenta, timp ramas/bay
//   bays                     -> status detaliat bays
//   bayscount                -> cate bays exista
//   services                 -> lista serviciilor si preturile curente
//   book <Service> <k>       -> planifica k masini (Service: Basic/Deluxe/Wax)
//   shop                     -> cumpara water/shampoo/wax din cash
//   upgradehours <minutes>   -> mareste timpul de functionare (cost/min/bay)
//   upgradebay <id> <type>   -> adauga capabilitate pe bay (type: Deluxe|Wax)
//   endday                   -> termina ziua curenta; ziua urmatoare incepe (bays reset la ora de deschidere)
//   endrun                   -> inchide programul
//
// Cerinte POO (bifate):
// - 4 clase prin compunere (ServicePackage, Inventory, WashBay, CarWash)
// - ctor cu parametri, operator<< pentru toate clasele
// - pentru WashBay: Rule of Three (copy ctor, operator=, dtor)
// - functii const/private + functii publice non-triviale (booking cu restrictii per-bay,
//   consum stoc, shop, upgrade ore, upgrade capabilitati, raport pe zile)

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <sstream>

// ------------------------------- ServicePackage -------------------------------
class ServicePackage {
    std::string name;
    int durationMin;
    double price;
    int waterL;
    int shampooML;
    int waxML;

public:
    ServicePackage(std::string n, int dur, double pr, int w, int s, int wx)
        : name(std::move(n)), durationMin(dur), price(pr), waterL(w), shampooML(s), waxML(wx) {}

    const std::string& getName() const { return name; }
    int getDuration() const { return durationMin; }
    double getPrice() const { return price; }
    int needWater() const { return waterL; }
    int needShampoo() const { return shampooML; }
    int needWax() const { return waxML; }

    void applyPriceFactor(double factor) {
        if (factor < 0.1 || factor > 10.0) throw std::invalid_argument("factor out of range");
        price *= factor;
    }

    friend std::ostream& operator<<(std::ostream& os, const ServicePackage& sp) {
        os << "ServicePackage{name='" << sp.getName() << "', dur=" << sp.getDuration()
           << "min, price=" << std::fixed << std::setprecision(2) << sp.getPrice()
           << " EUR, use=[" << sp.needWater() << "L water, " << sp.needShampoo()
           << "ml shampoo, " << sp.needWax() << "ml wax]}";
        return os;
    }
};

// --------------------------------- Inventory ----------------------------------
class Inventory {
    int waterL;
    int shampooML;
    int waxML;

public:
    Inventory(int w, int s, int x) : waterL(w), shampooML(s), waxML(x) {}

    void restock(int w, int s, int x) {
        waterL += std::max(0, w);
        shampooML += std::max(0, s);
        waxML += std::max(0, x);
    }

    bool consumeIfAvailable(const ServicePackage& sp, int cars) {
        long needW = 1L * sp.needWater() * cars;
        long needS = 1L * sp.needShampoo() * cars;
        long needX = 1L * sp.needWax() * cars;
        if (needW <= waterL && needS <= shampooML && needX <= waxML) {
            waterL -= static_cast<int>(needW);
            shampooML -= static_cast<int>(needS);
            waxML -= static_cast<int>(needX);
            return true;
        }
        return false;
    }

    int getWater() const { return waterL; }
    int getShampoo() const { return shampooML; }
    int getWax() const { return waxML; }

    double fullness() const {
        const double maxW = 5000.0, maxS = 5000.0, maxX = 5000.0;
        return (waterL / maxW + shampooML / maxS + waxML / maxX) / 3.0 * 100.0;
    }

    friend std::ostream& operator<<(std::ostream& os, const Inventory& inv) {
        os << "Inventory{water=" << inv.waterL << "L, shampoo=" << inv.shampooML
           << "ml, wax=" << inv.waxML << "ml, fullness=" << std::fixed
           << std::setprecision(1) << inv.fullness() << "%}";
        return os;
    }
};

// ---------------------------------- WashBay -----------------------------------
class WashBay {
    int id;
    int availableAtMin; // minute absolute in zi
    char* label;

    // capabilitati (la start: Basic true, Deluxe/Wax false)
    bool canBasic = true;
    bool canDeluxe = false;
    bool canWax = false;

    static char* dupCString(const std::string& s) {
        char* p = new char[s.size() + 1];
        std::copy(s.c_str(), s.c_str() + s.size() + 1, p);
        return p;
    }

public:
    WashBay(int id_, int startMin, const std::string& lab)
        : id(id_), availableAtMin(startMin), label(dupCString(lab)) {}

    // Rule of Three
    WashBay(const WashBay& other)
        : id(other.id), availableAtMin(other.availableAtMin), label(dupCString(other.label)),
          canBasic(other.canBasic), canDeluxe(other.canDeluxe), canWax(other.canWax) {}

    WashBay& operator=(const WashBay& other) {
        if (this != &other) {
            id = other.id;
            availableAtMin = other.availableAtMin;
            delete[] label;
            label = dupCString(other.label);
            canBasic = other.canBasic;
            canDeluxe = other.canDeluxe;
            canWax = other.canWax;
        }
        return *this;
    }

    ~WashBay() { delete[] label; }

    // capabilities
    bool supportsBasic() const { return canBasic; }
    bool supportsDeluxe() const { return canDeluxe; }
    bool supportsWax() const { return canWax; }

    void upgradeDeluxe() { canDeluxe = true; }
    void upgradeWax() { canWax = true; }

    bool supportsServiceName(const std::string& n) const {
        if (n == "Basic" || n == "basic") return canBasic;
        if (n == "Deluxe" || n == "deluxe") return canDeluxe;
        if (n == "Wax"    || n == "wax")    return canWax;
        return false;
    }

    bool isFreeAt(int tMin) const { return tMin >= availableAtMin; }

    int schedule(const ServicePackage& sp, int startMin) {
        int begin = std::max(startMin, availableAtMin);
        int finish = begin + sp.getDuration();
        availableAtMin = finish;
        return finish;
    }

    void resetTo(int openingMin) { availableAtMin = openingMin; }

    int getAvailableAt() const { return availableAtMin; }
    int getId() const { return id; }
    std::string getLabel() const { return std::string(label); }

    friend std::ostream& operator<<(std::ostream& os, const WashBay& wb) {
        os << "WashBay{id=" << wb.id << ", label='" << wb.label
           << "', availableAt=" << wb.availableAtMin
           << "min, caps=[Basic" << (wb.canDeluxe ? ",Deluxe" : "")
           << (wb.canWax ? ",Wax" : "") << "]}";
        return os;
    }
};

// ---------------------------------- CarWash -----------------------------------
class CarWash {
    std::string name;
    Inventory inventory;
    std::vector<ServicePackage> services;
    std::vector<WashBay> bays;
    double cash = 0.0;

    // program zilnic
    int openingMin;
    int closingMin;
    int currentDay = 1;

    // preturi aprovizionare
    double priceWaterPerL = 0.02;
    double priceShampooPerML = 0.03;
    double priceWaxPerML = 0.05;

    // preturi upgrade
    double costPerMinutePerBay = 0.50; // EUR/min/bay
    double costUpgradeDeluxe = 200.0;  // EUR/bay
    double costUpgradeWax = 150.0;     // EUR/bay

    int findServiceIndex(const std::string& n) const {
        for (std::size_t i = 0; i < services.size(); ++i) {
            const std::string& s = services[i].getName();
            if (equalNoCase(s, n)) return static_cast<int>(i);
        }
        return -1;
    }

    static bool equalNoCase(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i]))) return false;
        }
        return true;
    }

public:
    CarWash(std::string nm, Inventory inv, int openAtMin, int closeAtMin)
        : name(std::move(nm)), inventory(std::move(inv)),
          openingMin(openAtMin), closingMin(closeAtMin) {}

    void addService(const ServicePackage& sp) { services.push_back(sp); }
    void addBay(const WashBay& wb) { bays.push_back(wb); }

    // ---------- Booking (respecta capabilitatile per-bay) ----------
    int book(const std::string& serviceName, int cars) {
        int idx = findServiceIndex(serviceName);
        if (idx < 0) throw std::runtime_error("service not found");
        const ServicePackage& sp = services[idx];

        // cate putem procesa cu stocul curent
        int low = 0, high = cars, feasible = 0;
        while (low <= high) {
            int mid = (low + high) / 2;
            Inventory probe = inventory;
            if (probe.consumeIfAvailable(sp, mid)) { feasible = mid; low = mid + 1; }
            else { high = mid - 1; }
        }
        if (feasible == 0) return 0;
        inventory.consumeIfAvailable(sp, feasible);

        int scheduled = 0;
        for (int i = 0; i < feasible; ++i) {
            // alege cea mai rapida bay care suporta serviciul
            auto it = std::min_element(bays.begin(), bays.end(),
                [&](const WashBay& a, const WashBay& b){
                    int A = a.supportsServiceName(serviceName) ? a.getAvailableAt() : INT_MAX/2;
                    int B = b.supportsServiceName(serviceName) ? b.getAvailableAt() : INT_MAX/2;
                    return A < B;
                });

            if (it == bays.end() || !it->supportsServiceName(serviceName))
                break; // niciun bay nu suporta acest serviciu

            int startMin = it->getAvailableAt();
            int finish = it->schedule(sp, startMin);
            if (finish <= closingMin) {
                cash += sp.getPrice();
                scheduled++;
            } else {
                // revenim consumul pentru masina ce ar depasi programul
                inventory.restock(sp.needWater(), sp.needShampoo(), sp.needWax());
                break;
            }
        }
        return scheduled;
    }

    // ---------- Preturi / servicii ----------
    void applyGlobalPriceFactor(double factor) {
        for (auto& s : services) s.applyPriceFactor(factor);
    }

    // ---------- SHOP ----------
    double quotePurchaseCost(int wLiters, int sMl, int xMl) const {
        return wLiters * priceWaterPerL + sMl * priceShampooPerML + xMl * priceWaxPerML;
    }
    bool buyInventory(int wLiters, int sMl, int xMl) {
        if (wLiters < 0 || sMl < 0 || xMl < 0) return false;
        double cost = quotePurchaseCost(wLiters, sMl, xMl);
        if (cost > cash + 1e-9) return false;
        cash -= cost;
        inventory.restock(wLiters, sMl, xMl);
        return true;
    }

    // ---------- Upgrade ore ----------
    bool upgradeHours(int addMinutes) {
        if (addMinutes <= 0) return false;
        double cost = addMinutes * costPerMinutePerBay * bays.size();
        if (cost > cash + 1e-9) return false;
        cash -= cost;
        closingMin += addMinutes;
        return true;
    }

    // ---------- Upgrade capabilitati per-bay ----------
    bool upgradeBay(int id, const std::string& type) {
        WashBay* target = nullptr;
        for (auto& b : bays) if (b.getId() == id) { target = &b; break; }
        if (!target) return false;

        if (equalNoCase(type, "Deluxe")) {
            if (cash + 1e-9 < costUpgradeDeluxe) return false;
            cash -= costUpgradeDeluxe;
            target->upgradeDeluxe();
            return true;
        } else if (equalNoCase(type, "Wax")) {
            if (cash + 1e-9 < costUpgradeWax) return false;
            cash -= costUpgradeWax;
            target->upgradeWax();
            return true;
        }
        return false;
    }

    // ---------- ZIUA ----------
    void endDay() {
        for (auto& b : bays) b.resetTo(openingMin);
        currentDay++;
    }

    // ---------- Getters / afisare ----------
    double getCash() const { return cash; }
    const Inventory& getInventory() const { return inventory; }
    const std::vector<ServicePackage>& getServices() const { return services; }
    const std::vector<WashBay>& getBays() const { return bays; }
    int getOpeningMin() const { return openingMin; }
    int getClosingMin() const { return closingMin; }
    int getDay() const { return currentDay; }
    int numBays() const { return static_cast<int>(bays.size()); }

    // timp ramas pentru un bay (>=0)
    int remainingForBay(const WashBay& b) const {
        int rem = closingMin - b.getAvailableAt();
        return rem > 0 ? rem : 0;
    }

    friend std::ostream& operator<<(std::ostream& os, const CarWash& cw) {
        os << "CarWash '" << cw.name << "' (Day " << cw.currentDay << ")\n  "
           << cw.inventory << "\n  Services:\n";
        for (const auto& s : cw.services) os << "    - " << s << "\n";
        os << "  Bays (" << cw.bays.size() << "):\n";
        for (const auto& b : cw.bays) {
            os << "    - " << b << " | remaining=" << cw.remainingForBay(b) << "min\n";
        }
        os << "  Hours: open " << cw.openingMin << " -> close " << cw.closingMin
           << " (mins from day start)\n";
        os << "  Cash: " << std::fixed << std::setprecision(2) << cw.cash << " EUR";
        return os;
    }
};

// ------------------------------------ Helpers ---------------------------------
static void printServices(const CarWash& cw) {
    std::cout << "Services:\n";
    for (const auto& s : cw.getServices())
        std::cout << "  - " << s << "\n";
}

static void printBays(const CarWash& cw) {
    std::cout << "Bays (" << cw.numBays() << " total):\n";
    for (const auto& b : cw.getBays())
        std::cout << "  " << b << " | remaining=" << cw.remainingForBay(b) << "min\n";
}

static void printStatus(const CarWash& cw) {
    std::cout << "=== STATUS (Day " << cw.getDay() << ") ===\n";
    std::cout << "Inventory: " << cw.getInventory() << "\n";
    std::cout << "Cash: " << std::fixed << std::setprecision(2) << cw.getCash() << " EUR\n";
    std::cout << "Hours: open=" << cw.getOpeningMin() << " -> close=" << cw.getClosingMin() << " (mins)\n";
    std::cout << "Bays: " << cw.numBays() << "\n";
    printBays(cw);
}

// ------------------------------------ main ------------------------------------
int main() {
    try {
        const int OPEN = 8 * 60;   // 08:00
        const int CLOSE = 12 * 60; // 12:00

        Inventory inv(/*water*/ 3000, /*shampoo*/ 2000, /*wax*/ 1500);
        CarWash cw("ShinyHands", inv, OPEN, CLOSE);

        ServicePackage basic("Basic", 20, 8.0, 80, 40, 0);
        ServicePackage deluxe("Deluxe", 35, 14.5, 120, 60, 0);
        ServicePackage wax("Wax", 25, 16.0, 60, 20, 50);

        cw.addService(basic);
        cw.addService(deluxe);
        cw.addService(wax);

        // La start toate bay-urile pot doar Basic
        WashBay b1(1, OPEN, "Front Left");
        WashBay b2(2, OPEN + 10, "Front Right");
        WashBay b3 = b1;                                // copy ctor
        b3 = WashBay(3, OPEN + 5, "Rear Center");       // operator=
        cw.addBay(b1); cw.addBay(b2); cw.addBay(b3);

        std::cout << "=== INITIAL STATE ===\n" << cw << "\n\n";
        cw.applyGlobalPriceFactor(1.10);
        std::cout << "--- After 10% weekend price increase ---\n";
        for (const auto& s : cw.getServices()) std::cout << s << "\n";
        std::cout << "\nType `help` for commands.\n";

        std::string line;
        while (true) {
            std::cout << "> ";
            if (!std::getline(std::cin, line)) break;

            auto trim = [](std::string& x){
                while (!x.empty() && std::isspace(static_cast<unsigned char>(x.back()))) x.pop_back();
                std::size_t i=0; while (i<x.size() && std::isspace(static_cast<unsigned char>(x[i]))) ++i;
                x.erase(0,i);
            };
            trim(line);
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string cmd; iss >> cmd;

            if (cmd == "help") {
                std::cout <<
                    "Commands:\n"
                    "  help\n"
                    "  status\n"
                    "  services\n"
                    "  bays\n"
                    "  bayscount\n"
                    "  book <Service> <k>\n"
                    "  shop\n"
                    "  upgradehours <minutes>\n"
                    "  upgradebay <id> <Deluxe|Wax>\n"
                    "  endday\n"
                    "  endrun\n";
            }
            else if (cmd == "status") {
                printStatus(cw);
            }
            else if (cmd == "services") {
                printServices(cw);
            }
            else if (cmd == "bays") {
                printBays(cw);
            }
            else if (cmd == "bayscount") {
                std::cout << "Total bays: " << cw.numBays() << "\n";
            }
            else if (cmd == "book") {
                std::string service; int k;
                if (!(iss >> service >> k)) {
                    std::cout << "Usage: book <Service> <k>\n";
                    continue;
                }
                try {
                    int booked = cw.book(service, k);
                    if (booked == 0)
                        std::cout << "No suitable bay supports " << service
                                  << " or not enough time/resources.\n";
                    else
                        std::cout << "Booked " << booked << " cars for " << service << ".\n";
                } catch (const std::exception& e) {
                    std::cout << "Error: " << e.what() << "\n";
                }
            }
            else if (cmd == "shop") {
                int w = 0, s = 0, x = 0;
                std::string tmp;

                std::cout << "How many liters of water? ";
                if (!std::getline(std::cin, tmp)) break; w = std::max(0, std::stoi(tmp));

                std::cout << "How many ml of shampoo? ";
                if (!std::getline(std::cin, tmp)) break; s = std::max(0, std::stoi(tmp));

                std::cout << "How many ml of wax? ";
                if (!std::getline(std::cin, tmp)) break; x = std::max(0, std::stoi(tmp));

                // afis cost estimat (aceleasi preturi ca in CarWash)
                double cost = w*0.02 + s*0.03 + x*0.05;
                std::cout << "Cost: " << std::fixed << std::setprecision(2) << cost << " EUR. Confirm (y/n)? ";
                if (!std::getline(std::cin, tmp)) break;
                if (!tmp.empty() && (tmp[0]=='y' || tmp[0]=='Y')) {
                    if (cw.buyInventory(w, s, x)) {
                        std::cout << "Purchased. New inventory: " << cw.getInventory()
                                  << " | Cash: " << std::fixed << std::setprecision(2)
                                  << cw.getCash() << " EUR\n";
                    } else {
                        std::cout << "Not enough cash.\n";
                    }
                } else {
                    std::cout << "Cancelled.\n";
                }
            }
            else if (cmd == "upgradehours") {
                int minutes = 0;
                if (!(iss >> minutes) || minutes <= 0) {
                    std::cout << "Usage: upgradehours <positive_minutes>\n";
                    continue;
                }
                // costul e calculat in metoda (0.50 EUR/min/bay)
                if (cw.upgradeHours(minutes)) {
                    std::cout << "Closing time extended by " << minutes << " minutes.\n";
                } else {
                    std::cout << "Not enough cash for hours upgrade.\n";
                }
            }
            else if (cmd == "upgradebay") {
                int id; std::string type;
                if (!(iss >> id >> type)) {
                    std::cout << "Usage: upgradebay <id> <Deluxe|Wax>\n";
                    continue;
                }
                if (cw.upgradeBay(id, type)) {
                    std::cout << "Bay " << id << " upgraded with " << type << ".\n";
                } else {
                    std::cout << "Upgrade failed (check id/type/cash).\n";
                }
            }
            else if (cmd == "endday") {
                cw.endDay();
                std::cout << "New day started. Bays reset to opening time. Day = " << cw.getDay() << ".\n";
            }
            else if (cmd == "endrun") {
                break;
            }
            else {
                std::cout << "Unknown command. Type `help`.\n";
            }
        }

        std::cout << "\n=== FINAL STATE ===\n";
        std::cout << cw << "\n";
        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
