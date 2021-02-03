#ifndef A315E5F9_EB74_466D_9636_EFC9DA06F5FA
#define A315E5F9_EB74_466D_9636_EFC9DA06F5FA

#include "CommandParser.h"
#include "DataBase.h"

class AuthProcedure
{
protected:
    DataBaseAccess& dbAccess;
public:
    AuthProcedure(DataBaseAccess& dbAccess) : dbAccess(dbAccess) {}
    virtual ~AuthProcedure(){}
    virtual WorldWideMsg& getAuthorization(WorldWideMsg& request) = 0;
};

class UIDAuthProcedure : public AuthProcedure
{
public:
    using AuthProcedure::AuthProcedure;
    WorldWideMsg& getAuthorization(WorldWideMsg& request) override;
};

class EmailAuthProcedure : public AuthProcedure
{
public:
    using AuthProcedure::AuthProcedure;
    WorldWideMsg& getAuthorization(WorldWideMsg& request) override;
};

class AuthProcedure_factory
{
private:
    EmailAuthProcedure TypeEmail;
    UIDAuthProcedure TypeUID;
public:
    AuthProcedure_factory(DataBaseAccess& dbAccess) : TypeEmail(dbAccess), TypeUID(dbAccess) 
    {}
    AuthProcedure& getAuthProcedure(const Query& type);
};

class getAuthorization : public Command
{
private:
    AuthProcedure_factory AuthMethodFactory;
public:
    explicit getAuthorization(DataBaseAccess& access);
    WorldWideMsg& execute(WorldWideMsg& request) override;
};

#endif /* A315E5F9_EB74_466D_9636_EFC9DA06F5FA */
