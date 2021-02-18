#include "Authorization.h"
#include <iostream>
#include <functional>
#include <nlohmann/json.hpp>
#include "EmailVerificator.h"

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
    const Query& query = request.getQuery();
    auto dbInterface = dbAccess.getDataBaseInterface();

    std::cout << "Запрос авторизации по email" << std::endl;

    std::string email = query.getParameter("email");

    if (!Email_check(email))
    {
        json status =
            {
                {"status", "bad parameter"}
            };

        request.setData(std::move(status.dump()));
        request.setStatusCode(400);

        dbAccess.returnDataBaseInterface(std::move(dbInterface));

        std::cout << "Ошибка в формате email" << std::endl;

        return request;
    } 

    std::cout << "Проверяем пользователя в БД" << std::endl;

    User CheckUser = dbInterface->getUserProfile(email);
    if (!CheckUser.isValid())
    {
        dbAccess.returnDataBaseInterface(std::move(dbInterface));

        json status =
            {
                {"status", "user not found"}
            };

        request.setData(std::move(status.dump()));

        std::cout << "Пользователь не найден" << std::endl;

        return request;
    }

    std::cout << "Пользователь найден" << std::endl;

    

    return request;
}

WorldWideMsg& UIDAuthProcedure::getAuthorization(WorldWideMsg& request) 
{
    const Query& query = request.getQuery();
    uint64_t uid;
    uint64_t session_id;

    std::cout << "Запрос авторизации по uid" << std::endl;

    try
    {
        uid = std::hash<std::string>{}(query.getParameter("uid"));
        session_id = std::stoull(query.getParameter("sid"));
    }
    catch(const std::exception& e)
    {
        std::cout << "Что-то не так с параметрами запроса" << std::endl;

        json status =
            {
                {"status", "bad parameter"}
            };

        request.setData(std::move(status.dump()));
        request.setStatusCode(404);
        return request;
    }

    auto dbInterface = dbAccess.getDataBaseInterface();
    User user = dbInterface->getUserProfile(uid);

    if (user.isValid())
    {
        std::cout << "Пользователь найден в бд" << std::endl;

        if (user.getSID() != session_id)
        {
            std::cout << "Обновляем session ID" << std::endl;

            dbInterface->UpdateSessionID(uid, session_id);

            dbAccess.returnDataBaseInterface(std::move(dbInterface));
        }

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

    dbAccess.returnDataBaseInterface(std::move(dbInterface));

    std::cout << "Такого пользователя не существует" << std::endl;

    json status =
         {
             {"status", "user not found"}
         };

    request.setData(std::move(status.dump()));
    request.setStatusCode(404);

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
