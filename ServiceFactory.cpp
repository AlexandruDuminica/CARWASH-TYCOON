#include "ServiceFactory.h"
#include "BasicService.h"
#include "DeluxeService.h"
#include "WaxService.h"
#include "EcoService.h"
#include "CarWashExceptions.h"

std::unique_ptr<WashService> ServiceFactory::create(const std::string& key) {
    if (key == "basic")  return std::make_unique<BasicService>();
    if (key == "deluxe") return std::make_unique<DeluxeService>();
    if (key == "wax")    return std::make_unique<WaxService>();
    if (key == "eco")    return std::make_unique<EcoService>();
    throw InvalidServiceException("ServiceFactory: unknown key: " + key);
}
