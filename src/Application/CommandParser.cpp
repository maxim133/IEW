#include "CommandParser.h"
#include <iostream>

void CommandParser::addCommandParser(const std::string& id, Command* comm) 
{
    CommandContainer.emplace(std::make_pair(id, comm));
}

WorldWideMsg& CommandParser::execute(WorldWideMsg& request) 
{
    try
    {
        Command *command = CommandContainer.at(request.getPath());
        command->execute(request);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return request;
}
