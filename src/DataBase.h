#ifndef C228A19A_2DC0_4D75_9F4D_C2D0BFA2AC56
#define C228A19A_2DC0_4D75_9F4D_C2D0BFA2AC56

#include <string>
#include <queue>
#include <memory>
#include <mutex>

#include <xdevapi.h>

#include "User.h"

enum UIDs_TableFields
{
    UID_field = 0,
    EUID_field = 1,
    ts_field = 2,
    sid_field = 3,
    email_field = 4,
    phone_field = 5,
    vkid_field = 6,
    fbid_field = 7
};

enum Profile_TableFields
{
    id_field = 0,
    timestamp_field,
    dbirth_field,
    sx_field,
    about_field,
    guide_mode_field,
    guidepro_begin_field,
    guidepro_end_field,
    guidepro_cost_field,
    guidefree_field,
    guidefreefor_field,
    centerx_field,
    centery_field,
    guiderange_field,
    cost_field,
    shedule_field,
    guide_push_field,
    user_push_field,
    payment_info_field
};

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
    ::mysqlx::Table* users;
public:
    DataBaseInterface(const std::string address, bool selfcheck = false);
    ~DataBaseInterface();
    User getUserProfile(uint64_t uid);
    bool checkUserEmail(const std::string& email);
    void addUserProfile(const User& user);
    void updateUserProfile(const User& user);
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
