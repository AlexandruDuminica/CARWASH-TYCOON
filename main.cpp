#include <iostream>

#include "CarWash.h"
#include "BasicService.h"
#include "DeluxeService.h"
#include "WaxService.h"
#include "EcoService.h"

int main() {
    try {
        const int OPEN  = 8 * 60;
        const int CLOSE = 12 * 60;

        Inventory inv(3000, 2000, 1500);
        CarWash game("CarWash TYCOON", inv, OPEN, CLOSE);

        BasicService  s1("Basic",  20,  8.0,  80, 40,  0);
        DeluxeService s2("Deluxe", 35, 14.5, 120, 60,  0);
        WaxService    s3("Wax",    25, 16.0,  60, 20, 50);
        EcoService    s4("Eco",    22,  9.0,  50, 30,  0); // clasa derivată nouă

        game.addService(s1);
        game.addService(s2);
        game.addService(s3);
        game.addService(s4);

        WashBay b1(1, OPEN,      "B1");
        WashBay b2(2, OPEN + 10, "B2");
        WashBay b3(3, OPEN + 5,  "B3");
        game.addBay(b1);
        game.addBay(b2);
        game.addBay(b3);

        game.run();
    } catch (const CarWashException& ex) {
        std::cerr << "Eroare critica la pornire: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
