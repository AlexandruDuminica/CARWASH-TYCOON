// Car Wash Tycoon – Tema POO cu interfata text + SHOP + zile multiple
// -------------------------------------------------------------------
// Comenzi (tasteaza `help` in program):
//   help                 -> lista comenzilor
//   status               -> inventar, servicii, boxe, cash, ziua curenta
//   services             -> lista serviciilor si preturile curente
//   bays                 -> statusul boxelor (cand se elibereaza)
//   book <Serviciu> <k>  -> programeaza k masini (Basic/Deluxe/Wax)
//   shop                 -> cumpara apa/sampon/ceara din cash
//   endday               -> termina ziua curenta si treci la urmatoarea (progres pe zile)
//   endrun               -> inchide programul (afiseaza starea finala)
//
// Cerinte POO bifate ca in versiunile anterioare:
// - 4 clase prin compunere: ServicePackage, Inventory, WashBay, CarWash
// - Constructori cu parametri pentru fiecare
// - Pentru WashBay: constructor de copiere, operator=, destructor (Rule of Three)
// - operator<< pentru TOATE clasele
// - Functii const, metode private, functii publice non-triviale (booking, consum stoc,
//   shop, raport, avansare zi)
// - Scenariu in main: meniu text interactiv

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
    int durationMin;     // durata per masina
    double price;        // incasare per masina
    int waterL;          // consum inventar
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
    int availableAtMin; // cand devine liber (minute din zi)
    char* label;        // alocat dinamic

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
        : id(other.id), availableAtMin(other.availableAtMin), label(dupCString(other.label)) {}

    WashBay& operator=(const WashBay& other) {
        if (this != &other) {
            id = other.id;
            availableAtMin = other.availableAtMin;
            delete[] label;
            label = dupCString(other.label);
        }
        return *this;
    }

    ~WashBay() { delete[] label; }

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
           << "', availableAt=" << wb.availableAtMin << "min}";
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
    int openingMin; // ex: 08:00
    int closingMin; // ex: 12:00
    int currentDay = 1;

    // preturi de aprovizionare (EUR per unitate)
    double priceWaterPerL = 0.02;    // 2 eurocenti / L
    double priceShampooPerML = 0.03; // 3 eurocenti / ml
    double priceWaxPerML = 0.05;     // 5 eurocenti / ml

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

    // ---------- Booking ----------
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
            auto it = std::min_element(bays.begin(), bays.end(),
                [](const WashBay& a, const WashBay& b){ return a.getAvailableAt() < b.getAvailableAt(); });
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

    friend std::ostream& operator<<(std::ostream& os, const CarWash& cw) {
        os << "CarWash '" << cw.name << "' (Day " << cw.currentDay << ")\n  "
           << cw.inventory << "\n  Services:\n";
        for (const auto& s : cw.services) os << "    - " << s << "\n";
        os << "  Bays:\n";
        for (const auto& b : cw.bays) os << "    - " << b << "\n";
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
    std::cout << "Bays:\n";
    for (const auto& b : cw.getBays())
        std::cout << "  " << b << "\n";
}

static void printStatus(const CarWash& cw) {
    std::cout << "=== STATUS (Day " << cw.getDay() << ") ===\n";
    std::cout << "Inventory: " << cw.getInventory() << "\n";
    printServices(cw);
    printBays(cw);
    std::cout << "Cash: " << std::fixed << std::setprecision(2) << cw.getCash() << " EUR\n";
}

// ------------------------------------ main ------------------------------------
int main() {
    try {
        // 1) Setup initial
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

        WashBay b1(1, OPEN, "Front Left");
        WashBay b2(2, OPEN + 10, "Front Right");
        WashBay b3 = b1;                               // copy ctor
        b3 = WashBay(3, OPEN + 5, "Rear Center");      // operator=

        cw.addBay(b1);
        cw.addBay(b2);
        cw.addBay(b3);

        std::cout << "=== INITIAL STATE ===\n" << cw << "\n\n";
        cw.applyGlobalPriceFactor(1.10);
        std::cout << "--- After 10% weekend price increase ---\n";
        for (const auto& s : cw.getServices()) std::cout << s << "\n";
        std::cout << "\n";

        // 2) Interfata text
        std::cout << "Type `help` for commands.\n";
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
                    "  help                 -> this help\n"
                    "  status               -> show inventory, services, bays, cash, day\n"
                    "  services             -> list services\n"
                    "  bays                 -> list bay status\n"
                    "  book <Service> <k>   -> schedule k cars (Service: Basic/Deluxe/Wax)\n"
                    "  shop                 -> buy water/shampoo/wax with cash\n"
                    "  endday               -> finish the current day; next day starts (bays reset)\n"
                    "  endrun               -> quit program and show final state\n";
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
            else if (cmd == "book") {
                std::string service; int k;
                if (!(iss >> service >> k)) {
                    std::cout << "Usage: book <Service> <k>\n";
                    continue;
                }
                try {
                    int booked = cw.book(service, k);
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

                double cost = w*0.02 + s*0.03 + x*0.05; // aceleasi preturi ca in CarWash::quotePurchaseCost
                std::cout << "Cost: " << std::fixed << std::setprecision(2) << cost << " EUR. Confirm (y/n)? ";
                if (!std::getline(std::cin, tmp)) break;
                if (!tmp.empty() && (tmp[0]=='y' || tmp[0]=='Y')) {
                    // folosim metoda CarWash (care scade cash si mareste stocul)
                    // pentru a respecta incapsularea
                    // (quote + buy pentru validari)
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
            else if (cmd == "endday") {
                cw.endDay();
                std::cout << "New day started. Day = " << cw.getDay() << ". Bays reset to opening time.\n";
            }
            else if (cmd == "endrun") {
                break;
            }
            else {
                std::cout << "Unknown command. Type `help`.\n";
            }
        }

        // 3) Final
        std::cout << "\n=== FINAL STATE ===\n";
        std::cout << "Inventory: " << cw.getInventory() << "\n";
        std::cout << "Bays:\n";
        for (const auto& b : cw.getBays()) std::cout << "  " << b << "\n";
        std::cout << "Total Cash: " << std::fixed << std::setprecision(2) << cw.getCash() << " EUR\n";

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
