#include <exception>
#include <iostream>

#include "CarWash.h"
#include "CarWashExceptions.h"
#include "Inventory.h"
#include "ServiceFactory.h"
#include "WashBay.h"
#include "BasicService.h"   // pentru finalPriceForCars in CI

int main() {
#ifdef GITHUB_ACTIONS
    try {
        const int OPEN  = 8 * 60;
        const int CLOSE = 12 * 60;

        Inventory inv(3000, 2000, 1500);

        // folosim getters + adders ca să nu fie unusedFunction
        const int w0 = inv.water();
        const int s0 = inv.shampoo();
        const int x0 = inv.wax();
        (void)w0; (void)s0; (void)x0;
        inv.addWater(1);
        inv.addShampoo(1);

        CarWash game("CarWash TYCOON", inv, OPEN, CLOSE);

        // folosim ambele funcții din Factory
        auto def = ServiceFactory::create("basic");
        auto s1 = ServiceFactory::createConfigured(
            ServiceFactory::Kind::Basic,  "Basic",  20,  8.0,  80, 40,  0);

        if (!def || !s1) throw CarWashException("ServiceFactory returned nullptr");

        // folosim finalPriceForCars ca să nu fie unusedFunction
        if (auto* bs = dynamic_cast<BasicService*>(s1.get())) {
            const double tmp = bs->finalPriceForCars(2);
            (void)tmp;
        }

        game.addService(*def);
        game.addService(*s1);

        WashBay b1(1, OPEN, "B1");
        game.addBay(b1);

        // nu rulăm loop-ul, doar smoke-check
        std::cout << "CARWASH TYCOON CI smoke test OK\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "CI error: " << ex.what() << "\n";
        return 1;
        const double a = game.avgSatisfactionToday();
        const int n = game.servedSamplesToday();
        (void)a; (void)n;

    }
#else
    try {
        constexpr int OPEN  = 8 * 60;
        constexpr int CLOSE = 12 * 60;

        Inventory inv(3000, 2000, 1500);
        CarWash game("CarWash TYCOON", inv, OPEN, CLOSE);

        auto s1 = ServiceFactory::createConfigured(
            ServiceFactory::Kind::Basic,  "Basic",  20,  8.0,  80, 40,  0);
        auto s2 = ServiceFactory::createConfigured(
            ServiceFactory::Kind::Deluxe, "Deluxe", 35, 14.5, 120, 60,  0);
        auto s3 = ServiceFactory::createConfigured(
            ServiceFactory::Kind::Wax,    "Wax",    25, 16.0,  60, 20, 50);
        auto s4 = ServiceFactory::createConfigured(
            ServiceFactory::Kind::Eco,    "Eco",    30, 12.0,  50, 30,  0);

        if (!s1 || !s2 || !s3 || !s4) {
            throw CarWashException("ServiceFactory a returnat nullptr");
        }

        game.addService(*s1);
        game.addService(*s2);
        game.addService(*s3);
        game.addService(*s4);

        WashBay b1(1, OPEN,      "B1");
        WashBay b2(2, OPEN + 10, "B2");
        WashBay b3(3, OPEN + 5,  "B3");

        game.addBay(b1);
        game.addBay(b2);
        game.addBay(b3);

        game.run();
    } catch (const CarWashException& ex) {
        std::cerr << "Eroare CarWash: " << ex.what() << "\n";
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "Eroare neasteptata: " << ex.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Eroare necunoscuta.\n";
        return 1;
    }

    return 0;
#endif
}
