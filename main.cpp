#include <exception>
#include <iostream>

#include "CarWash.h"
#include "CarWashExceptions.h"
#include "Inventory.h"
#include "ServiceFactory.h"
#include "WashBay.h"

int main() {
#ifdef GITHUB_ACTIONS
    std::cout << "CARWASH TYCOON MSan smoke test\n";
    return 0;
#else
    try {
        const int OPEN  = 8 * 60;
        const int CLOSE = 12 * 60;

        Inventory inv(3000, 2000, 1500);
        CarWash game("CarWash TYCOON", inv, OPEN, CLOSE);

        // Factory pattern: construim serviciile prin ServiceFactory
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
