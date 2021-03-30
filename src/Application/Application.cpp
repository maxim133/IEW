#include "Application.h"
#include <vector>
#include <thread>
#include "easylogging++.h"

ApplicationWorker::ApplicationWorker(WorldWideInterface* ClientInterface, CommandParser& commandParser) : 
ClientInterface(ClientInterface), parser(commandParser)
{
    
}

void ApplicationThread(ApplicationWorker* self) 
{
    CommandParser& parser = self->parser;
    WorldWideInterface* Interface = self->ClientInterface;

    while(1)
    {
        try
        {
            WorldWideMsg Request = Interface->getRequest();
            WorldWideMsg& Response = parser.execute(Request);
            Interface->setResponse(Response);
        }
        catch(const std::runtime_error& e)
        {
            LOG(ERROR) << e.what();
        }
    }
}

void ApplicationWorker::start(int threadCount) 
{
    std::vector<std::thread> ThreadContainer;

    for (int counter = 0; counter < threadCount; ++counter)
    {
        std::thread AppThread(ApplicationThread, this);
        ThreadContainer.push_back(std::move(AppThread));
    }

    for(auto& AppThread : ThreadContainer)
    {
        if (AppThread.joinable())
        {
            AppThread.join();
        }
    }
}
