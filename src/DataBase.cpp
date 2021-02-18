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
    ::mysqlx::RowResult Result = users.select("*")
                            .where("uid = " + std::to_string(uid)).execute();

    const ::mysqlx::Row row = Result.fetchOne();

    if (!row.isNull())
    {
        uint64_t sid;

        if (row[8].isNull())
        {
            sid = 0;
        }
        else
        {
            sid = row[8].get<uint64_t>();
        }

        return User(row[1].get<uint64_t>(), sid, row[2].get<std::string>(), 
                row[3].get<std::string>(), row[4].get<std::string>());
    }

    return User();
}

User DataBaseInterface::getUserProfile(const std::string& email) 
{
    ::mysqlx::RowResult Result = users.select("*")
                            .where("Email='"+ email + "'").execute();

    const ::mysqlx::Row row = Result.fetchOne();

    if (!row.isNull())
    {
        uint64_t sid;

        if (row[8].isNull())
        {
            sid = 0;
        }
        else
        {
            sid = row[8].get<uint64_t>();
        }

        return User(row[1].get<uint64_t>(), sid, row[2].get<std::string>(), 
                row[3].get<std::string>(), row[4].get<std::string>());
    }

    return User();
}

void DataBaseInterface::addUserProfile(const User& user) 
{
    users.insert("uid", "FirstName", "LastName", "Email")
        .values(user.getUID(), user.getFirstName(), user.getLastName(), user.getEmail())
        .execute();
}

void DataBaseInterface::UpdateSessionID(uint64_t uid, uint64_t sid) 
{
    users.update().set("sid", sid).where("uid = " + std::to_string(uid)).execute();
}
