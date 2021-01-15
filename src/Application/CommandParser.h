#ifndef DC994F72_E5A8_4C6F_8656_358984B1AF61
#define DC994F72_E5A8_4C6F_8656_358984B1AF61

#include "WorldwideInterface.h"

#include <map>

class Command
{
public:
    virtual ~Command() {}
    virtual WorldWideMsg& execute(WorldWideMsg& request) = 0;
};

class CommandParser
{
    std::map<std::string, Command*> CommandContainer;
public:
    void addCommandParser(const std::string& id, Command* comm);
    WorldWideMsg& execute(WorldWideMsg& request);
};

#endif /* DC994F72_E5A8_4C6F_8656_358984B1AF61 */
