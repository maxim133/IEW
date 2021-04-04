#include "DataBase.h"
#include "easylogging++.h"
#include <iostream>
#include <ctime>

using namespace ::mysqlx;

static const std::string UIDs_TableCreateReq =
    "CREATE TABLE IF NOT EXISTS iew_user_uids "
    "("
    "uid BIGINT UNSIGNED PRIMARY KEY UNIQUE,"
    "euid VARCHAR(128) UNIQUE," 
    "timestamp INT UNSIGNED,"
    "sid BIGINT UNSIGNED,"
    "Email VARCHAR(128) UNIQUE,"
    "Phone VARCHAR(30) UNIQUE,"
    "vkid VARCHAR(128) UNIQUE,"
    "fbid VARCHAR(128) UNIQUE"
    ");";

static const std::string IEW_SwitchReq = "USE IEW;";

DataBaseAccess::DataBaseAccess(const DataBaseConfig& InterfaceConfig)
{
    std::string DBaddress =  InterfaceConfig.getURL();
    try
    {
        auto DBInterface =  std::make_unique<DataBaseInterface>(DBaddress, true);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }    

    for (int counter = 0; counter < InterfaceConfig.ThreadCount; ++counter)
    {
        try
        {
            DBInterfaces.push(std::make_unique<DataBaseInterface>(DBaddress));
        }
        catch (const std::exception& e)
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

DataBaseInterface::DataBaseInterface(const std::string address, bool selfcheck) :
session(address), IEW_schema(session, "IEW")
{
    if (selfcheck)
    {       
        session.sql(IEW_SwitchReq).execute();
        session.sql(UIDs_TableCreateReq).execute();
    }

    users = new ::mysqlx::Table(IEW_schema, "iew_user_uids");
}

DataBaseInterface::~DataBaseInterface() 
{
    //TODO: при извлечении из очереди, вызывается деструктор временного объекта
    // и всё удаляется
    // delete users;
}

User DataBaseInterface::getUserProfile(uint64_t uid) 
{
    std::string UIDstr = std::to_string(uid);

    ::mysqlx::RowResult Result = users->select("*")
                            .where("uid = " + UIDstr).execute();

    const ::mysqlx::Row row = Result.fetchOne();

    if (row.isNull())
    {
        return User();
    }

    User user;
    user.Validity = UserValidity::Valid;
    user.uid = uid;

    if (!row[UIDs_TableFields::EUID_field].isNull()) 
        user.euid = row[UIDs_TableFields::EUID_field].get<std::string>();
    row[UIDs_TableFields::sid_field].isNull() ? user.sid = 0 : user.sid = row[UIDs_TableFields::sid_field].get<uint64_t>();
    if (!row[UIDs_TableFields::email_field].isNull())
        user.email = row[UIDs_TableFields::email_field].get<std::string>();
    if (!row[UIDs_TableFields::phone_field].isNull())
        user.PhoneNumber = row[UIDs_TableFields::phone_field].get<std::string>();
    if (!row[UIDs_TableFields::vkid_field].isNull()) 
        user.vkid = row[UIDs_TableFields::vkid_field].get<std::string>();
    if (!row[UIDs_TableFields::fbid_field].isNull())
        user.fbid = row[UIDs_TableFields::fbid_field].get<std::string>();

    std::string ProfileTableName = "iew_" + UIDstr;

    try
    {
        ::mysqlx::Table ProfileTable = IEW_schema.getTable(ProfileTableName, true);
        ::mysqlx::RowResult MAXid_Result = ProfileTable.select("MAX(id)").execute();
        const ::mysqlx::Row MAXid = MAXid_Result.fetchOne();

        if (MAXid[0].isNull())
        {
            LOG(INFO) << "Таблица профиля " << UIDstr << " пустая";

            return user;
        }

        uint32_t MAXid_value = MAXid[0].get<uint32_t>();

        ::mysqlx::RowResult Profile_Result = ProfileTable.select("*")
                                                        .where("id="+std::to_string(MAXid_value))
                                                        .execute();

        const ::mysqlx::Row row = Profile_Result.fetchOne();

        if (!row[Profile_TableFields::dbirth_field].isNull())
            user.user_dbirth = row[Profile_TableFields::dbirth_field].get<std::string>();
        if (!row[Profile_TableFields::sx_field].isNull())
            user.user_sx = row[Profile_TableFields::sx_field].get<std::string>();
        if (!row[Profile_TableFields::about_field].isNull())
            user.user_about = row[Profile_TableFields::about_field].get<std::string>();
        if (!row[Profile_TableFields::guide_mode_field].isNull())
            user.guide_mode = row[Profile_TableFields::guide_mode_field].get<bool>();
        if (!row[Profile_TableFields::guidepro_begin_field].isNull())
            user.guide_pro_start = row[Profile_TableFields::guidepro_begin_field].get<std::time_t>();
        if (!row[Profile_TableFields::guidepro_end_field].isNull())
            user.guide_pro_end = row[Profile_TableFields::guidepro_end_field].get<std::time_t>();
        if (!row[Profile_TableFields::guidepro_cost_field].isNull())
            user.guide_pro_cost = row[Profile_TableFields::guidepro_cost_field].get<uint32_t>();
        if (!row[Profile_TableFields::guidefree_field].isNull())
            user.guide_free = row[Profile_TableFields::guidefree_field].get<bool>();
        if (!row[Profile_TableFields::guidefreefor_field].isNull())
            user.guide_free_for = row[Profile_TableFields::guidefreefor_field].get<std::string>();
        if (!row[Profile_TableFields::centerx_field].isNull())
            user.guide_pro_center_x = row[Profile_TableFields::centerx_field].get<double>();
        if (!row[Profile_TableFields::centery_field].isNull())
            user.guide_pro_center_y = row[Profile_TableFields::centery_field].get<double>();
        if (!row[Profile_TableFields::guiderange_field].isNull())
            user.guide_range = row[Profile_TableFields::guiderange_field].get<uint32_t>();
        if (!row[Profile_TableFields::cost_field].isNull())
            user.guide_h_cost = row[Profile_TableFields::cost_field].get<uint32_t>();
        if (!row[Profile_TableFields::shedule_field].isNull())
            user.guide_pro_sched = row[Profile_TableFields::shedule_field].get<std::string>();
        if (!row[Profile_TableFields::guide_push_field].isNull())
            user.guide_push = row[Profile_TableFields::guide_push_field].get<bool>();
        if (!row[Profile_TableFields::user_push_field].isNull())
            user.user_push = row[Profile_TableFields::user_push_field].get<bool>();
        if (!row[Profile_TableFields::payment_info_field].isNull())
            user.guide_payment_info = row[Profile_TableFields::payment_info_field].get<std::string>();
    }
    catch(const std::exception& e)
    {
        LOG(WARNING) << "Ошибка запроса данных из БД для профиля " << UIDstr;
        LOG(WARNING) << e.what();

        return User();
    }

    return user;
}

bool DataBaseInterface::checkUserEmail(const std::string& email) 
{
    ::mysqlx::RowResult Result = users->select("*")
                                     .where("Email='" + email + "'")
                                     .execute();
    const ::mysqlx::Row row = Result.fetchOne();

    return !row.isNull();
}

void DataBaseInterface::addUserProfile(const User& user) 
{
    const std::string ProfileTableName = "iew_" + std::to_string(user.uid);
    const std::string PicTableName = "Pic_" + std::to_string(user.uid);

    std::string Profile_TableCreateReq =
    "CREATE TABLE " + ProfileTableName +
    "("
    "id INT UNSIGNED PRIMARY KEY AUTO_INCREMENT,"
    "timestamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
    "user_dbirth VARCHAR(12),"
    "user_sx VARCHAR(20),"
    "user_about TINYTEXT,"
    "guide_mode BOOL,"
    "guide_pro_start INT UNSIGNED,"
    "guide_pro_end INT UNSIGNED,"
    "guide_pro_cost INT UNSIGNED,"
    "guide_free BOOL,"
    "guide_free_for VARCHAR(512),"
    "guide_pro_center_x DOUBLE,"
    "guide_pro_center_y DOUBLE,"
    "guide_range INT UNSIGNED,"
    "guide_h_cost INT UNSIGNED,"
    "guide_pro_sched VARCHAR(512),"
    "guide_push BOOL,"
    "user_push BOOL,"
    "guide_payment_info VARCHAR(512)"
    ");";

    std::string ProfilePic_TableCreateReq =
    "CREATE TABLE " + PicTableName +
    "("
    "id INT UNSIGNED PRIMARY KEY,"
    "ref VARCHAR(128) UNIQUE"
    ");";

    users->insert("uid", "Email")
        .values(user.uid, user.email)
        .execute();

    session.sql(IEW_SwitchReq).execute();
    session.sql(Profile_TableCreateReq).execute();
    session.sql(ProfilePic_TableCreateReq).execute();
}

void DataBaseInterface::updateUserProfile(const User& user) 
{
    const std::string ProfileTableName = "iew_" + std::to_string(user.uid);

    try
    {
        ::mysqlx::Table ProfileTable = IEW_schema.getTable(ProfileTableName);

        ProfileTable.insert("user_dbirth", "user_sx", "user_about", "guide_mode", "guide_pro_start", "guide_pro_end", "guide_pro_cost",
                            "guide_free", "guide_free_for", "guide_pro_center_x", "guide_pro_center_y", "guide_range", "guide_h_cost", "guide_pro_sched", "guide_push", "user_push", "guide_payment_info")
            .values(user.user_dbirth, user.user_sx, user.user_about, user.guide_mode, user.guide_pro_start, user.guide_pro_end, user.guide_pro_cost,
                    user.guide_free, user.guide_free_for, user.guide_pro_center_x, user.guide_pro_center_y, user.guide_range, user.guide_h_cost, user.guide_pro_sched.dump(), user.guide_push, user.user_push,
                    user.guide_payment_info.dump())
            .execute();

        users->update().set("timestamp", std::time(nullptr))
                        .set("euid", user.euid)
                        .set("Email", user.email)
                        .set("Phone", user.PhoneNumber)
                        .where("uid = " + std::to_string(user.uid))
                        .execute();
    }
    catch(const std::exception& e)
    {
        LOG(WARNING) << "Can't update profile in the table for user " << user.uid;
        LOG(WARNING) << e.what();
    }
}

void DataBaseInterface::UpdateSessionID(uint64_t uid, uint64_t sid) 
{
    try
    {
        users->update().set("sid", sid).set("timestamp", std::time(nullptr))
                                    .where("uid = " + std::to_string(uid))
                                    .execute();
    }
    catch(const std::exception& e)
    {
        LOG(WARNING) << "Can't update sid for uid " << uid;
        LOG(WARNING) << e.what();
    }
}
