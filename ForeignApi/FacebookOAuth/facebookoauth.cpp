#include "facebookoauth.h"


FacebookOAuth::FacebookOAuth() : AbstractForeignApi ()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}


FacebookOAuth::~FacebookOAuth()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}


bool FacebookOAuth::initApi()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    const QSet<QString> set_need_config =
    {
        "userinfo_url"
    };

    const auto [is_init, map_received_config] = readConfigFile("facebook", set_need_config);

    if(is_init)
    {
        facebookOAuth_settings.userinfo_url = map_received_config.value("userinfo_url");


        manager = new QNetworkAccessManager;


        state = State::Ready;
    }

    return is_init;
}


void FacebookOAuth::getUserInfo(QString &&access_token, Response_Function &&response_function)
{
    auto calling_function = [this, access_token = std::move(access_token)]()
    {
        QUrl request_url(facebookOAuth_settings.userinfo_url);

        QUrlQuery url_query;

        url_query.addQueryItem(QStringLiteral("fields"), "id,name,email");
        url_query.addQueryItem(QStringLiteral("access_token"), access_token);

        request_url.setQuery(url_query.toString(QUrl::FullyDecoded));


        header_request.setUrl(request_url);

        current_reply = manager->get(header_request);

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});

}


/* Response:

{
  "id": "{user-id}",
  "name": "Fiona Fox",
  "email": "fiona@example.com"
}
*/


