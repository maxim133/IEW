#include "UserProfile.h"
#include <functional>
#include "EmailVerificator.h"
#include <iostream>
#include "easylogging++.h"
#include "EmailVerificator.h"

using nlohmann::json;

getProfile::getProfile(DataBaseAccess& access) :
dbAccess(access)
{
    
}

WorldWideMsg& getProfile::execute(WorldWideMsg& request) 
{
    uint64_t uid;
    uint64_t session_id;

    auto& query = request.getQuery();

    try
    {
        uid = std::hash<std::string>{}(query.getParameter("uid"));
        session_id = std::stoull(query.getParameter("sid"));

        LOG(INFO) << "Запрос профиля от пользователя " << uid;
    }
    catch(const std::exception& e)
    {
        LOG(INFO) << "Что-то не так с параметрами запроса профиля";

        json status =
            {
                {"status", "bad parameter"}};

        request.setData(std::move(status.dump()));
        request.setStatusCode(404);
        return request;
    }

    auto dbInterface = dbAccess.getDataBaseInterface();

    User user = dbInterface->getUserProfile(uid);

    if (user.Validity == UserValidity::NotValid)
    {
        LOG(INFO) << "Такого пользователя не существует";

        dbAccess.returnDataBaseInterface(std::move(dbInterface));

        json status =
            {
                {"status", "user not found"}};

        request.setData(std::move(status.dump()));
        request.setStatusCode(404);

        return request;
    }

    LOG(INFO) << "Пользователь найден в бд";

    if (user.sid != session_id)
    {
        LOG(INFO) << "Неправильный session ID";

        dbAccess.returnDataBaseInterface(std::move(dbInterface));

        json status =
            {
                {"status", "bad sid"}};

        request.setData(std::move(status.dump()));
        request.setStatusCode(403);

        return request;
    }

    if (query.contains("euid"))
    {
        //TODO: метод не реализован
        std::string_view euid = query.getParameter("euid");
        LOG(INFO) << "Запрошен профиль с euid=" << euid;

        dbAccess.returnDataBaseInterface(std::move(dbInterface));

        json status =
            {
                {"status", "access denidied"}};

        request.setData(std::move(status.dump()));
        request.setStatusCode(403);
    }

    const std::string& URI = request.getPath();

    if (URI == "/profile/get")
    {
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
    }
    else if (URI == "/profile/update")
    {
        try
        {
            json userdata = json::parse(request.getData());

            //TODO: Добавить валидацию входящих данных
            User user = 
            {
                UserValidity::Valid,
                std::time(nullptr),
                uid,
                userdata.at("user_euid"),
                session_id,
                std::string(),
                std::string(),
                userdata.at("user_dbirth"),
                userdata.at("user_sx"),
                userdata.at("user_about"),
                userdata.at("user_email"),
                userdata.at("user_phone"),
                userdata.at("guide_mode"),
                userdata.at("guide_pro_start"),
                userdata.at("guide_pro_end"),
                userdata.at("guide_pro_cost"),
                userdata.at("guide_free"),
                userdata.at("guide_free_for"),
                userdata.at("guide_pro_center_x"),
                userdata.at("guide_pro_center_y"),
                userdata.at("guide_range"),
                userdata.at("guide_h_cost"),
                userdata.at("guide_pro_sched"),
                userdata.at("guide_push"),
                userdata.at("user_push"),
                userdata.at("guide_payment_info")
            };

            dbInterface->updateUserProfile(user);

            json status =
                {
                    {"status", "ok"}};
            request.setData(std::move(status.dump()));
            request.setStatusCode(200);
        }
        catch (const std::exception& e)
        {
            LOG(WARNING) << "Ошибка обновления данных профиля для " << uid <<
            ". Ошибка в формате json: " << e.what();

            json status =
                {
                    {"status", "bad profile struct"}
                };

            request.setData(std::move(status.dump()));
            request.setStatusCode(400);
        }
    }

    dbAccess.returnDataBaseInterface(std::move(dbInterface));
    
    return request;
}