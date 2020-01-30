#include "twilio.h"

Twilio::Twilio() : AbstractForeignApi ()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}


Twilio::~Twilio()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}


bool Twilio::initApi()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";


    const QSet<QString> set_need_config =
    {
        "account_SID",
        "auth_token",
        "from_number",
        "send_sms_url"
    };

    const auto [is_init, map_received_config] = readConfigFile("twilio", set_need_config);

    if(is_init)
    {
        twilio_settings.account_SID = map_received_config.value("account_SID");

        twilio_settings.authentication_credentials =
        (twilio_settings.account_SID + ":" + map_received_config.value("auth_token")).toUtf8().toBase64();

        twilio_settings.from_number = map_received_config.value("from_number");

        manager = new QNetworkAccessManager;


        // Set headers
        default_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        default_request.setRawHeader("Authorization", "Basic " + twilio_settings.authentication_credentials);

        QString url = map_received_config.value("send_sms_url");
        url.replace("<account_sid>", twilio_settings.account_SID);

        default_request.setUrl(QUrl(url));

        header_request = default_request;

        state = State::Ready;
    }

    return is_init;
}




void Twilio::sendSMS(QString &&body, QString &&to_number, Response_Function &&response_function)
{
    auto calling_function = [this, body = std::move(body), to_number = std::move(to_number)]()
    {
        QUrlQuery url_query;

        url_query.addQueryItem(QStringLiteral("Body"), body);
        url_query.addQueryItem(QStringLiteral("From"), twilio_settings.from_number);
        url_query.addQueryItem(QStringLiteral("To"), to_number);

        current_reply = manager->post(header_request, url_query.toString(QUrl::FullyDecoded).toUtf8());

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});

}

