#ifndef AA02D53D_CDA8_4ACD_B8B6_533886C6E994
#define AA02D53D_CDA8_4ACD_B8B6_533886C6E994

#include <string>
#include <mutex>
#include <vector>

#include "UIDgenerator.h"

class User
{
    enum UserValidity
    {
        NotValid = 0,
        Valid = 1
    };
private:
    UserValidity Validity;
    uint64_t uid;
    uint64_t sid;
    std::string FirstName;
    std::string LastName;
    std::string email;
    std::string PhoneNumber;
public:
    User() {Validity = NotValid;}
    User(uint64_t uid, uint64_t sid, const std::string& FirstName, const std::string& LastName,
    const std::string& email);
    User(const std::string& uid, const std::string& email);
    bool isValid() const {return Validity;}
    const std::string& getFirstName() const {return FirstName;}
    const std::string& getLastName() const {return LastName;}
    const std::string& getEmail() const {return email;}
    uint64_t getUID() const {return uid;}
    uint64_t getSID() const {return sid;}
    const std::string& getPhoneNumber() const {return PhoneNumber;}
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

#endif /* AA02D53D_CDA8_4ACD_B8B6_533886C6E994 */
