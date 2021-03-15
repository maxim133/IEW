#include "Excursion.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <nlohmann/json.hpp>

using nlohmann::json;

UserList generate(double x, double y, double w, double h, unsigned int cntUsers, unsigned int cntCities)
{
    UserList users;
    srand(std::time(nullptr));
    UserList cities;
    for (int i = 0; i<cntCities; i++)
    {
        UserData city;
        city.x = (double(rand()%10000)/10000)* w * 0.8 + x + w/10;
        city.y = (double(rand()%10000)/10000)* h * 0.8 + y + h/10;
        cities.push_back(city);
        //std::cout << city.x << "; " << city.y << std::endl;
    }
    for(int i = 0; i < cntUsers; i++)
    {
        UserData user;
        int icity = rand()%(cntCities + 3);
        if (icity < cntCities)
        {
            user.x = (double(rand()%10000)/10000) * w * 0.1 + cities[icity].x - w *0.05;
            user.y = (double(rand()%10000)/10000) * h * 0.1 + cities[icity].y - h *0.05;
        }
        else
        {
            user.x = (double(rand()%10000)/10000) * w + x;
            user.y = (double(rand()%10000)/10000) * w + y;
        }
        //std::cout << user.x << "; " << user.y << std::endl;
        users.push_back(user);
    }
    return users;
}

double dist(UserData l, UserData r)
{
    return std::sqrt((l.x - r.x)*(l.x - r.x) + (l.y - r.y)*((l.y - r.y)));
}

PinList clasterize(const UserList& users, double x1, double x2, double y1, double y2, double scaleFactor)
{
    PinList pins;
    UserList localUsers;
    std::copy_if(users.begin(), users.end(), std::back_inserter(localUsers),
                 [x1,x2,y1,y2](UserData u)
                    {
                        return u.x >= x1 && u.x <= x2 && u.y >= y1 && u.y <=y2;
                    });
    double clusterDist = std::min(x2-x1, y2-y1) / scaleFactor;
    std::vector<bool> processed;
    processed.assign(localUsers.size(), false);
    while(std::count_if(processed.begin(), processed.end(), [](bool b){return !b;}) > 0)
    {
        std::vector<int> neighbours;
        for(UserData u : localUsers)
        {
            int cnt = 0;
            for(int i = 0; i < localUsers.size(); i++)
            {
                if(!processed[i] && dist(u, localUsers[i]) < clusterDist)
                {
                    cnt++;
                }
            }
            neighbours.push_back(cnt);
        }
        int maxIndex = std::distance(neighbours.begin(), std::max_element(neighbours.begin(), neighbours.end()));
        Pin p;
        p.cnt = neighbours[maxIndex];
        p.x = localUsers[maxIndex].x;
        p.y = localUsers[maxIndex].y;
        pins.push_back(p);
        for(int i = 0; i < localUsers.size(); i++)
        {
            if(dist(localUsers[i], localUsers[maxIndex]) < clusterDist)
            {
                processed[i] = true;
            }
        }
    }
    return pins;
}

UserList users = generate(30, -6, 30, 20, 2000, 30);

WorldWideMsg& GuidesLocation::execute(WorldWideMsg& request) 
{
    const Query& query = request.getQuery();

    double x1 = std::stod(query.getParameter("x1"));
    double x2 = std::stod(query.getParameter("x2"));
    double y1 = std::stod(query.getParameter("y1"));
    double y2 = std::stod(query.getParameter("y2"));

    PinList pins = clasterize(users, x1, x2, y1, y2, 10);

    json j;

    for (const auto& pin: pins)
    {
        j["guide"].push_back({pin.x, pin.y, pin.cnt, 1, 100, 0, 0});
    }

    j["status"] = "ok";

    request.setData(std::move(j.dump()));

    return request;
}

void ExcursionUser::send(std::string_view msg) 
{
    Socket->send(msg, uWS::OpCode::TEXT);
}

void ExcursionUser::async_send(std::string_view msg) 
{
    loop->defer([this, msg]() 
    {
        Socket->send(msg, uWS::OpCode::TEXT);            
    });
}

Traveler::Traveler(Excursion* excursion) : ExcursionUser(excursion)
{
    Trace = el::Loggers::getLogger(excursion->get_eid() + "__T");

    el::Configurations Config;
    Config.setToDefault();
    Config.setGlobally(el::ConfigurationType::Filename, std::string("excursions/" + excursion->get_eid() + "/TravelTrace.log"));
    Config.setGlobally(el::ConfigurationType::ToFile, std::string("true"));
    Config.set(el::Level::Info,
            el::ConfigurationType::Format, "%datetime %msg");
    Trace->configure(Config);
}

bool ExcursionUser::isValid() 
{
    return (Listener && Socket);
}

ExcursionUser::ExcursionUser(Excursion* excursion) : excursion(excursion)
{
    
}

void TravelerWorker(Traveler* self)
{
    self->loop = uWS::Loop::get();
    Excursion* excursion = self->excursion;
    self->Listener = new uWS::App();
    auto* logger = self->Trace;

    self->Listener->ws<PerSocketData>("/traveler/" + self->excursion->get_eid(), 
    {
            .compression = uWS::SHARED_COMPRESSOR,
            .maxPayloadLength = 80 * 1024,
            .idleTimeout = 120,
            .maxBackpressure = 1 * 1024 * 1024,
            .upgrade = [](auto *res, auto *req, auto *context) {

                std::cout << "Traveler upgrade handler" << std::endl;

            /* You may read from req only here, and COPY whatever you need into your PerSocketData.
             * PerSocketData is valid from .open to .close event, accessed with ws->getUserData().
             * HttpRequest (req) is ONLY valid in this very callback, so any data you will need later
             * has to be COPIED into PerSocketData here. */

            /* Immediately upgrading without doing anything "async" before, is simple */
                res->template upgrade<PerSocketData>({
                /* We initialize PerSocketData struct here */
                    
                }, req->getHeader("sec-websocket-key"),
                    req->getHeader("sec-websocket-protocol"),
                    req->getHeader("sec-websocket-extensions"),
                    context);

                /* If you don't want to upgrade you can instead respond with custom HTTP here,
                * such as res->writeStatus(...)->writeHeader(...)->end(...); or similar.*/

                /* Performing async upgrade, such as checking with a database is a little more complex;
                * see UpgradeAsync example instead. */
            },
            .open = [&](uWS::WebSocket<false, true>* ws) 
            {
                std::cout << "Traveler " << ws->getRemoteAddressAsText() << " opened the websocket" << std::endl;
                
                self->Socket = ws;
                json status;

                if (excursion->isValid())
                {
                    status["status"] = "ok";
                }
                else
                {
                    status["status"] = "guide not ready";
                }

                self->send(status.dump());
            },
            .message = [&](auto* ws, std::string_view message, uWS::OpCode opCode) 
            {
                json status;

                std::cout << "Traveler: " << message << std::endl;

                if (!excursion->isValid())
                {
                    status["status"] = "guide not ready";
                    self->send(status.dump());

                    return;
                }

                excursion->sendtoGuide(message);
                logger->info(message);
            },
            .drain = [](auto */*ws*/) {
                    /* Check getBufferedAmount here */
                std::cout << "Drain " << std::endl;
            },

            .close = [&](auto */*ws*/, int /*code*/, std::string_view /*message*/) 
            {
                    std::cout << "Close " << std::endl;

                    self->Socket = nullptr;
            }
            }).listen(9001, [](auto *listen_socket) 
            {
                if (listen_socket) {
                    std::cout << "Thread " << std::this_thread::get_id() << " listening on port " << 9001 << std::endl;
                } else {
                    std::cout << "Thread " << std::this_thread::get_id() << " failed to listen on port 9001" << std::endl;
                }
    }).run();
}

void Traveler::run() 
{
    Worker = new std::thread(TravelerWorker, this);
    Worker->detach();
}

Guide::Guide(Excursion* excursion) : ExcursionUser(excursion)
{
    Trace = el::Loggers::getLogger(excursion->get_eid() + "__G");
    el::Configurations Config;
    Config.setToDefault();
    Config.setGlobally(el::ConfigurationType::Filename, std::string("excursions/" + excursion->get_eid() + "/GuideTrace.log"));
    Config.setGlobally(el::ConfigurationType::ToFile, std::string("true"));
    Config.set(el::Level::Info,
            el::ConfigurationType::Format, "%datetime %msg");
    Trace->configure(Config);
}

void GuideWorker(Guide* self)
{
    self->loop = uWS::Loop::get();
    Excursion* excursion = self->excursion;
    self->Listener = new uWS::App();
    auto* logger = self->Trace;

    self->Listener->ws<PerSocketData>("/guide/" + self->excursion->get_eid(), 
    {
            .compression = uWS::SHARED_COMPRESSOR,
            .maxPayloadLength = 80 * 1024,
            .idleTimeout = 120,
            .maxBackpressure = 1 * 1024 * 1024,
            .upgrade = [](auto *res, auto *req, auto *context) {

                std::cout << "Guide upgrade handler" << std::endl;

            /* You may read from req only here, and COPY whatever you need into your PerSocketData.
             * PerSocketData is valid from .open to .close event, accessed with ws->getUserData().
             * HttpRequest (req) is ONLY valid in this very callback, so any data you will need later
             * has to be COPIED into PerSocketData here. */

            /* Immediately upgrading without doing anything "async" before, is simple */
                res->template upgrade<PerSocketData>({
                /* We initialize PerSocketData struct here */
                    
                }, req->getHeader("sec-websocket-key"),
                    req->getHeader("sec-websocket-protocol"),
                    req->getHeader("sec-websocket-extensions"),
                    context);

                /* If you don't want to upgrade you can instead respond with custom HTTP here,
                * such as res->writeStatus(...)->writeHeader(...)->end(...); or similar.*/

                /* Performing async upgrade, such as checking with a database is a little more complex;
                * see UpgradeAsync example instead. */
            },
            .open = [&](uWS::WebSocket<false, true>* ws) 
            {
                std::cout << "Guide " << ws->getRemoteAddressAsText() << " opened the websocket" << std::endl;
                self->Socket = ws;
                json status;

                if (excursion->isValid())
                {
                    status["status"] = "ok";
                }
                else
                {
                    status["status"] = "traveler not ready";
                }

                self->send(status.dump());
            },
            .message = [&](auto* ws, std::string_view message, uWS::OpCode opCode) 
            {
                json status;

                std::cout << "Guide: " << message << std::endl;

                if (!excursion->isValid())
                {
                    status["status"] = "guide not ready";
                    self->send(status.dump());

                    return;
                }

                // excursion->sendtoTraveler(message);
                logger->info(message);
            },
            .drain = [](auto */*ws*/) {
                    /* Check getBufferedAmount here */
                std::cout << "Drain " << std::endl;
            },

            .close = [&](auto */*ws*/, int /*code*/, std::string_view /*message*/) 
            {
                    std::cout << "Close " << std::endl;

                    self->Socket = nullptr;
            }
            }).listen(9002, [](auto *listen_socket) 
            {
                if (listen_socket) {
                    std::cout << "Thread " << std::this_thread::get_id() << " listening on port 9002" << std::endl;
                } else {
                    std::cout << "Thread " << std::this_thread::get_id() << " failed to listen on port 9002" << std::endl;
                }
    }).run();
}

void Guide::run() 
{
    Worker = new std::thread(GuideWorker, this);
    Worker->detach();
}

bool Excursion::isValid() 
{
    return (traveler->isValid() && guide->isValid());
}

Excursion::Excursion(const std::string& eid) : eid(eid)
{
    traveler = new Traveler(this);
    traveler->run();

    guide = new Guide(this);
    guide->run();
}

void Excursion::sendtoGuide(std::string_view msg) 
{
    guide->async_send(msg);
}

void Excursion::sendtoTraveler(std::string_view msg) 
{
    traveler->async_send(msg);
}