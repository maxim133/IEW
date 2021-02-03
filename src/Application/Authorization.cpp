#include "Authorization.h"
#include <iostream>
#include <functional>
#include <nlohmann/json.hpp>

using nlohmann::json;

WorldWideMsg& getAuthorization::execute(WorldWideMsg& request) 
{
    auto& query = request.getQuery();
    
    try
    {
        auto& AuthorizationMethod = AuthMethodFactory.getAuthProcedure(query);
        return AuthorizationMethod.getAuthorization(request);
    }
    catch(const std::out_of_range& e)
    {
        request.setStatusCode(404);
        return request;
    } 

    return request;
}

getAuthorization::getAuthorization(DataBaseAccess& access) :
AuthMethodFactory(access)
{
    
}

WorldWideMsg& EmailAuthProcedure::getAuthorization(WorldWideMsg& request) 
{
    return request;
}

WorldWideMsg& UIDAuthProcedure::getAuthorization(WorldWideMsg& request) 
{
    const Query& query = request.getQuery();
    uint64_t uid_hash;
    uint64_t session_id;

    try
    {
        uid_hash = std::hash<std::string>{}(query.getParameter("uid"));
        session_id = std::stoull(query.getParameter("sid"));
    }
    catch(const std::exception& e)
    {
        request.setStatusCode(404);
        return request;
    }

    auto dbInterface = dbAccess.getDataBaseInterface();
    User user = dbInterface->getUserProfile(uid_hash);
    dbAccess.returnDataBaseInterface(std::move(dbInterface));

    if (user.isValid())
    {
        json j =
            {
                {"FirstName", user.getFirstName()},
                {"LastName", user.getLastName()},
                {"email", user.getEmail()},
                {"status", "ok"},
            };

        request.setData(std::move(j.dump()));

        request.setStatusCode(200);

        return request;
    }
    else
    {
        request.setStatusCode(403);
    }

    return request;
}

AuthProcedure& AuthProcedure_factory::getAuthProcedure(const Query& type)
{
    if (type.contains("uid"))
    {
        return TypeUID;
    }

    throw std::out_of_range("Bad query type");
}
