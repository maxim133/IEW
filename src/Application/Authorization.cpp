#include "Authorization.h"
#include <iostream>
#include <functional>
#include <nlohmann/json.hpp>
#include "EmailVerificator.h"
#include "easylogging++.h"

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

    // User CheckUser = dbInterface->getUserProfile(email);
    // if (dbInterface->checkUserEmail(email))
    // {
    //     dbAccess.returnDataBaseInterface(std::move(dbInterface));

    //     json status =
    //         {
    //             {"status", "user not found"}
    //         };

    //     request.setData(std::move(status.dump()));

    //     std::cout << "Пользователь не найден" << std::endl;

    //     return request;
    // }

    // std::cout << "Пользователь найден" << std::endl;

    

    return request;
}

WorldWideMsg& UIDAuthProcedure::getAuthorization(WorldWideMsg& request) 
{
    const Query& query = request.getQuery();
    uint64_t uid;
    uint64_t session_id;

    LOG(INFO) << "Запрос авторизации по uid";

    try
    {
        uid = std::hash<std::string>{}(query.getParameter("uid"));
        session_id = std::stoull(query.getParameter("sid"));
    }
    catch(const std::exception& e)
    {
        LOG(INFO) << "Что-то не так с параметрами запроса";

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

    if (user.Validity == UserValidity::Valid)
    {
        LOG(INFO) << "Пользователь найден в бд";

        if (user.sid != session_id)
        {
            LOG(INFO) << "Обновляем session ID";

            dbInterface->UpdateSessionID(uid, session_id);
        }

        json j =
            {
                {"user_euid", user.euid},
                {"user_dbirth", user.user_dbirth},
                {"user_sx", user.user_sx},
                {"user_about", user.user_about},
                {"user_email", user.email},
                {"user_phone", user.PhoneNumber},
                {"guide_mode", user.guide_mode},
                {"guide_pro_start", user.guide_pro_start},
                {"guide_pro_end", user.guide_pro_end},
                {"guide_pro_cost", user.guide_pro_cost},
                {"guide_free", user.guide_free},
                {"guide_free_for", user.guide_free_for},
                {"guide_pro_center_x", user.guide_pro_center_x},
                {"guide_pro_center_y", user.guide_pro_center_y},
                {"guide_range", user.guide_range},
                {"guide_h_cost", user.guide_h_cost},
                {"guide_pro_sched", user.guide_pro_sched},
                {"guide_push", user.guide_push},
                {"user_push", user.user_push},
                {"guide_payment_info", user.guide_payment_info},
                {"status", "ok"},
            };

        request.setData(std::move(j.dump()));

        request.setStatusCode(200);
        
        dbAccess.returnDataBaseInterface(std::move(dbInterface));

        return request;
    }

    dbAccess.returnDataBaseInterface(std::move(dbInterface));

    LOG(INFO) << "Такого пользователя не существует";

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
