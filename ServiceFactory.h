#pragma once
#include <memory>
#include <string>
#include "WashService.h"

class ServiceFactory {
public:
    static std::unique_ptr<WashService> create(const std::string& key);
};
