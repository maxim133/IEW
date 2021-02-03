#ifndef A6A46FD3_7338_4F70_BA31_9048686D173F
#define A6A46FD3_7338_4F70_BA31_9048686D173F

#include "WorldwideInterface.h"
#include "CommandParser.h"

class ApplicationWorker
{
private:
    WorldWideInterface* ClientInterface;
    CommandParser& parser;
public:
    friend void ApplicationThread(ApplicationWorker* self);
    ApplicationWorker() = delete;
    ApplicationWorker(WorldWideInterface* ClientInterface,
      CommandParser& commandParser);
    void start(int threadCount = 1);
};

#endif /* A6A46FD3_7338_4F70_BA31_9048686D173F */
