#include "iospusher.h"

IosPusher::IosPusher(const QSqlDatabase &data_base, const QString _connection_db_name_)
    : AbstractPusher (IOS_PLATFORM, data_base, _connection_db_name_)
{

}

IosPusher::~IosPusher()
{

}


bool IosPusher::initializePusher()
{
    // Set headers
    header_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    header_request.setAttribute(QNetworkRequest::HTTP2AllowedAttribute, true);

    // Required when delivering notifications to devices running iOS 13 and later, or watchOS 6 and later.
    // Ignored on earlier system versions.
    // alert - displays an alert, plays a sound, or badges your app's icon.

    header_request.setRawHeader("apns-push-type", "alert");

    // The date at which the notification is no longer valid.
    // If the value is 0, APNs attempts to deliver the notification only once and does not store it.

    header_request.setRawHeader("apns-expiration", "0");

    // The topic of the notification.
    // When using certificate-based authentication, the topic is usually your app's bundle ID.

    header_request.setRawHeader("apns-topic", Global::getAPNS().apns_topic.toUtf8());


    // https://github.com/Tulioh/PushSender/blob/master/apnsconnector.cpp SSL
    // Set certificate
    QFile file_cert(Global::getPrimaryPath() + "/Configs/Certificates.p12");

    if(file_cert.open(QIODevice::ReadOnly))
    {
        QSslKey ssl_key;
        QSslCertificate ssl_certificate;

        QSslCertificate::importPkcs12(&file_cert, &ssl_key, &ssl_certificate, nullptr, Global::getAPNS().apns_certificate_password.toUtf8());

        QSslConfiguration ssl_config = QSslConfiguration::defaultConfiguration();

        ssl_config.setLocalCertificate(ssl_certificate);
        ssl_config.setPrivateKey(ssl_key);

        header_request.setSslConfiguration(ssl_config);

        file_cert.close();

        return true;
    }
    else
    {
        Logger::writeLogInFile("Error: Failed to open the " + file_cert.fileName(), __PRETTY_FUNCTION__);

        return false;
    }
}

void IosPusher::terminate()
{
    if(list_request_queue.isEmpty() && (state == State::Ready))
    {
        emit finished();
    }
}


void IosPusher::sendPush()
{
    auto goPush = [this]()
    {
        current_user.user_id = sql_builder.getValue(0).toInt();

        // :path = /3/device/00fc13adff785122b4ad3223420982341241421348097878e577c991de8f0
        // device token
        current_user.device_token = sql_builder.getValue(1).toString();

        header_request.setUrl(QUrl(Global::getAPNS().apns_push_url + current_user.device_token));

        current_reply = manager->post(header_request, current_push.body);

        connectReplyToSlots();
    };


    if(state == State::Ready)
    {
        while(!list_request_queue.isEmpty())
        {
            const Request request = list_request_queue.takeFirst();

            current_push = request.push;

            sql_builder.execQuery(request.query.select, request.query.vec_value);

            if(sql_builder.isExec())
            {
                if(sql_builder.isNext())
                {
                    state = State::Pushing;

                    goPush();

                    break;
                }
            }
            else
            {
                error_request.addDescription(__FUNCTION__, "Can't select push tokens");
                error_request.addQueryError(sql_builder);

                writePushLog();

                current_push = Request::Push();
            }
        }

        if(Global::isNeedToStopServer() && list_request_queue.isEmpty() && (state == State::Ready))
        {
            emit finished();
        }
    }
    else if(state == State::Pushing)
    {
        goPush();
    }
}


/* Response:
 *
 * A successful response:
 *
 * // Headers:
 * * apns-id = eabeae54-14a8-11e5-b60b-1697f925ec7b
 * * :status = 200
 *
 * An error response:
 *
 * // Headers:
 * * :status = 400
 * * content-type = application/json
 * * apns-id: <a_UUID>
 *
 * // Body:
 * { "reason" : "BadDeviceToken" }
 *
 * Errors which means that user device token is invalid:
 *
 * 1. BadDeviceToken - The specified device token is invalid.
 * Verify that the request contains a valid token and that the token matches the environment.
 *
 * 2. Unregistered - The device token is inactive for the specified topic.
 *
 * 3. DeviceTokenNotForTopic
 *
*/


void IosPusher::handleResponse()
{
    static const QSet<int> map_apns_status_code =
    {
        200, 400, 403, 405, 410, 413, 429, 500, 503
    };


    bool is_valid = false;

    int status_code = 0;

    // Get response status code
    {
        const auto attribute_code = current_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

        is_valid = attribute_code.isValid();

        if(is_valid)
        {
            status_code = attribute_code.toInt(&is_valid);

            is_valid = is_valid && map_apns_status_code.contains(status_code);
        }
    }


    if(is_valid)
    {
        if(status_code != 200)
        {
            const QJsonObject response_obj = QJsonDocument::fromJson(received_data).object();

            is_valid = !response_obj.isEmpty();

            if(is_valid)
            {
                const QString reason = response_obj.value("reason").toString();

                is_valid = !reason.isEmpty();

                if(is_valid)
                {
                    is_valid = (reason == QLatin1String("Unregistered")) ||
                               (reason == QLatin1String("BadDeviceToken")) ||
                               (reason == QLatin1String("DeviceTokenNotForTopic"));

                    if(is_valid)
                    {
                        // delete device token
                        SqlBuilder del_sql_builder(sql_builder);

                        del_sql_builder.setForwardOnly(true);

                        const QString del = "DELETE FROM push_token "
                                            "WHERE id_user = ? AND push_token = ?;";

                        del_sql_builder.execQuery(del, {current_user.user_id, current_user.device_token});

                        is_valid = del_sql_builder.isExec();

                        if(!is_valid)
                        {
                            error_request.addDescription(__FUNCTION__, "Can't delete device token");
                            error_request.addQueryError(del_sql_builder);
                        }
                    }
                    else
                    {
                        error_request.addDescription(__FUNCTION__, reason);
                    }
                }
                else
                {
                    error_request.addDescription(__FUNCTION__, "Response reason is empty");
                }
            }
            else
            {
                error_request.addDescription(__FUNCTION__, "Response object is empty");
            }
        }
    }
    else
    {
        error_request.addDescription(__FUNCTION__, current_reply->errorString());
        error_request.addNetworkErrorCode(current_reply->error());
    }


    current_reply->deleteLater();

    current_reply = nullptr;

    received_data.clear();


    if(is_valid)
    {
        if(sql_builder.isNext()) // continue
        {
            sendPush();
        }
        else
        {
            state = State::Ready;

            if(!list_request_queue.isEmpty()) // start new push
            {
                sendPush();
            }
            else // no more pushes, clear data
            {
                current_user.user_id = 0;
                current_user.device_token.clear();

                current_push = Request::Push();

                if(Global::isNeedToStopServer())
                {
                    emit finished();
                }
            }
        }
    }
    else
    {
        writePushLog();

        emit finished();
    }
}

