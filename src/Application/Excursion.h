#ifndef DD331190_1E0C_4501_9918_5CD2CD266797
#define DD331190_1E0C_4501_9918_5CD2CD266797

#include "CommandParser.h"
#include <vector>

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

#endif /* DD331190_1E0C_4501_9918_5CD2CD266797 */
