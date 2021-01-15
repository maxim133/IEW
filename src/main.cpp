#include <iostream>
#include "Application.h"
#include "CommandParser.h"
#include "Excursion.h"

#include <unistd.h>

int main(int, char**) 
{
    WorldWideInterface* ApplicationClientInterface = CreateWorldWideInterface(InterfaceType::FastCGI, "127.0.0.1:8000");
    CommandParser commandParser;
    Command* ShowGuides = new GuidesLocation();
    commandParser.addCommandParser("/guides/show", ShowGuides);

    ApplicationWorker application(ApplicationClientInterface, commandParser);

    application.start();
}
