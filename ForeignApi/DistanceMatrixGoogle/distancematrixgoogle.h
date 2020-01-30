#ifndef DISTANCEMATRIXGOOGLE_H
#define DISTANCEMATRIXGOOGLE_H

#include "ForeignApi/abstractforeignapi.h"




class DistanceMatrixGoogle : AbstractForeignApi
{

public:
    DistanceMatrixGoogle();

    ~DistanceMatrixGoogle();


private:

    struct DistanceMatrixGoogle_Settings
    {
        QByteArray api_key;

        QString distance_duration_url;

    } distance_matrix_google_settings;


    bool initApi();



public:

    struct Coordinates
    {
        double from_latitude;
        double from_longitude;

        double to_latitude;
        double to_longitude;
    };

    void getDistanceAndDuration(Coordinates &&coordinates, Response_Function &&response_function);


    friend class Worker;

    friend class ForeignApiHandler;

    friend class AbstractForeignApi;
};

#endif // DISTANCEMATRIXGOOGLE_H
