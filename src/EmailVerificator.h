#ifndef B6597087_EC72_485F_B0A5_6B33094102E3
#define B6597087_EC72_485F_B0A5_6B33094102E3

#include <string>

enum EmailVerificatorStatus
{
    OK = 0,
    Error
};

class EmailVerificator
{
private:

public:
    EmailVerificator();
    EmailVerificatorStatus sendVerificationCode(uint16_t code, const std::string& email);
};

bool Email_check(std::string& email);
uint16_t generateSecretCode();

#endif /* B6597087_EC72_485F_B0A5_6B33094102E3 */
