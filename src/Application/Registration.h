#ifndef C277109B_4A0C_4AC5_973E_4904DD901D2D
#define C277109B_4A0C_4AC5_973E_4904DD901D2D

#include "CommandParser.h"
#include "DataBase.h"
#include "EmailVerificator.h"
#include "UIDgenerator.h"

class RegProcedure
{
protected:
    DataBaseAccess& dbAccess;
public:
    RegProcedure(DataBaseAccess& dbAccess) : dbAccess(dbAccess) {}
    virtual ~RegProcedure(){}
    virtual WorldWideMsg& getRegistration(WorldWideMsg& request) = 0;
};

class EmailRegProcedure : public RegProcedure
{
public:
    using RegProcedure::RegProcedure;
    WorldWideMsg& getRegistration(WorldWideMsg& request) override;
};

class RegProcedure_factory
{
private:
    EmailRegProcedure TypeEmail;
public:
    RegProcedure_factory(DataBaseAccess& dbAccess) : TypeEmail(dbAccess) 
    {}
    RegProcedure& getRegProcedure(const Query& type);
};

class Registration : public Command
{
private:
    RegProcedure_factory RegistrationMethodFactory;
public:
    explicit Registration(DataBaseAccess& access);
    WorldWideMsg& execute(WorldWideMsg& request) override;
};

#endif /* C277109B_4A0C_4AC5_973E_4904DD901D2D */
