#ifndef TWILIO_H
#define TWILIO_H

#include "ForeignApi/abstractforeignapi.h"


class Twilio : AbstractForeignApi
{

public:
    Twilio();

    ~Twilio();

private:

    struct Twilio_Settings
    {
        QString account_SID;

        QByteArray authentication_credentials;

        QString from_number;

    } twilio_settings;


    bool initApi();



public:

    void sendSMS(QString &&body, QString &&to_number, Response_Function &&response_function);



    friend class Worker;

    friend class ForeignApiHandler;
};

#endif // TWILIO_H
