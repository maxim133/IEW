#include "EmailVerificator.h"
#include "UIDgenerator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <curl/curl.h>
#include <random>
#include <regex>

bool Email_check(std::string& email)
{
    const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");

    email = std::regex_replace(email, std::regex("%40"), "@");

    return regex_match(email, pattern);
}

uint16_t generateSecretCode()
{
    std::mt19937 gen(std::time(nullptr)); 
    std::uniform_int_distribution<int> uid(1000, 9999);

    return uid(gen);
}

EmailVerificator::EmailVerificator() 
{
    
}

struct upload_status 
{
    const char* readptr;
    size_t sizeleft;
};

static size_t payload_source(char* ptr, size_t size, size_t nmemb, void* userp)
{
    struct upload_status* upload_ctx = reinterpret_cast<upload_status*>(userp);

    if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) 
    {
        return 0;
    }

    if (upload_ctx->sizeleft)
    {
        size_t bytes_to_copy = upload_ctx->sizeleft < size*nmemb ? upload_ctx->sizeleft : size*nmemb;
        std::memcpy(ptr, upload_ctx->readptr, bytes_to_copy);
        upload_ctx->readptr+=bytes_to_copy;
        upload_ctx->sizeleft-=bytes_to_copy;
        
        return bytes_to_copy;
    }

    return 0;
}

EmailVerificatorStatus EmailVerificator::sendVerificationCode(uint16_t code, const std::string& email)
{
    CURL* curl = curl_easy_init();

    if (curl == nullptr)
        return EmailVerificatorStatus::Error;

        time_t rawtime;
    char time_buffer[128];
    time(&rawtime);
    struct tm* timeinfo = localtime(&rawtime);
    strftime(time_buffer, 128, "%a, %d %b %Y %H:%M:%S %z", timeinfo);

    const char payload_template[] =
        "Date: %s\r\n"
        "To: %s\r\n"
        "From: %s\r\n"
        "Message-ID: <%s@etreta.ru>\r\n"
        "Subject: Code verification\r\n"
        "\r\n"
        "%s\r\n\r\n";
    
    const char* TO = email.c_str();
    const char FROM[] = "iew@etreta.ru";
    std::string msg = std::to_string(code);
    
    std::string message_id = UIDgenerator::generateUID(16);

    size_t payload_text_len = strlen(payload_template) +  
                                  strlen(time_buffer) +
                                  strlen(TO) + strlen(FROM) +
                                  message_id.size() + msg.size() + 1;

    char* payload_text = new char[payload_text_len];
    memset(payload_text, 0, payload_text_len);

    sprintf(payload_text, payload_template, time_buffer, TO,
                FROM, message_id.c_str(), msg.c_str());

    struct upload_status upload_ctx =
    {
        .readptr = payload_text,
        .sizeleft = strlen(payload_text)
    };

    struct curl_slist* recipients = nullptr;
    
    //TODO: сделать считывание из конфига
    curl_easy_setopt(curl, CURLOPT_USERNAME, "iew@etreta.ru");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "GW9X170l<Q0kpLk#");
    curl_easy_setopt(curl, CURLOPT_URL, "smtps://mail.etreta.ru");
    curl_easy_setopt(curl, CURLOPT_PORT, "465");
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
    recipients = curl_slist_append(recipients, email.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

        return EmailVerificatorStatus::Error;
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    
    delete payload_text;

    return EmailVerificatorStatus::OK;
}
