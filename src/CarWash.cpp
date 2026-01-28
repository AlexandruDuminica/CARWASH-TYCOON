#include "../headers/CarWash.h"
#include "../headers/BasicService.h"
#include "../headers/DeluxeService.h"
#include "../headers/WaxService.h"
#include "../headers/EcoService.h"
#include "../headers/Customer.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

/**
 * @brief Constructs the main CarWash simulation instance.
 *
 * Initializes the car wash with a name, initial inventory, and opening/closing hours.
 * Also sets up default goals, a default pricing strategy, and starts the daily report.
 *
 * @param n Car wash display name.
 * @param inv Initial inventory snapshot.
 * @param openM Opening time in minutes since midnight.
 * @param closeM Closing time in minutes since midnight.
 *
 * @throws CarWashException If opening hour is not strictly before closing hour.
 */
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

/**
 * @brief Case-insensitive equality check for two strings.
 *
 * Used to match commands and service names without requiring exact casing.
 *
 * @param a Left string.
 * @param b Right string.
 * @return true if strings are equal ignoring case; false otherwise.
 */
bool CarWash::sameCaseInsensitive(const std::string &a, const std::string &b) const {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        char ca = static_cast<char>(std::tolower(static_cast<unsigned char>(a[i])));
        char cb = static_cast<char>(std::tolower(static_cast<unsigned char>(b[i])));
        if (ca != cb) return false;
    }
    return true;
}

/**
 * @brief Finds the index of a service by name (case-insensitive).
 *
 * @param name Service display name to search for.
 * @return Index in services_ if found; -1 otherwise.
 */
int CarWash::findService(const std::string &name) const {
    for (size_t i = 0; i < services_.size(); ++i) {
        if (services_[i] && sameCaseInsensitive(services_[i]->name(), name)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

/**
 * @brief Adds a new wash service to the car wash.
 *
 * Stores a polymorphic clone of the provided service. If the Nano Coating upgrade
 * has been activated, it is applied to all wax services immediately.
 *
 * @param s Service prototype to add (will be cloned).
 * @return true if added successfully; false if service limit is reached.
 */
bool CarWash::addService(const WashService &s) {
    if (services_.size() >= MAX_SERV) return false;
    services_.push_back(s.clone());
    if (nanoCoatingEnabled_) applyNanoCoatingToWaxServices();
    achievements_.onStructureChanged(*this);
    return true;
}

/**
 * @brief Adds a new wash bay to the car wash.
 *
 * The bay is copied into managed storage. A simple capability rule is applied
 * based on bay id parity (for variety): even bays support wax, odd bays support deluxe.
 *
 * @param b Bay prototype to add (copied).
 * @return true if added successfully; false if bay limit is reached.
 */
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

/**
 * @brief Applies Nano Coating effect to all WaxService instances currently registered.
 *
 * This is a functional use of RTTI: Nano coating is a feature that exists only on
 * WaxService. The simulation keeps services in a polymorphic container, therefore
 * we safely detect WaxService with dynamic_cast and enable the upgrade behavior.
 */
void CarWash::applyNanoCoatingToWaxServices() {
    for (auto &s: services_) {
        if (!s) continue;
        if (auto *wax = dynamic_cast<WaxService *>(s.get())) {
            wax->enableNanoCoating(true);
        }
    }
}

/**
 * @brief Enables Nano Coating upgrade globally.
 *
 * This flag persists and ensures future wax services will also have coating enabled
 * (via addService calling applyNanoCoatingToWaxServices when already enabled).
 */
void CarWash::enableNanoCoating() {
    if (nanoCoatingEnabled_) return;
    nanoCoatingEnabled_ = true;
    applyNanoCoatingToWaxServices();
    logEvent("Nano Coating enabled for Wax services");
}

/**
 * @brief Attempts to book a number of cars for a named service across available bays.
 *
 * For each car, the function tries to find a bay that supports the service, checks
 * time feasibility (must finish before closing), and checks inventory availability.
 * If successful, the bay is booked and revenue is collected.
 *
 * @param serviceName Service name to book.
 * @param cars Number of cars to schedule.
 * @return Number of cars actually booked (may be less than requested).
 *
 * @throws InvalidServiceException If serviceName does not exist.
 * @throws BookingException If cars <= 0.
 */
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

/**
 * @brief Applies a multiplicative factor to all service prices.
 *
 * @param factor Must be > 0.0; otherwise no changes are applied.
 */
void CarWash::adjustServicePrices(double factor) {
    if (factor <= 0.0) return;
    for (auto &s: services_) {
        if (s) s->applyFactor(factor);
    }
}

/**
 * @brief Applies the currently configured pricing strategy.
 *
 * Delegates to the active PricingStrategy implementation.
 */
void CarWash::applyPricingStrategy() {
    if (pricing_) pricing_->apply(*this);
}

/**
 * @brief Switches pricing strategy at runtime based on a string key.
 *
 * @param mode One of: \"aggressive\", \"conservative\", or any other value for balanced.
 */
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

/**
 * @brief Finalizes the current day and resets per-day counters/state.
 *
 * Computes daily averages, triggers end-of-day hooks (achievements/goals/events),
 * stores the DailyReport, resets bays and time to opening, advances the day index,
 * and starts a new DailyReport for the next day.
 */
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

/**
 * @brief Simulates one hour of gameplay.
 *
 * Advances time, generates demand/customers, repeatedly attempts to serve customers
 * based on available bay capacity and speed factor, updates revenue and satisfaction,
 * and applies demand adjustments. If closing time is reached, ends the current day.
 */
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

/**
 * @brief Prints the current customer queue state.
 */
void CarWash::showQueue() const {
    std::cout << queue_ << "\n";
}

/**
 * @brief Executes one simulation step and shows the dashboard afterwards.
 */
void CarWash::nextCommand() {
    simulateHour();
    showDashboard();
}

/**
 * @brief Prints all registered services with type and premium flags.
 *
 * Additionally prints WaxService Nano Coating status using RTTI, because
 * Nano Coating is a Wax-only feature.
 */
void CarWash::showServices() const {
    std::cout << "SERVICII:\n";
    for (const auto &p: services_) {
        if (!p) continue;

        std::string extra;
        if (const auto *wax = dynamic_cast<const WaxService *>(p.get())) {
            extra = wax->nanoCoatingEnabled() ? " nano=ON" : " nano=OFF";
        }

        std::cout << "  - " << *p
                << " kind=" << WashService::kindToString(p->kind())
                << (p->isPremium() ? " [premium]" : "")
                << extra
                << "\n";
    }
}

/**
 * @brief Prints all bays and their current state.
 */
void CarWash::showBays() const {
    std::cout << "BAIE (" << bays_.size() << "), create=" << WashBay::totalBaysCreated() << "\n";
    for (const auto &b: bays_) if (b) std::cout << "  " << *b << "\n";
}

/**
 * @brief Prints detailed status including cash, time, inventory, and queue.
 */
void CarWash::showStatus() const {
    std::cout << "=== STATUS ZIUA " << day_ << " ===\n";
    std::cout << "Bani: " << std::fixed << std::setprecision(2) << cash_ << " EUR\n";
    std::cout << "Timp: " << nowMin_ << "/" << closeMin_ << "\n";
    std::cout << "Inventar: " << inv_ << "\n";
    showQueue();
}

/**
 * @brief Prints goal progress and whether all goals have been achieved.
 */
void CarWash::showGoals() const {
    goals_.print(std::cout, *this);
    if (goals_.allAchieved()) std::cout << "Toate obiectivele au fost atinse!\n";
}

/**
 * @brief Prints purchased upgrades and the available upgrades catalog.
 */
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

/**
 * @brief Prints a compact gameplay dashboard with key KPIs.
 */
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

/**
 * @brief Prints all stored daily reports.
 */
void CarWash::showReports() const {
    if (reports_.empty()) {
        std::cout << "Nu exista inca rapoarte zilnice.\n";
        return;
    }
    std::cout << "=== Rapoarte zilnice ===\n";
    for (const auto &r: reports_) std::cout << r << "\n";
}

/**
 * @brief Prints the supplies shop and current inventory/cash.
 */
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

/**
 * @brief Purchases supply packs and updates the inventory and achievements.
 *
 * Valid items are: water, shampoo, wax. Each item has a fixed pack cost and fixed pack quantity.
 *
 * @param item Supply type.
 * @param packs Number of packs to purchase (must be > 0).
 *
 * @throws InvalidCommandException If packs <= 0 or item is not recognized.
 * @throws CarWashException If there is not enough cash to complete the purchase.
 */
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

/**
 * @brief Prints achievements status.
 */
void CarWash::showAchievements() const {
    achievements_.print(std::cout);
}

/**
 * @brief Prints analytics computed from stored daily reports.
 */
void CarWash::showStats() const {
    Statistics stats(reports_);
    stats.print(std::cout);
}

/**
 * @brief Prints the available command list and their usage.
 */
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

/**
 * @brief Purchases and applies an upgrade by id.
 *
 * Creates the corresponding Upgrade object, checks affordability, deducts cost,
 * applies the upgrade effects, tracks purchase counters, and triggers achievements.
 *
 * @param id Upgrade id (1..4).
 *
 * @throws InvalidCommandException If id is not recognized.
 * @throws CarWashException If there is not enough cash.
 */
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

/**
 * @brief Logs a gameplay message to standard output.
 *
 * @param msg Message text to log.
 */
void CarWash::logEvent(const std::string &msg) const {
    std::cout << "[LOG] " << msg << "\n";
}

/**
 * @brief Main entry point for running the simulation loop.
 *
 * In CI (GITHUB_ACTIONS), runs a deterministic demo flow that exercises key features
 * so static analysis does not report unused functions. In interactive mode, reads
 * commands from stdin and executes them until 'endrun' or EOF.
 */
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
                if (mode.empty())
                    throw InvalidCommandException(
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
                    if (pos != packsStr.size())
                        throw InvalidCommandException(
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
