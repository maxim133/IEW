#ifndef C228A19A_2DC0_4D75_9F4D_C2D0BFA2AC56
#define C228A19A_2DC0_4D75_9F4D_C2D0BFA2AC56

#include <string>
#include <queue>
#include <memory>
#include <mutex>

#include <xdevapi.h>

#include "UserProfile.h"

class DataBaseConfig
{
public:
    int ThreadCount = 1;
    std::string address;
    std::string port;
    std::string user;
    std::string password;
    std::string getURL() const
    {
        return std::string("mysqlx://" + user + ":" + password +
                        "@" + address + ":" + port);
    }
};

class DataBaseInterface
{
private:
    ::mysqlx::Session session;
    ::mysqlx::Schema IEW_schema;
    ::mysqlx::Table users;
public:
    DataBaseInterface(const std::string address);
    User getUserProfile(uint64_t uid);
    User getUserProfile(const std::string& email);
    void addUserProfile(const User& user);
    void UpdateSessionID(uint64_t uid, uint64_t sid);
};

class DataBaseAccess
{
private:
    std::mutex DBInterfaces_mutex;
    std::queue<std::unique_ptr<DataBaseInterface>> DBInterfaces;
public:
    DataBaseAccess(const DataBaseConfig& InterfaceConfig);
    std::unique_ptr<DataBaseInterface> getDataBaseInterface();
    void returnDataBaseInterface(std::unique_ptr<DataBaseInterface> ptr);
};

#endif /* C228A19A_2DC0_4D75_9F4D_C2D0BFA2AC56 */
