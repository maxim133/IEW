#include "Registration.h"

#include <nlohmann/json.hpp>
#include <ctime>
#include <algorithm>
#include <iostream>

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

    std::cout << "Запрос на регистрацию" << std::endl;

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

    User CheckUser = dbInterface->getUserProfile(email);
    if (CheckUser.isValid())
    {
        json status =
            {
                {"status", "email exist"}
            };

        request.setData(std::move(status.dump()));
        dbAccess.returnDataBaseInterface(std::move(dbInterface));

        std::cout << "Такой email уже есть в БД" << std::endl;

        return request;
    }

    const std::string& requestMsg = request.getData();
    if (!requestMsg.empty())
    {
        std::cout << "Получен проверочный код" << std::endl;
        try
        {
            json request_json = json::parse(requestMsg);
            uint16_t SecretCode = request_json.value("vcode", 0);
            if (SecretCode != 0)
            {
                std::cout << "Проверяем секретный код" << std::endl;
                WaitingUser user = std::move(RegisteringUsers.checkUser(email, SecretCode));

                std::cout << "Успешно" << std::endl;

                User dbUser(user.get_uid(),
                            user.get_id());

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

            std::cout << "Ошибка в формате запроса" << std::endl;

            std::cout << e.what() << std::endl;

            return request;
        }
        catch (std::out_of_range)
        {
            std::cout << "Пользователь не существует" << std::endl;
        }
        catch (std::runtime_error& e)
        {
            std::cout << "Ошибка проверки кода: " << e.what() << std::endl;

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

    std::cout << "Отправляем код подтверждения на почту" << std::endl;

    EmailVerificator SecretCodeVerification;
    EmailVerificatorStatus status = SecretCodeVerification.sendVerificationCode(SecretCode, email);
    if (status != EmailVerificatorStatus::OK)
    {
        std::cout << "Ошибка отправки кода" << std::endl;

        json status =
            {
                {"status", "SMTP server error"}
            };

        request.setData(std::move(status.dump()));

        dbAccess.returnDataBaseInterface(std::move(dbInterface));
        return request;
    }

    std::cout << "Код отправлен. Ждем подтверждения кода от клиента" << std::endl;

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
