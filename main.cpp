// Car Wash Tycoon – POO Tema 1

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <cctype>

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

    // accesori const
    const std::string& getName() const { return name; }
    int getDuration() const { return durationMin; }
    double getPrice() const { return price; }
    int needWater() const { return waterL; }
    int needShampoo() const { return shampooML; }
    int needWax() const { return waxML; }

    // o functie "mai complexa": ajustare dinamica a pretului (ex. weekend surge)
    void applyPriceFactor(double factor) {
        if (factor < 0.1 || factor > 10.0) throw std::invalid_argument("factor out of range");
        price *= factor;
    }

    friend std::ostream& operator<<(std::ostream& os, const ServicePackage& sp) {
        os << "ServicePackage{name='" << sp.name << "', dur=" << sp.durationMin
           << "min, price=" << std::fixed << std::setprecision(2) << sp.price
           << "€, use=[" << sp.waterL << "L water, " << sp.shampooML << "ml shampoo, "
           << sp.waxML << "ml wax]}";
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

    // incarca stoc
    void restock(int w, int s, int x) {
        waterL += std::max(0, w);
        shampooML += std::max(0, s);
        waxML += std::max(0, x);
    }

    // "mai complex": verifica si consuma atomically resurse pentru k masini
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

    // procent "health" stoc (functie calcul)
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
// Are un label alocat dinamic pentru a demonstra Rule of Three.
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

    // constructor de copiere
    WashBay(const WashBay& other)
        : id(other.id), availableAtMin(other.availableAtMin), label(dupCString(other.label)) {}

    // operator= copiere
    WashBay& operator=(const WashBay& other) {
        if (this != &other) {
            id = other.id;
            availableAtMin = other.availableAtMin;
            delete[] label;
            label = dupCString(other.label);
        }
        return *this;
    }

    // destructor
    ~WashBay() { delete[] label; }

    bool isFreeAt(int tMin) const { return tMin >= availableAtMin; }

    // "mai complex": programeaza o masina; intoarce ora finalizarii
    int schedule(const ServicePackage& sp, int startMin) {
        int begin = std::max(startMin, availableAtMin);
        int finish = begin + sp.getDuration();
        availableAtMin = finish;
        return finish;
    }

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
    int closingMin; // program zilnic

    // utilitar privat: cauta serviciu dupa nume case-insensitive
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
    CarWash(std::string nm, Inventory inv, int closeAtMin)
        : name(std::move(nm)), inventory(std::move(inv)), closingMin(closeAtMin) {}

    void addService(const ServicePackage& sp) { services.push_back(sp); }
    void addBay(const WashBay& wb) { bays.push_back(wb); }

    // "mai complex": booking pentru k masini, alegand mereu boxa ce se elibereaza cel mai devreme
    // intoarce numarul efectiv de masini programate (poate fi < k daca se depaseste programul)
    int book(const std::string& serviceName, int cars) {
        int idx = findServiceIndex(serviceName);
        if (idx < 0) throw std::runtime_error("service not found");
        const ServicePackage& sp = services[idx];

        // verifica resursele pentru toate masinile; daca nu sunt, incearca cat se poate
        int low = 0, high = cars, feasible = 0;
        while (low <= high) { // binary search cat putem procesa cu stocul actual
            int mid = (low + high) / 2;
            Inventory probe = inventory;
            if (probe.consumeIfAvailable(sp, mid)) { feasible = mid; low = mid + 1; }
            else { high = mid - 1; }
        }
        if (feasible == 0) return 0;
        // consuma pentru feasible
        inventory.consumeIfAvailable(sp, feasible);

        int scheduled = 0;
        for (int i = 0; i < feasible; ++i) {
            // alege boxa cu availableAt minim
            auto it = std::min_element(bays.begin(), bays.end(),
                [](const WashBay& a, const WashBay& b){ return a.getAvailableAt() < b.getAvailableAt(); });
            int startMin = it->getAvailableAt();
            int finish = it->schedule(sp, startMin);
            if (finish <= closingMin) {
                cash += sp.getPrice();
                scheduled++;
            } else {
                // daca aceasta masina depaseste programul, facem "undo" consum resurse pt masina asta
                inventory.restock(sp.needWater(), sp.needShampoo(), sp.needWax());
                // si rupem bucla
                break;
            }
        }
        return scheduled;
    }

    // raport si incasari
    double getCash() const { return cash; }
    const Inventory& getInventory() const { return inventory; }
    const std::vector<ServicePackage>& getServices() const { return services; }
    const std::vector<WashBay>& getBays() const { return bays; }

    // "mai complex": ruleaza promotii/ajustari pret si recomputa proiectii (ex: weekend)
    void applyGlobalPriceFactor(double factor) {
        for (auto& s : services) s.applyPriceFactor(factor);
    }

    friend std::ostream& operator<<(std::ostream& os, const CarWash& cw) {
        os << "CarWash '" << cw.name << "'\n  " << cw.inventory << "\n  Services:\n";
        for (const auto& s : cw.services) os << "    - " << s << "\n";
        os << "  Bays:\n";
        for (const auto& b : cw.bays) os << "    - " << b << "\n";
        os << "  Cash: " << std::fixed << std::setprecision(2) << cw.cash << "€";
        return os;
    }
};

// ------------------------------------ main ------------------------------------
int main() {
    try {
        // 1) Construim entitatile (constructori cu parametri)
        Inventory inv(/*water*/ 3000, /*shampoo*/ 2000, /*wax*/ 1500);
        CarWash cw("ShinyHands", inv, /*closing at*/ 12 * 60 + 0); // inchidere 12:00

        // servicii
        ServicePackage basic("Basic", 20, 8.0, 80, 40, 0);
        ServicePackage deluxe("Deluxe", 35, 14.5, 120, 60, 0);
        ServicePackage wax("Wax", 25, 16.0, 60, 20, 50);

        // adaugare servicii
        cw.addService(basic);
        cw.addService(deluxe);
        cw.addService(wax);

        // boxe (demonstram rule-of-three pe WashBay prin copiere/atribuire)
        WashBay b1(1, 8 * 60, "Front Left");
        WashBay b2(2, 8 * 60 + 10, "Front Right");
        WashBay b3 = b1; // copy ctor
        b3 = WashBay(3, 8 * 60 + 5, "Rear Center"); // operator=

        cw.addBay(b1);
        cw.addBay(b2);
        cw.addBay(b3);

        std::cout << "=== INITIAL STATE ===\n" << cw << "\n\n";

        // 2) Exemplu: crestere pret weekend (functie non-triviala)
        cw.applyGlobalPriceFactor(1.10);
        std::cout << "--- After 10% weekend price increase ---\n";
        for (const auto& s : cw.getServices()) std::cout << s << "\n";
        std::cout << "\n";

        // 3) Booking-uri citite din stdin (sau tastatura.txt)
        int Q = 0;
        if (!(std::cin >> Q)) {
            std::cout << "(No stdin provided. Running a default demo...)\n";
            Q = 4;
            // fallback: cateva comenzi implicite
            std::vector<std::pair<std::string,int>> demo {
                {"Basic", 3}, {"Deluxe", 2}, {"Wax", 1}, {"Basic", 4}
            };
            for (const auto& e : demo) {
                int booked = cw.book(e.first, e.second);
                std::cout << "Booked " << booked << " cars for " << e.first << ".\n";
            }
        } else {
            for (int i = 0; i < Q; ++i) {
                std::string service;
                int k;
                std::cin >> service >> k;
                int booked = cw.book(service, k);
                std::cout << "Booked " << booked << " cars for " << service << ".\n";
            }
        }

        // 4) Afisare stari finale, folosind operator<< compus
        std::cout << "\n=== FINAL STATE ===\n";
        std::cout << "Inventory: " << cw.getInventory() << "\n";
        std::cout << "Bays:\n";
        for (const auto& b : cw.getBays()) std::cout << "  " << b << "\n";
        std::cout << "Total Cash: " << std::fixed << std::setprecision(2) << cw.getCash() << "€\n";

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
