#ifndef DD331190_1E0C_4501_9918_5CD2CD266797
#define DD331190_1E0C_4501_9918_5CD2CD266797

#include "CommandParser.h"
#include "App.h"
#include <vector>
#include <thread>
#include <easylogging++.h>

struct UserData
{
    double x = 0;
    double y = 0;
};

typedef std::vector<UserData> UserList;

struct Pin
{
    double x = 0;
    double y = 0;
    unsigned int cnt = 0;
};

typedef std::vector<Pin> PinList;

class GuidesLocation : public Command
{
public:
    WorldWideMsg& execute(WorldWideMsg& request) override;
};

class Excursion;

struct PerSocketData {
  std::string_view user_secure_token;
};

class ExcursionUser
{
protected:
    std::thread* Worker = nullptr;
    struct uWS::Loop* loop = nullptr;
    uWS::App* Listener = nullptr;
    uWS::WebSocket<false, true>* Socket = nullptr;
    Excursion* excursion;
    el::Logger* Trace;
public:
    bool isValid();
    ExcursionUser(Excursion* excursion);
    virtual ~ExcursionUser()
    {
        delete Worker;
    }
    virtual void run() = 0;
    void send(std::string_view msg);
    void async_send(std::string_view msg);
};

class Traveler : public ExcursionUser
{
private:
public:
    Traveler(Excursion* excursion);
    void run() override;
    friend void TravelerWorker(Traveler* self);
};

class Guide : public ExcursionUser
{
private:
public:
    Guide(Excursion* excursion);
    void run() override;
    friend void GuideWorker(Guide* self);
};

class Excursion
{
    ExcursionUser* traveler;
    ExcursionUser* guide;
    std::string eid;
public:
    bool isValid();
    Excursion(const std::string& eid);
    const std::string& get_eid() {return eid;}
    void sendtoGuide(std::string_view msg);
    void sendtoTraveler(std::string_view msg);
};

#endif /* DD331190_1E0C_4501_9918_5CD2CD266797 */
