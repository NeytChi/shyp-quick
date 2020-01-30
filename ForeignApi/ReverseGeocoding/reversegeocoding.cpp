#include "reversegeocoding.h"


ReverseGeocoding::ReverseGeocoding() : AbstractForeignApi ()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}


ReverseGeocoding::~ReverseGeocoding()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}



bool ReverseGeocoding::initApi()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    const QSet<QString> set_need_config =
    {
        "api_key",
        "address_from_lat_long_url"
    };

    const auto [is_init, map_received_config] = readConfigFile("reverse_geocoding", set_need_config);

    if(is_init)
    {
        reverse_geocoding_settings.api_key = map_received_config.value("api_key").toUtf8();

        reverse_geocoding_settings.address_from_lat_long_url = map_received_config.value("address_from_lat_long_url");


        manager = new QNetworkAccessManager;


        state = State::Ready;
    }

    return is_init;
}


void ReverseGeocoding::getAddressFromLatLong(const double latitude, const double longitude, Response_Function &&response_function)
{
    auto calling_function = [this, latitude, longitude]()
    {
        QUrl request_url(reverse_geocoding_settings.address_from_lat_long_url);

        QUrlQuery url_query;


        url_query.addQueryItem(QStringLiteral("latlng"), QString::number(latitude, 'f', 6) + "," + QString::number(longitude, 'f', 6));
        url_query.addQueryItem(QStringLiteral("language"), "en-GB");
        url_query.addQueryItem(QStringLiteral("key"), reverse_geocoding_settings.api_key);


        qDebug() << url_query.toString(QUrl::FullyDecoded);

        request_url.setQuery(url_query.toString(QUrl::FullyDecoded));


        header_request.setUrl(request_url);

        current_reply = manager->get(header_request);

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
   "results" : [
      {
         "address_components" : [
            {
               "long_name" : "1600",
               "short_name" : "1600",
               "types" : [ "street_number" ]
            },
            ...
         ],
         "formatted_address" : "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA",
         "geometry" : {
            "location" : {
               "lat" : 37.4224764,
               "lng" : -122.0842499
            },
            "location_type" : "ROOFTOP",
            "viewport" : {
               "northeast" : {
                  "lat" : 37.4238253802915,
                  "lng" : -122.0829009197085
               },
               "southwest" : {
                  "lat" : 37.4211274197085,
                  "lng" : -122.0855988802915
               }
            }
         },
         "place_id" : "ChIJ2eUgeAK6j4ARbn5u_wAGqWA",
         "types" : [ "street_address" ]
      }
   ],
   "status" : "OK"
}

}

*/


void ReverseGeocoding::getLatLongFromAddress(QString &&address, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, address = std::move(address)]()
    {
        QUrl request_url(reverse_geocoding_settings.address_from_lat_long_url);

        QUrlQuery url_query;


        url_query.addQueryItem(QStringLiteral("address"), address);
        url_query.addQueryItem(QStringLiteral("language"), "en-GB");
        url_query.addQueryItem(QStringLiteral("key"), reverse_geocoding_settings.api_key);


        qDebug() << url_query.toString(QUrl::FullyDecoded);

        request_url.setQuery(url_query.toString(QUrl::FullyDecoded));


        header_request.setUrl(request_url);

        current_reply = manager->get(header_request);

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
   "results" : [
      {
         "address_components" : [
            {
               "long_name" : "1600",
               "short_name" : "1600",
               "types" : [ "street_number" ]
            },
            ...
         ],
         "formatted_address" : "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA",
         "geometry" : {
            "location" : {
               "lat" : 37.4224764,
               "lng" : -122.0842499
            },
            "location_type" : "ROOFTOP",
            "viewport" : {
               "northeast" : {
                  "lat" : 37.4238253802915,
                  "lng" : -122.0829009197085
               },
               "southwest" : {
                  "lat" : 37.4211274197085,
                  "lng" : -122.0855988802915
               }
            }
         },
         "place_id" : "ChIJ2eUgeAK6j4ARbn5u_wAGqWA",
         "types" : [ "street_address" ]
      }
   ],
   "status" : "OK"
}



*/







