#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#define MAX_SERV 8
#define MAX_BAYS 8

class ServicePackage {
    std::string name_;
    int dur_;
    double price_;
    int wL_;
    int sML_;
    int xML_;
public:
    ServicePackage(std::string n, int d, double p, int w, int s, int x)
        : name_(std::move(n)), dur_(d), price_(p), wL_(w), sML_(s), xML_(x) {}

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] int    duration()  const noexcept { return dur_; }
    [[nodiscard]] double price()     const noexcept { return price_; }
    [[nodiscard]] int    needW()     const noexcept { return wL_; }
    [[nodiscard]] int    needS()     const noexcept { return sML_; }
    [[nodiscard]] int    needX()     const noexcept { return xML_; }

    bool applyFactor(double f) { if (f <= 0.0) return false; price_ *= f; return true; }

    friend std::ostream& operator<<(std::ostream& os, const ServicePackage& sp) {
        os << "Service{name='" << sp.name() << "', t=" << sp.duration()
           << "min, price=" << std::fixed << std::setprecision(2) << sp.price()
           << " EUR, use=[" << sp.needW() << "L, " << sp.needS()
           << "ml, " << sp.needX() << "ml]}";
        return os;
    }
};

class Inventory {
    int wL_;
    int sML_;
    int xML_;
public:
    Inventory(int w, int s, int x) : wL_(w), sML_(s), xML_(x) {}

    void add(int w, int s, int x) {
        if (w > 0) wL_ += w;
        if (s > 0) sML_ += s;
        if (x > 0) xML_ += x;
    }

    bool takeIfCan(const ServicePackage& sp, int cars) {
        long W = 1L * sp.needW() * cars;
        long S = 1L * sp.needS() * cars;
        long X = 1L * sp.needX() * cars;
        if (W <= wL_ && S <= sML_ && X <= xML_) {
            wL_ -= int(W); sML_ -= int(S); xML_ -= int(X);
            return true;
        }
        return false;
    }

    [[nodiscard]] int w() const noexcept { return wL_; }
    [[nodiscard]] int s() const noexcept { return sML_; }
    [[nodiscard]] int x() const noexcept { return xML_; }

    [[nodiscard]] double fullness() const {
        const double kW = 5000.0, kS = 5000.0, kX = 5000.0;
        return (wL_/kW + sML_/kS + xML_/kX) / 3.0 * 100.0;
    }

    friend std::ostream& operator<<(std::ostream& os, const Inventory& inv) {
        os << "Inventory{water=" << inv.w() << "L, shampoo=" << inv.s()
           << "ml, wax=" << inv.x() << "ml, full=" << std::fixed
           << std::setprecision(1) << inv.fullness() << "%}";
        return os;
    }
};

class WashBay {
    int   id_;
    int   availMin_;
    char* label_;
    bool  canBasic_{true};
    bool  canDeluxe_{false};
    bool  canWax_{false};

    char* dup(const std::string& s) {
        char* p = new char[s.size() + 1];
        std::copy(s.c_str(), s.c_str() + s.size() + 1, p);
        return p;
    }
public:
    WashBay(int id, int startMin, const std::string& text)
        : id_(id), availMin_(startMin), label_(dup(text)) {}

    WashBay(const WashBay& o)
        : id_(o.id_), availMin_(o.availMin_), label_(dup(o.label_)),
          canBasic_(o.canBasic_), canDeluxe_(o.canDeluxe_), canWax_(o.canWax_) {}

    WashBay& operator=(const WashBay& o) {
        if (this != &o) {
            id_ = o.id_; availMin_ = o.availMin_;
            delete[] label_; label_ = dup(o.label_);
            canBasic_  = o.canBasic_;
            canDeluxe_ = o.canDeluxe_;
            canWax_    = o.canWax_;
        }
        return *this;
    }

    ~WashBay() { delete[] label_; }

    [[nodiscard]] int  id()      const noexcept { return id_; }
    [[nodiscard]] int  availAt() const noexcept { return availMin_; }
    [[nodiscard]] bool hasBasic()  const noexcept { return canBasic_; }
    [[nodiscard]] bool hasDeluxe() const noexcept { return canDeluxe_; }
    [[nodiscard]] bool hasWax()    const noexcept { return canWax_; }

    void addDeluxe() noexcept { canDeluxe_ = true; }
    void addWax()    noexcept { canWax_    = true; }

    bool canDo(const std::string& n) const {
        if (n=="Basic"  || n=="basic")  return canBasic_;
        if (n=="Deluxe" || n=="deluxe") return canDeluxe_;
        if (n=="Wax"    || n=="wax")    return canWax_;
        return false;
    }

    int bookOne(const ServicePackage& sp, int earliest) {
        int start  = (earliest > availMin_) ? earliest : availMin_;
        int finish = start + sp.duration();
        availMin_  = finish;
        return finish;
    }

    void reset(int openMin) { availMin_ = openMin; }

    friend std::ostream& operator<<(std::ostream& os, const WashBay& b) {
        os << "Bay #" << b.id() << " (at=" << b.availAt() << "m, [";
        bool first = true;
        if (b.hasBasic())  { os << "Basic";  first = false; }
        if (b.hasDeluxe()) { os << (first ? "" : ",") << "Deluxe"; first = false; }
        if (b.hasWax())    { os << (first ? "" : ",") << "Wax"; }
        os << "])";
        return os;
    }
};

class CarWash {
    std::string name_;
    Inventory   inv_;

    ServicePackage* serv_[MAX_SERV];
    int nServ_{0};

    WashBay* bays_[MAX_BAYS];
    int nBays_{0};

    double cash_{0.0};
    int openMin_;
    int closeMin_;
    int day_{1};

    double priceW_{0.02};
    double priceS_{0.03};
    double priceX_{0.05};
    double costPerMinPerBay_{0.50};
    double costDeluxe_{200.0};
    double costWax_{150.0};

    double r2(double v) const { return std::round(v * 100.0) / 100.0; }
    bool geMoney(double a, double b) const { return r2(a) >= r2(b); }
    bool gtMoney(double a, double b) const { return r2(a) >  r2(b); }

    bool sameCaseInsensitive(const std::string& a, const std::string& b) const {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            char ca = std::tolower((unsigned char)a[i]);
            char cb = std::tolower((unsigned char)b[i]);
            if (ca != cb) return false;
        }
        return true;
    }

    int findService(const std::string& name) const {
        for (int i = 0; i < nServ_; ++i) {
            if (serv_[i] && sameCaseInsensitive(serv_[i]->name(), name)) return i;
        }
        return -1;
    }

    void trim(std::string& s) {
        while (!s.empty() && std::isspace((unsigned char)s.back())) { s.pop_back(); }
        size_t i = 0;
        while (i < s.size() && std::isspace((unsigned char)s[i])) { ++i; }
        s.erase(0, i);
    }

    int toIntOrZero(const std::string& t) {
        if (t.empty()) return 0;
        char* e = nullptr; long v = std::strtol(t.c_str(), &e, 10);
        if (e == t.c_str() || *e != '\0') return 0;
        if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) return 0;
        return int(v);
    }

public:
    CarWash(std::string n, Inventory inv, int openM, int closeM)
        : name_(std::move(n)), inv_(inv), openMin_(openM), closeMin_(closeM) {
        for (int i = 0; i < MAX_SERV; ++i) serv_[i] = nullptr;
        for (int i = 0; i < MAX_BAYS; ++i)  bays_[i]  = nullptr;
    }

    ~CarWash() {
        for (int i = 0; i < nServ_; ++i) { delete serv_[i]; serv_[i] = nullptr; }
        for (int i = 0; i < nBays_; ++i)  { delete bays_[i];  bays_[i]  = nullptr; }
    }

    bool addService(const ServicePackage& s) {
        if (nServ_ >= MAX_SERV) return false;
        serv_[nServ_] = new ServicePackage(s);
        ++nServ_;
        return true;
    }

    bool addBay(const WashBay& b) {
        if (nBays_ >= MAX_BAYS) return false;
        bays_[nBays_] = new WashBay(b);
        ++nBays_;
        return true;
    }

    int bookCars(const std::string& serviceName, int cars) {
        int si = findService(serviceName);
        if (si < 0) return 0;
        const ServicePackage& sp = *serv_[si];

        int low = 0, high = cars, ok = 0;
        while (low <= high) {
            int mid = (low + high) / 2;
            Inventory probe = inv_;
            if (probe.takeIfCan(sp, mid)) { ok = mid; low = mid + 1; }
            else { high = mid - 1; }
        }
        if (ok == 0) return 0;
        inv_.takeIfCan(sp, ok);

        int done = 0;
        for (int c = 0; c < ok; ++c) {
            int idx = -1; int bestTime = std::numeric_limits<int>::max() / 2;
            for (int i = 0; i < nBays_; ++i) {
                if (!bays_[i]) continue;
                if (!bays_[i]->canDo(serviceName)) continue;
                int a = bays_[i]->availAt();
                if (a < bestTime) { bestTime = a; idx = i; }
            }
            if (idx == -1) break;
            int finish = bays_[idx]->bookOne(sp, bays_[idx]->availAt());
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

    void priceAll(double f) {
        for (int i = 0; i < nServ_; ++i) if (serv_[i]) serv_[i]->applyFactor(f);
    }

    double priceW() const noexcept { return priceW_; }
    double priceS() const noexcept { return priceS_; }
    double priceX() const noexcept { return priceX_; }

    double quoteBuy(int w, int s, int x) const { return w * priceW_ + s * priceS_ + x * priceX_; }

    bool buy(int w, int s, int x) {
        if (w < 0 || s < 0 || x < 0) return false;
        double c = quoteBuy(w, s, x);
        if (gtMoney(c, cash_)) return false;
        cash_ -= r2(c);
        inv_.add(w, s, x);
        return true;
    }

    double costMinPerBay() const noexcept { return costPerMinPerBay_; }

    bool addMinutes(int m) {
        if (m <= 0) return false;
        double unit = costPerMinPerBay_ * double(nBays_ > 0 ? nBays_ : 1);
        double cost = m * unit;
        if (gtMoney(cost, cash_)) return false;
        cash_ -= r2(cost);
        closeMin_ += m;
        return true;
    }

    double costDeluxe() const noexcept { return costDeluxe_; }
    double costWax() const noexcept { return costWax_; }

    bool upgradeBay(int id, const std::string& kind) {
        WashBay* t = nullptr;
        for (int i = 0; i < nBays_; ++i) {
            if (bays_[i] && bays_[i]->id() == id) { t = bays_[i]; break; }
        }
        if (!t) return false;

        if (sameCaseInsensitive(kind, "Deluxe")) {
            if (!geMoney(cash_, costDeluxe_)) return false;
            cash_ -= r2(costDeluxe_);
            t->addDeluxe();
            return true;
        }
        if (sameCaseInsensitive(kind, "Wax")) {
            if (!geMoney(cash_, costWax_)) return false;
            cash_ -= r2(costWax_);
            t->addWax();
            return true;
        }
        return false;
    }

    void endDay() {
        for (int i = 0; i < nBays_; ++i) if (bays_[i]) bays_[i]->reset(openMin_);
        ++day_;
    }

    int maxBuyW() const { return priceW_ > 0.0 ? int(std::floor(r2(cash_) / priceW_)) : 0; }
    int maxBuyS() const { return priceS_ > 0.0 ? int(std::floor(r2(cash_) / priceS_)) : 0; }
    int maxBuyX() const { return priceX_ > 0.0 ? int(std::floor(r2(cash_) / priceX_)) : 0; }
    int maxAddMin() const {
        double u = costPerMinPerBay_ * double(nBays_ > 0 ? nBays_ : 1);
        return u > 0.0 ? int(std::floor(r2(cash_) / u)) : 0;
    }

    void showServices() const {
        std::cout << "Services:\n";
        for (int i = 0; i < nServ_; ++i) if (serv_[i]) std::cout << "  - " << *serv_[i] << "\n";
    }
    void showBays() const {
        std::cout << "Bays (" << nBays_ << "):\n";
        for (int i = 0; i < nBays_; ++i) if (bays_[i]) {
            int rem = closeMin_ - bays_[i]->availAt(); if (rem < 0) rem = 0;
            std::cout << "  " << *bays_[i] << " | remaining=" << rem << "min\n";
        }
    }
    void showStatus() const {
        std::cout << "=== STATUS (Day " << day_ << ") ===\n";
        std::cout << "Inventory: " << inv_ << "\n";
        std::cout << "Cash: " << std::fixed << std::setprecision(2) << cash_ << " EUR\n";
        std::cout << "Hours: open=" << openMin_ << " -> close=" << closeMin_ << " (mins)\n";
        showBays();
    }
    void showHelp() const {
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

    void run() {
        std::cout << "=== INITIAL STATE ===\n"; showStatus(); std::cout << "\n";
        priceAll(1.10);
        std::cout << "--- After 10% weekend price increase ---\n"; showServices(); std::cout << "\n";
        std::cout << "Type `help` for commands.\n";

        std::string line;
        while (true) {
            std::cout << "> ";
            if (!std::getline(std::cin, line)) { break; }
            trim(line);
            if (line.empty()) { continue; }

            std::istringstream iss(line); // <-- fix
            std::string cmd; iss >> cmd;

            if (cmd == "help") {
                showHelp();
            } else if (cmd == "status") {
                showStatus();
            } else if (cmd == "services") {
                showServices();
            } else if (cmd == "bays") {
                showBays();
            } else if (cmd == "bayscount") {
                std::cout << "Total bays: " << nBays_ << "\n";
            } else if (cmd == "book") {
                std::string svc; int k = 0; iss >> svc >> k;
                if (svc.empty() || k <= 0) {
                    std::cout << "Usage: book <Service> <k>\n";
                    continue;
                }
                int got = bookCars(svc, k);
                if (got == 0) { std::cout << "No time/resources/bay.\n"; }
                else { std::cout << "Booked " << got << " cars for " << svc << ".\n"; }
            } else if (cmd == "shop") {
                std::string t; int w=0, s=0, x=0;

                std::cout << "Water L (max " << maxBuyW() << "): ";
                if (!std::getline(std::cin, t)) { break; }
                trim(t); w = toIntOrZero(t);

                std::cout << "Shampoo ml (max " << maxBuyS() << "): ";
                if (!std::getline(std::cin, t)) { break; }
                trim(t); s = toIntOrZero(t);

                std::cout << "Wax ml (max " << maxBuyX() << "): ";
                if (!std::getline(std::cin, t)) { break; }
                trim(t); x = toIntOrZero(t);

                double cost = quoteBuy(w, s, x);
                std::cout << "Cost: " << std::fixed << std::setprecision(2) << cost << " EUR. Confirm (y/n)? ";
                if (!std::getline(std::cin, t)) { break; }
                trim(t);
                if (!t.empty() && (t[0] == 'y' || t[0] == 'Y')) {
                    if (buy(w, s, x)) {
                        std::cout << "Purchased. Cash: " << std::fixed << std::setprecision(2) << cash_ << " EUR\n";
                    } else {
                        std::cout << "Not enough cash.\n";
                    }
                } else {
                    std::cout << "Cancelled.\n";
                }
            } else if (cmd == "upgradehours") {
                int m = 0; iss >> m;
                if (m <= 0) {
                    std::cout << "Usage: upgradehours <positive_minutes> (max " << maxAddMin() << ")\n";
                    continue;
                }
                if (m > maxAddMin()) {
                    std::cout << "You can afford at most " << maxAddMin() << " minutes.\n";
                    continue;
                }
                if (addMinutes(m)) { std::cout << "Closing time extended by " << m << " minutes.\n"; }
                else { std::cout << "Not enough cash.\n"; }
            } else if (cmd == "upgradebay") {
                int id = 0; std::string kind; iss >> id >> kind;
                if (id <= 0 || kind.empty()) {
                    std::cout << "Usage: upgradebay <id> <Deluxe|Wax>\n";
                    continue;
                }
                if (upgradeBay(id, kind)) { std::cout << "Bay " << id << " upgraded: " << kind << ".\n"; }
                else { std::cout << "Upgrade failed.\n"; }
            } else if (cmd == "endday") {
                endDay();
                showStatus();
                std::cout << "\n";
            } else if (cmd == "endrun") {
                break;
            } else {
                std::cout << "Unknown. Type `help`.\n";
            }
        }

        std::cout << "\n=== FINAL STATE ===\n";
        showStatus();
    }
};

int main() {
    const int OPEN  = 8 * 60;
    const int CLOSE = 12 * 60;

    Inventory inv(3000, 2000, 1500);
    CarWash game("ShinyHands", inv, OPEN, CLOSE);

    ServicePackage s1("Basic",  20,  8.0,  80, 40,  0);
    ServicePackage s2("Deluxe", 35, 14.5, 120, 60,  0);
    ServicePackage s3("Wax",    25, 16.0,  60, 20, 50);
    game.addService(s1); game.addService(s2); game.addService(s3);

    WashBay b1(1, OPEN,      "B1");
    WashBay b2(2, OPEN + 10, "B2");
    WashBay b3(3, OPEN + 5,  "B3");
    game.addBay(b1); game.addBay(b2); game.addBay(b3);

    game.run();
    return 0;
}


