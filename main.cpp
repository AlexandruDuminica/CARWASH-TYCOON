#include <exception>
#include <iostream>

#include "headers/CarWash.h"
#include "headers/CarWashExceptions.h"
#include "headers/Inventory.h"
#include "headers/ServiceFactory.h"
#include "headers/WashBay.h"

int main() {
    try {
        const int OPEN = 8 * 60;
        const int CLOSE = 12 * 60;

        Inventory inv(3000, 2000, 1500);
        std::cout << "Initial inventory: " << inv << "\n";

        CarWash game("CarWash TYCOON", inv, OPEN, CLOSE);

        auto basicDefault = ServiceFactory::create("basic");
        auto basicCfg = ServiceFactory::createConfigured(
            ServiceFactory::Kind::Basic, "Basic", 20, 8.0, 80, 40, 0);

        auto deluxeCfg = ServiceFactory::createConfigured(
            ServiceFactory::Kind::Deluxe, "Deluxe", 35, 14.5, 120, 60, 0);

        auto waxCfg = ServiceFactory::createConfigured(
            ServiceFactory::Kind::Wax, "Wax", 25, 16.0, 60, 20, 50);

        auto ecoCfg = ServiceFactory::createConfigured(
            ServiceFactory::Kind::Eco, "Eco", 30, 12.0, 50, 30, 0);

        if (!basicDefault || !basicCfg || !deluxeCfg || !waxCfg || !ecoCfg) {
            throw CarWashException("ServiceFactory a returnat nullptr");
        }

        game.addService(*basicDefault);
        game.addService(*basicCfg);
        game.addService(*deluxeCfg);
        game.addService(*waxCfg);
        game.addService(*ecoCfg);

        WashBay b1(1, OPEN, "B1");
        WashBay b2(2, OPEN + 10, "B2");
        WashBay b3(3, OPEN + 5, "B3");

        game.addBay(b1);
        game.addBay(b2);
        game.addBay(b3);

#ifdef GITHUB_ACTIONS
        game.run();
        return 0;
#else
        game.run();
        return 0;
#endif
    } catch (const CarWashException &ex) {
        std::cerr << "Eroare CarWash: " << ex.what() << "\n";
        return 1;
    } catch (const std::exception &ex) {
        std::cerr << "Eroare neasteptata: " << ex.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Eroare necunoscuta.\n";
        return 1;
    }
}
