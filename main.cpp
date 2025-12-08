#include <iostream>

#include "CarWash.h"
#include "BasicService.h"
#include "DeluxeService.h"
#include "WaxService.h"
#include "EcoService.h"

int main() {
    const int OPEN  = 8 * 60;
    const int CLOSE = 18 * 60;

    Inventory inv(5000, 3000, 2000);
    CarWash game("CarWashSIM", inv, OPEN, CLOSE);

    game.addService(BasicService("Basic",  20,  8.5,  80, 40,  0));
    game.addService(DeluxeService("Deluxe",35, 14.5, 120, 60,  0));
    game.addService(WaxService("Wax",      25, 16.0,  60, 20, 50));
    game.addService(EcoService("Eco",      22,  9.0,  50, 30,  0));

    game.addBay(WashBay(1, OPEN,      "B1"));
    game.addBay(WashBay(2, OPEN + 10, "B2"));
    game.addBay(WashBay(3, OPEN + 5,  "B3"));

    try {
        game.run();
    } catch (const CarWashException &ex) {
        std::cerr << "Eroare critica: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
