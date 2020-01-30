#include "distancematrixgoogle.h"

DistanceMatrixGoogle::DistanceMatrixGoogle() : AbstractForeignApi ()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}


DistanceMatrixGoogle::~DistanceMatrixGoogle()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}


bool DistanceMatrixGoogle::initApi()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    const QSet<QString> set_need_config =
    {
        "api_key",
        "distance_duration_url"
    };

    const auto [is_init, map_received_config] = readConfigFile("distance_matrix_google", set_need_config);

    if(is_init)
    {
        distance_matrix_google_settings.api_key = map_received_config.value("api_key").toUtf8();

        distance_matrix_google_settings.distance_duration_url = map_received_config.value("distance_duration_url");


        manager = new QNetworkAccessManager;


        state = State::Ready;
    }

    return is_init;
}


void DistanceMatrixGoogle::getDistanceAndDuration(Coordinates &&coordinates, Response_Function &&response_function)
{
    auto calling_function = [this, coordinates = std::move(coordinates)]()
    {
        QUrl request_url(distance_matrix_google_settings.distance_duration_url);

        QUrlQuery url_query;

        // origins=latitude,longitude
        // origins=41.43206,-81.38992

        {
            const QString origins = QString::number(coordinates.from_latitude, 'f', 6) + "," + QString::number(coordinates.from_longitude, 'f', 6);

            url_query.addQueryItem(QStringLiteral("origins"), origins);
        }

        {
            const QString destinations = QString::number(coordinates.to_latitude, 'f', 6) + "," + QString::number(coordinates.to_longitude, 'f', 6);

            url_query.addQueryItem(QStringLiteral("destinations"), destinations);
        }
        url_query.addQueryItem(QStringLiteral("mode"), "driving");
        url_query.addQueryItem(QStringLiteral("language"), "en-GB");

        //  take traffic conditions into account
        url_query.addQueryItem(QStringLiteral("departure_time"), "now");

        url_query.addQueryItem(QStringLiteral("traffic_model"), "best_guess");

        url_query.addQueryItem(QStringLiteral("key"), distance_matrix_google_settings.api_key);


        request_url.setQuery(url_query.toString(QUrl::FullyDecoded));


        header_request.setUrl(request_url);

        current_reply = manager->get(header_request);

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
  "status": "OK",
  "origin_addresses": [ "Vancouver, BC, Canada", "Seattle ],
  "destination_addresses": [ "San Francisco, Californie ],
  "rows":
[
    {
    "elements":
    [   {
        "status": "OK",
        "duration":
            {
            "value": 340110, // seconds
            "text": "3 jours 22 heures"
            },
        "distance":
            {
            "value": 1734542, // meters
            "text": "1 735 km"
            },
        "duration_in_traffic":
            {
            "value": 1734542,
            "text": "1 735 km"
            }
        }
    ]
    }
]
}

*/



