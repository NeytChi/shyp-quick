#ifndef FACEBOOKOAUTH_H
#define FACEBOOKOAUTH_H


#include "ForeignApi/abstractforeignapi.h"


class FacebookOAuth : AbstractForeignApi
{
public:
    FacebookOAuth();

    ~FacebookOAuth();

private:

    struct FacebookOAuth_Settings
    {
        QString userinfo_url;

    } facebookOAuth_settings;


    bool initApi();


public:

    void getUserInfo(QString &&access_token, Response_Function &&response_function);


    friend class Worker;

    friend class ForeignApiHandler;

    friend class AbstractForeignApi;
};

#endif // FACEBOOKOAUTH_H
