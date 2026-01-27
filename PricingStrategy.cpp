#include "PricingStrategy.h"
#include "CarWash.h"

void AggressivePricing::apply(CarWash &wash) {
    if (wash.currentDemand() < 3 || wash.averageSatisfaction() < 3.5) {
        wash.adjustServicePrices(0.95); // -5%
        wash.logEvent("AggressivePricing: reducere preturi -5%");
    }
}

void BalancedPricing::apply(CarWash &wash) {
    wash.logEvent("BalancedPricing: fara ajustare preturi");
}

void ConservativePricing::apply(CarWash &wash) {
    if (wash.currentDemand() > 4 && wash.averageSatisfaction() > 4.0) {
        wash.adjustServicePrices(1.05); // +5%
        wash.logEvent("ConservativePricing: crestere preturi +5%");
    }
}