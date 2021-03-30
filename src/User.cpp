#include "User.h"
#include "EmailVerificator.h"
#include "easylogging++.h"

WaitingUser::WaitingUser(const std::string& id) :
id(id),uid(std::move(UIDgenerator::generateUID()))
{
    SecretCode = generateSecretCode();
}

void WaitingUser::updateSecretCode(uint16_t code) 
{
    SecretCode = code;
}

void UsersBuffer::addUser(const std::string& id, uint16_t SecretCode) 
{
    std::lock_guard<std::mutex> guard(AttemptLock);

    UsersContainer.emplace_back(id, SecretCode);
}

WaitingUser UsersBuffer::checkUser(const std::string& id, uint16_t SecretCode) 
{
    std::lock_guard<std::mutex> guard(AttemptLock);

    auto userIt = std::find_if( UsersContainer.begin(), UsersContainer.end(), [&](WaitingUser& user)
    {
        return (user.get_id() == id);
    });

    if (userIt == UsersContainer.end())
    {
        throw std::out_of_range("Bad user id");
    }

    /*Если не осталось попыток проверки кода, удаляем юзера из кэша*/
    if (!(*userIt).attempt())
    {
        UsersContainer.erase(userIt);
        throw std::runtime_error("To many attempts");
    }

    LOG(INFO) << "Получили код " << std::to_string(SecretCode) << ". Ожидаем код "
    << std::to_string((*userIt).getSecretCode());

    if ((*userIt).getSecretCode() != SecretCode)
    {
        throw std::runtime_error("Bad secret code");
    }

    WaitingUser user = *userIt;
    UsersContainer.erase(userIt);

    return user;
}

WaitingUser* UsersBuffer::containsUser(const std::string& id) noexcept
{
    std::lock_guard<std::mutex> guard(AttemptLock);

    auto userIt = std::find_if( UsersContainer.begin(), UsersContainer.end(), [&](WaitingUser& user)
    {
        return (user.get_id() == id);
    });

    if (userIt == UsersContainer.end())
    {
        return nullptr;
    }

    return &(*userIt);
}