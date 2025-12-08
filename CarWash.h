#pragma once
#include <memory>
#include <string>
#include <vector>
#include <iosfwd>

#include "Inventory.h"
#include "WashBay.h"
#include "WashService.h"
#include "CarQueue.h"
#include "DemandManager.h"
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
    int nowMin_;
    int day_{1};

    CarQueue queue_;
    DemandManager demand_;

    static constexpr int MAX_SERV = 20;
    static constexpr int MAX_BAYS = 20;

    bool sameCaseInsensitive(const std::string &a,
                             const std::string &b) const;
    int  findService(const std::string &name) const;

public:
    CarWash(std::string n, Inventory inv, int openM, int closeM);

    bool addService(const WashService &s);
    bool addBay(const WashBay &b);

    int bookCars(const std::string &serviceName, int cars);
    void priceAll(double f);

    void simulateHour();
    void showQueue() const;
    void nextCommand();

    void showServices() const;
    void showBays() const;
    void showStatus() const;
    void showHelp() const;

    void run();
};
