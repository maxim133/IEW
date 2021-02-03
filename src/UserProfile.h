#ifndef AA02D53D_CDA8_4ACD_B8B6_533886C6E994
#define AA02D53D_CDA8_4ACD_B8B6_533886C6E994

#include <string>

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
    std::string FirstName;
    std::string LastName;
    std::string email;
public:
    User() {Validity = NotValid;}
    User(uint64_t uid, const std::string& FirstName, const std::string& LastName,
    const std::string& email) : 
    uid(uid), FirstName(FirstName), LastName(LastName), email(email)
    {
        Validity = Valid;
    }
    bool isValid() const {return Validity;}
    const std::string& getFirstName() const {return FirstName;}
    const std::string& getLastName() const {return LastName;}
    const std::string& getEmail() const {return email;}
};

#endif /* AA02D53D_CDA8_4ACD_B8B6_533886C6E994 */
