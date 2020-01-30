#ifndef GOOGLEOAUTH_H
#define GOOGLEOAUTH_H


#include "ForeignApi/abstractforeignapi.h"



class GoogleOAuth : AbstractForeignApi
{

public:
    GoogleOAuth();

    ~GoogleOAuth();

private:

    struct GoogleOAuth_Settings
    {
        QString userinfo_url;

    } googleOAuth_settings;


    bool initApi();



public:

    void getUserInfo(QString &&access_token, Response_Function &&response_function);

    friend class Worker;

    friend class ForeignApiHandler;

    friend class AbstractForeignApi;
};

#endif // GOOGLEOAUTH_H
