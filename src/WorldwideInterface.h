#ifndef B30A11F0_4F5A_4BD1_8C8A_9FD6F738C354
#define B30A11F0_4F5A_4BD1_8C8A_9FD6F738C354

#include <string>
#include <mutex>
#include <map>
#include "fcgi_config.h"
#include "fcgiapp.h"

enum InterfaceType
{
    FastCGI,
};

enum METHOD
{
    GET,
    POST,
    PATCH,
    DELETE
};

class Query
{
private:
    std::map<std::string, std::string> Params;
public:
    Query(const char* str);
    Query(Query&& self)
    {
        Params = std::move(self.Params);
    }
    Query& operator=(Query&& self)
	{
        Params = std::move(self.Params);

        return *this;
    }
    const std::string& getParameter(const std::string& name) const
    {
        return Params.at(name);
    }
    bool contains(const std::string name) const
    {
        if (Params.find(name) == Params.end())
        {
            return false;
        } 

        return true;
    }
};

class WorldWideMsg
{
private:
    uint16_t StatusCode;
    METHOD Method;
    std::string URI;
    Query query;
    std::string data;
public:
    WorldWideMsg(METHOD Method, const char* URI, const char* Query, const char* data) :
    Method(Method), URI(URI), query(Query), data(data) {}
    WorldWideMsg(METHOD Method, const char* URI, const char* Query) :
    Method(Method), URI(URI), query(Query) {}
    ~WorldWideMsg() {}
    WorldWideMsg(const WorldWideMsg&) = delete;
    WorldWideMsg& operator=(const WorldWideMsg&) = delete;
    WorldWideMsg(WorldWideMsg&& self) :
	Method(self.Method), query(std::move(query))
	{
        data = std::move(self.data);
        URI = std::move(self.URI);
	}
    WorldWideMsg& operator=(WorldWideMsg&& self)
	{
        Method = self.Method;
        data = std::move(self.data);
        URI = std::move(self.URI);
 
		return *this;
	}
    void setData(const std::string data)
    {
        this->data = data;
    }
    const std::string& getData() const {return data;}
    const std::string& getPath() const {return URI;}
    const Query& getQuery() const {return query;}
    void setStatusCode(uint16_t code) {StatusCode = code;}
    uint16_t getStatusCode() const {return StatusCode;}
};

class WorldWideInterface
{
public:
    virtual WorldWideMsg getRequest() = 0;
    virtual void setResponse(const WorldWideMsg& resp) = 0;
};

WorldWideInterface* CreateWorldWideInterface(InterfaceType type, const char* path = nullptr);

class FastCgiProcessor : public WorldWideInterface
{
private:
    std::mutex AcceptLock;
    int socketId;
    FCGX_Request request;
public:
    FastCgiProcessor(const std::string& path);
    FastCgiProcessor();
    WorldWideMsg getRequest() override;
    void setResponse(const WorldWideMsg& resp) override;
};

#endif /* B30A11F0_4F5A_4BD1_8C8A_9FD6F738C354 */
