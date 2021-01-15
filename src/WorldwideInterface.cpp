#include "WorldwideInterface.h"
#include <stdexcept>
#include <thread>
#include "fcgi_stdio.h"
#include <map>
#include <iostream>
#include <cstring>

#define MAX_ContentLength 4096

static std::map<std::string, METHOD> METHOD_TypeConverter = 
{
    {"GET", GET},
    {"POST", POST},
    {"PATCH", PATCH},
    {"DELETE", DELETE},
};

FastCgiProcessor::FastCgiProcessor(const std::string& path)
{
    FCGX_Init();

    socketId = FCGX_OpenSocket(path.c_str(), 20);
    if (socketId == -1)
        throw std::runtime_error("Can't open socket for " + path);
    
    if (FCGX_InitRequest(&request, socketId, 0) != 0)
        throw std::runtime_error("Can't initialize request for " + path);
}

FastCgiProcessor::FastCgiProcessor() 
{
    FCGX_Init();

    if (FCGX_InitRequest(&request, 0, 0) != 0)
        throw std::runtime_error("Can't initialize request ");
}

WorldWideMsg FastCgiProcessor::getRequest() 
{
    AcceptLock.lock();
    int rc = FCGX_Accept_r(&request);
    AcceptLock.unlock();

    if (rc < 0)
        throw std::runtime_error("FCGX_Accept_r error " + std::to_string(errno));
    
    const char* REQUEST_METHOD_str = FCGX_GetParam("REQUEST_METHOD", request.envp);
    METHOD MethodType = METHOD_TypeConverter.at(REQUEST_METHOD_str);

    const char* URI = FCGX_GetParam("SCRIPT_FILENAME", request.envp);

    const char* Query = FCGX_GetParam("QUERY_STRING", request.envp);

    const char* ContentLength = FCGX_GetParam("CONTENT_LENGTH", request.envp);
    
    if (*ContentLength != 0)
    {
        int size = atoi(ContentLength);

        if (size > MAX_ContentLength)
        {
            throw std::runtime_error("Content Length value is to long");
        }

        std::string buffer;
        buffer.reserve(size);

        FCGX_GetStr(buffer.data(), size, request.in);

        return WorldWideMsg(MethodType, URI, Query, buffer);
    }

    return WorldWideMsg(MethodType, URI, Query);
}

void FastCgiProcessor::setResponse(const WorldWideMsg& resp) 
{
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    FCGX_PutS("\r\n", request.out);

    const std::string& data = resp.getData();
    FCGX_PutStr(data.c_str(), data.size(), request.out);
}
WorldWideInterface* CreateWorldWideInterface(InterfaceType type, const char* path) 
{
    switch (type)
    {
    case FastCGI:
    {
        if (path)
        {
            return new FastCgiProcessor(path);
        }

        return new FastCgiProcessor();
    }
    default:
        return nullptr;
    }
}

Query::Query(const char* str) 
{
    const char* str_ptr = str;

    while (*str_ptr != '\0')
    {
        const char* key_ptr = str_ptr;
        int key_size = 0;
        int value_size = 0;

        while (*str_ptr != '=')
        {
            if (*str_ptr == '\0')
                break;

            ++str_ptr;
            ++key_size;
        }

        ++str_ptr;
        const char* value_ptr = str_ptr;

        while (*str_ptr != '&')
        {
            if (*str_ptr == '\0')
                break;

            ++str_ptr;
            ++value_size;
        }

        Params.emplace(std::make_pair(std::string(key_ptr, key_size), std::string(value_ptr, value_size)));

        ++str_ptr;
    }
}
