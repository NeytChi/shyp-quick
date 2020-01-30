#ifndef REVERSEGEOCODING_H
#define REVERSEGEOCODING_H

#include "ForeignApi/abstractforeignapi.h"


class ReverseGeocoding : AbstractForeignApi
{

public:
    ReverseGeocoding();

    ~ReverseGeocoding();


private:

    struct ReverseGeocoding_Settings
    {
        QByteArray api_key;

        QString address_from_lat_long_url;

    } reverse_geocoding_settings;


    bool initApi();



public:

    void getAddressFromLatLong(const double latitude, const double longitude, Response_Function &&response_function);

    void getLatLongFromAddress(QString &&address, Response_Function &&response_function);


    friend class Worker;

    friend class ForeignApiHandler;
};

#endif // REVERSEGEOCODING_H
