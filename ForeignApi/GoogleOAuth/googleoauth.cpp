#include "googleoauth.h"

GoogleOAuth::GoogleOAuth() : AbstractForeignApi ()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}


GoogleOAuth::~GoogleOAuth()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}


bool GoogleOAuth::initApi()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    const QSet<QString> set_need_config =
    {
        "userinfo_url"
    };

    const auto [is_init, map_received_config] = readConfigFile("google", set_need_config);

    if(is_init)
    {
        googleOAuth_settings.userinfo_url = map_received_config.value("userinfo_url");


        manager = new QNetworkAccessManager;


        state = State::Ready;
    }

    return is_init;
}


void GoogleOAuth::getUserInfo(QString &&access_token, Response_Function &&response_function)
{
    auto calling_function = [this, access_token = std::move(access_token)]()
    {
        header_request.setUrl({googleOAuth_settings.userinfo_url + "?access_token=" + access_token});

        current_reply = manager->get(header_request);

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});

}


/* Response:

{
"email":"email@gmail.com",
"email_verified":true,
"picture":"https://lh3.googleusercontent.com/a-/AAuE7mBe4e4HKO2aj0tPrqLz",
"sub":"18556202148319123" // Google id
}
*/

//-----
