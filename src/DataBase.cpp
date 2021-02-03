#include "DataBase.h"

#include <iostream>

using namespace ::mysqlx;

DataBaseAccess::DataBaseAccess(const DataBaseConfig& InterfaceConfig)
{
    std::string DBaddress =  InterfaceConfig.getURL();

    for (int counter = 0; counter < InterfaceConfig.ThreadCount; ++counter)
    {
        try
        {
            DBInterfaces.push(std::make_unique<DataBaseInterface>(DBaddress));
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}

std::unique_ptr<DataBaseInterface> DataBaseAccess::getDataBaseInterface() 
{
    std::lock_guard<std::mutex> guard(this->DBInterfaces_mutex);
    std::unique_ptr<DataBaseInterface> interface = std::move(DBInterfaces.front());
    DBInterfaces.pop();

    return interface;
}

void DataBaseAccess::returnDataBaseInterface(std::unique_ptr<DataBaseInterface> ptr) 
{
    std::lock_guard<std::mutex> guard(this->DBInterfaces_mutex);
    DBInterfaces.push(std::move(ptr));
}

DataBaseInterface::DataBaseInterface(const std::string address) :
session(address), IEW_schema(session, "IEW"), users(IEW_schema, "IEW_users")
{
}

User DataBaseInterface::getUserProfile(uint64_t uid) 
{
    ::mysqlx::RowResult myResult = users.select("*")
                            .where("uid = " + std::to_string(uid)).execute();

    const ::mysqlx::Row& row = myResult.fetchOne();

    if (!row.isNull())
    {
        return User(row[1].get<uint64_t>(), row[2].get<std::string>(), 
                row[3].get<std::string>(), row[4].get<std::string>());
    }

    return User();
}
