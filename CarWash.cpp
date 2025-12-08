#include "CarWash.h"
#include "CarWashExceptions.h"
#include <algorithm>
#include <iomanip>
#include <cctype>

CarWash::CarWash(std::string n, Inventory inv, int openM, int closeM)
        : name_(std::move(n)), inv_(inv),
          openMin_(openM), closeMin_(closeM), nowMin_(openM)
{
    if(openM >= closeM)
        throw CarWashException("Ore program invalide");
}

bool CarWash::sameCaseInsensitive(const std::string& a,const std::string& b) const {
    if(a.size()!=b.size()) return false;
    for(size_t i=0;i<a.size();i++)
        if(std::tolower(a[i])!=std::tolower(b[i])) return false;
    return true;
}

int CarWash::findService(const std::string& name) const {
    for(size_t i=0; i<services_.size(); i++){
        if(sameCaseInsensitive(services_[i]->name(), name))
            return int(i);
    }
    return -1;
}

bool CarWash::addService(const WashService& s){
    if(services_.size()>=MAX_SERV) return false;
    services_.push_back(s.clone());
    return true;
}

bool CarWash::addBay(const WashBay& b){
    if(bays_.size()>=MAX_BAYS) return false;
    bays_.push_back(std::make_unique<WashBay>(b));
    return true;
}

void CarWash::simulateHour(){
    nowMin_ += 60;

    std::vector<std::string> names;
    names.reserve(services_.size());
    for(auto &s : services_) names.push_back(s->name());

    queue_.generate(names);

    int processed = 0;
    for(size_t r = 0; r < bays_.size()*3; r++){
        if(queue_.empty()) break;

        std::string svc = queue_.get();
        int si = findService(svc);
        if(si < 0){
            queue_.fail();
            demand_.fail();
            continue;
        }
        auto &sp = *services_[si];

        bool ok = false;
        for(auto &bay : bays_){
            if(!bay->canDo(sp.name())) continue;
            int finish = bay->availAt() + sp.duration();
            if(finish <= closeMin_ &&
               inv_.takeIfCan(sp,1))
            {
                bay->bookOne(sp, bay->availAt());
                cash_ += sp.price();
                ok = true;
                processed++;
                demand_.success();
                break;
            }
        }
        if(!ok){
            queue_.fail();
            demand_.fail();
        }
    }

    int adj = demand_.adjust();
    if(adj > 0) queue_.increaseDemand();
    if(adj < 0) queue_.decreaseDemand();
    if(adj!=0) demand_.reset();

    if(nowMin_ >= closeMin_){
        for(auto &b : bays_)
            b->reset(openMin_);
        nowMin_ = openMin_;
        day_++;
    }

    std::cout<<"Ora simulata: procesate="<<processed
             <<" ramase="<<queue_.size()
             <<" pierduti="<<queue_.lostCustomers()
             <<" cerere="<<queue_.currentDemand()
             <<"\n";
}

void CarWash::showQueue() const {
    std::cout<<"Masini in coada="<<queue_.size()
             <<" | Pierduti="<<queue_.lostCustomers()
             <<" | Cerere/h="<<queue_.currentDemand()<<"\n";
}

void CarWash::nextCommand(){
    simulateHour();
}

void CarWash::showServices() const {
    std::cout<<"SERVICII:\n";
    for(auto &s:services_){
        std::cout<<"  "<<*s<<"\n";
    }
}

void CarWash::showBays() const {
    std::cout<<"BAIE:\n";
    for(auto &b : bays_)
        std::cout<<"  "<<*b<<"\n";
}

void CarWash::showStatus() const {
    std::cout<<"=== STATUS ZIUA "<<day_<<" ===\n";
    std::cout<<"Bani: "<<std::fixed<<std::setprecision(2)<<cash_<<" EUR\n";
    std::cout<<"Inventar: "<<inv_<<"\n";
    showQueue();
}

void CarWash::showHelp() const {
    std::cout<<"Comenzi:\n"
             <<" status\n"
             <<" services\n"
             <<" bays\n"
             <<" queue\n"
             <<" next\n"
             <<" endrun\n";
}

void CarWash::run(){
    std::cout<<"=== SIMULARE CARWASH ACTIVATA ===\n";
    showHelp();
    std::string cmd;
    while(true){
        std::cout<<"> ";
        if(!std::getline(std::cin, cmd)) break;
        if(cmd=="status") showStatus();
        else if(cmd=="services") showServices();
        else if(cmd=="bays") showBays();
        else if(cmd=="queue") showQueue();
        else if(cmd=="next") nextCommand();
        else if(cmd=="endrun") break;
        else std::cout<<"Comanda necunoscuta\n";
    }
}
