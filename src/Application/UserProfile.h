#ifndef AA02D53D_CDA8_4ACD_B8B6_533886C6E994
#define AA02D53D_CDA8_4ACD_B8B6_533886C6E994

#include "CommandParser.h"
#include "DataBase.h"
#include "User.h"

class getProfile : public Command
{
private:
    DataBaseAccess& dbAccess;
public:
    explicit getProfile(DataBaseAccess& access);
    WorldWideMsg& execute(WorldWideMsg& request) override;
};

#endif /* AA02D53D_CDA8_4ACD_B8B6_533886C6E994 */
