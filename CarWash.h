#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Inventory.h"
#include "WashBay.h"
#include "WashService.h"
#include "CarWashExceptions.h"
#include "DeluxeService.h"

class CarWash {
    std::string name_;
    Inventory   inv_;

    std::vector<std::unique_ptr<WashService>> services_;
    std::vector<std::unique_ptr<WashBay>>     bays_;

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

    static constexpr int MAX_SERV = 8;
    static constexpr int MAX_BAYS = 8;

    double r2(double v) const;
    bool geMoney(double a, double b) const;
    bool gtMoney(double a, double b) const;

    bool sameCaseInsensitive(const std::string& a, const std::string& b) const;
    int  findService(const std::string& name) const;

    static void trim(std::string& s);
    static int  toIntOrZero(const std::string& t);

public:
    CarWash(std::string n, Inventory inv, int openM, int closeM);

    bool addService(const WashService& s);
    bool addBay(const WashBay& b);

    int  bookCars(const std::string& serviceName, int cars);

    void priceAll(double factor);

    double priceW() const noexcept { return priceW_; }
    double priceS() const noexcept { return priceS_; }
    double priceX() const noexcept { return priceX_; }

    double quoteBuy(int w, int s, int x) const;

    bool buy(int w, int s, int x);

    double costMinPerBay() const noexcept { return costPerMinPerBay_; }

    bool addMinutes(int m);

    double costDeluxe() const noexcept { return costDeluxe_; }
    double costWax() const noexcept { return costWax_; }

    bool upgradeBay(int id, const std::string& kind);

    void endDay();

    int maxBuyW() const;
    int maxBuyS() const;
    int maxBuyX() const;
    int maxAddMin() const;

    void showServices() const;
    void showBays() const;
    void showStatus() const;
    void showHelp() const;

    void run();
};
