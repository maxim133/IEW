#include <iostream>
#include "Application.h"
#include "CommandParser.h"
#include "Excursion.h"
#include "Authorization.h"
#include "Registration.h"
#include "UserProfile.h"
#include "easylogging++.h"

#include <unistd.h>

INITIALIZE_EASYLOGGINGPP

int main(int, char**) 
{
    el::Configurations conf;
	if (!conf.parseFromFile("log.conf"))
	{
		conf.setToDefault();
		conf.setGlobally(el::ConfigurationType::ToFile,"false");
		el::Loggers::reconfigureAllLoggers(conf);

		LOG(WARNING) << "No log.conf. Log using default settings.";
	}

	LOG(INFO) << "Server start";

    WorldWideInterface* ApplicationClientInterface = CreateWorldWideInterface(InterfaceType::FastCGI, "127.0.0.1:8000");

    Excursion excurtion("unicalexcurtionid");

     //TODO: убрать это в конфигурационный файл
    DataBaseConfig DataBaseInterfaceConfig = 
    {
        1,
        "localhost",
        "33060",
        "test",
        "test"
    };
    DataBaseAccess DataBaseInterface(DataBaseInterfaceConfig);

    CommandParser commandParser;
    Command* showGuides = new GuidesLocation();
    commandParser.addCommandParser("/guides/show", showGuides);

    Command* authorization = new getAuthorization(DataBaseInterface);
    commandParser.addCommandParser("/auth", authorization);

    Command* registration = new Registration(DataBaseInterface);
    commandParser.addCommandParser("/registration", registration);

    Command* getprofile = new getProfile(DataBaseInterface);
    commandParser.addCommandParser("/profile/get", getprofile);
    commandParser.addCommandParser("/profile/update", getprofile);

    ApplicationWorker RESTapplication(ApplicationClientInterface, commandParser);

    RESTapplication.start();
}
