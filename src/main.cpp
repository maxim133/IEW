#include <iostream>
#include "Application.h"
#include "CommandParser.h"
#include "Excursion.h"
#include "Authorization.h"
#include "Registration.h"

#include <unistd.h>

int main(int, char**) 
{
    std::cout << "Server start" << std::endl;

    WorldWideInterface* ApplicationClientInterface = CreateWorldWideInterface(InterfaceType::FastCGI, "127.0.0.1:8000");

    //TODO: убрать это в конфигурационный файл
    DataBaseConfig DataBaseInterfaceConfig = 
    {
        1,
        "localhost",
        "33060",
        "root",
        "ikf593kic"
    };
    DataBaseAccess DataBaseInterface(DataBaseInterfaceConfig);

    CommandParser commandParser;
    Command* showGuides = new GuidesLocation();
    commandParser.addCommandParser("/guides/show", showGuides);

    Command* authorization = new getAuthorization(DataBaseInterface);
    commandParser.addCommandParser("/auth", authorization);

    Command* registration = new Registration(DataBaseInterface);
    commandParser.addCommandParser("/registration", registration);

    ApplicationWorker application(ApplicationClientInterface, commandParser);

    application.start();
}
