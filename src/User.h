#ifndef ED3DCF69_0CBD_4587_AD4F_9421D36B7964
#define ED3DCF69_0CBD_4587_AD4F_9421D36B7964

#include <string>
#include <mutex>
#include <vector>
#include <ctime>
#include <nlohmann/json.hpp>

#include "UIDgenerator.h"

enum UserValidity
{
    NotValid = 0,
    Valid = 1
};

struct User
{
    UserValidity Validity = NotValid;
    std::time_t UpdateTS=0;
    uint64_t uid = 0;
    std::string euid;
    uint64_t sid = 0;
    std::string vkid;
    std::string fbid;
    std::string user_dbirth;
    std::string user_sx;
    std::string user_about;
    std::string email;
    std::string PhoneNumber;
    bool guide_mode = false;
    std::time_t guide_pro_start = 0;
    std::time_t guide_pro_end = 0;
    uint32_t guide_pro_cost = 0;
    bool guide_free = false;
    std::string guide_free_for;
    double guide_pro_center_x = 0;
    double guide_pro_center_y = 0;
    uint32_t guide_range = 0;
    uint32_t guide_h_cost = 0;
    nlohmann::json guide_pro_sched;
    bool guide_push = false;
    bool user_push = false;
    nlohmann::json guide_payment_info;
};

class WaitingUser
{
private:
    std::string uid;
    std::string id;
    uint16_t SecretCode;
    uint32_t Attemption = 4;
public:
    WaitingUser(const std::string& id, uint16_t SecretCode) :
    id(std::move(id)),uid(std::move(UIDgenerator::generateUID())), 
    SecretCode(SecretCode) {}
    WaitingUser(const std::string& id);
    bool attempt()
    {
        return (--Attemption);
    }
    const std::string& get_id() const {return id;}
    const std::string& get_uid() const {return uid;}
    uint16_t getSecretCode() const {return SecretCode;}
    void updateSecretCode(uint16_t code);
};

class UsersBuffer
{
private:
    std::mutex AttemptLock;
    std::vector<WaitingUser> UsersContainer;
public:
    UsersBuffer() 
    {
        UsersContainer.reserve(100);
    }
    void addUser(const std::string& id, uint16_t SecretCode);
    WaitingUser checkUser(const std::string& id, uint16_t SecretCode);
    WaitingUser* containsUser(const std::string& id) noexcept;
};

#endif /* ED3DCF69_0CBD_4587_AD4F_9421D36B7964 */
