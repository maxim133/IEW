#include "Registration.h"

#include <nlohmann/json.hpp>
#include <ctime>
#include <algorithm>
#include <iostream>
#include "easylogging++.h"
#include "User.h"

using nlohmann::json;

static UsersBuffer RegisteringUsers;

Registration::Registration(DataBaseAccess& access) :
RegistrationMethodFactory(access)
{
    
}

WorldWideMsg& Registration::execute(WorldWideMsg& request) 
{
    auto& query = request.getQuery();

    try
    {
        auto& RefistrationMethod = RegistrationMethodFactory.getRegProcedure(query);
        return RefistrationMethod.getRegistration(request);
    }
    catch(const std::out_of_range& e)
    {
        request.setStatusCode(404);
        return request;
    } 

    return request;
}

WorldWideMsg& EmailRegProcedure::getRegistration(WorldWideMsg& request) 
{
    static json StatusOK = {{"status", "ok"}};
    const Query& query = request.getQuery();
    auto dbInterface = dbAccess.getDataBaseInterface();

    request.setStatusCode(200);

    LOG(INFO) << "Запрос на регистрацию";

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
        
        LOG(INFO) << "Ошибка в формате email";

        return request;
    }  

    if (dbInterface->checkUserEmail(email))
    {
        json status =
            {
                {"status", "email exist"}
            };

        request.setData(std::move(status.dump()));
        dbAccess.returnDataBaseInterface(std::move(dbInterface));

        LOG(INFO) << "Такой email уже есть в БД";

        return request;
    }

    const std::string& requestMsg = request.getData();
    if (!requestMsg.empty())
    {
        LOG(INFO) << "Получен проверочный код";
        try
        {
            json request_json = json::parse(requestMsg);
            uint16_t SecretCode = request_json.value("vcode", 0);
            if (SecretCode != 0)
            {
                LOG(INFO) << "Проверяем секретный код";
                WaitingUser user = std::move(RegisteringUsers.checkUser(email, SecretCode));

                LOG(INFO) << "Успешно";

                User dbUser;
                dbUser.uid = std::hash<std::string>{}(user.get_uid());
                dbUser.email = user.get_id();

                //TODO: добавить обработку исключения от БД
                dbInterface->addUserProfile(dbUser);

                json response =
                {
                    {"uid", user.get_uid()},
                    {"status", "ok"}
                };

                request.setData(std::move(response.dump()));
                dbAccess.returnDataBaseInterface(std::move(dbInterface));
                
                return request;
            }
        }
        catch (json::exception& e)
        {
            json status =
            {
                {"status", "bad parameter"}
            };

            request.setData(std::move(status.dump()));
            request.setStatusCode(400);

            dbAccess.returnDataBaseInterface(std::move(dbInterface));

            LOG(INFO) << "Ошибка в формате запроса " << e.what();

            return request;
        }
        catch (std::out_of_range)
        {
            LOG(INFO) << "Пользователь не существует";
        }
        catch (std::runtime_error& e)
        {
            LOG(INFO) << "Ошибка проверки кода: " << e.what();

            json status =
                {
                    {"status", "vcode error"}
                };

            request.setData(std::move(status.dump()));

            dbAccess.returnDataBaseInterface(std::move(dbInterface));
            return request;
        }
    }

    uint16_t SecretCode = generateSecretCode();
    
    auto user = RegisteringUsers.containsUser(email);

    if (user != nullptr)
    {
        user->updateSecretCode(SecretCode);
    }
    else
    {
        RegisteringUsers.addUser(email, SecretCode);
    }

    LOG(INFO) << "Отправляем код подтверждения на почту";

    EmailVerificator SecretCodeVerification;
    EmailVerificatorStatus status = SecretCodeVerification.sendVerificationCode(SecretCode, email);
    if (status != EmailVerificatorStatus::OK)
    {
        LOG(INFO) << "Ошибка отправки кода";

        json status =
            {
                {"status", "SMTP server error"}
            };

        request.setData(std::move(status.dump()));

        dbAccess.returnDataBaseInterface(std::move(dbInterface));
        return request;
    }

    LOG(INFO) << "Код отправлен. Ждем подтверждения кода от клиента";

    request.setData(std::move(StatusOK.dump()));

    dbAccess.returnDataBaseInterface(std::move(dbInterface));

    return request;
}

RegProcedure& RegProcedure_factory::getRegProcedure(const Query& type) 
{
    if (type.contains("email"))
    {
        return TypeEmail;
    }

    throw std::out_of_range("Bad query type");
}
