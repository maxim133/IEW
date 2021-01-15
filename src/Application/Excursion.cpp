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

UserList users = generate(0, 0, 100, 100, 100, 5);

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
        j["guide"] = {pin.x, pin.y, pin.cnt, 1, 100, 0, 0};
    }

    j["status"] = "ok";

    std::cout << j;

    request.setData(std::move(j.dump()));

    return request;
}
