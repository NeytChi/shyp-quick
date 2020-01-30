#include "abstractpusher.h"


AbstractPusher::AbstractPusher(const QString &_platform_, const QSqlDatabase &data_base, const QString &_connection_db_name_)
    : platform(_platform_), sql_builder(data_base, _connection_db_name_)
{
    sql_builder.setForwardOnly(true);

    manager = new QNetworkAccessManager();
}


AbstractPusher::~AbstractPusher()
{
    if(manager != nullptr)
    {
        manager->deleteLater();
    }
}



void AbstractPusher::connectReplyToSlots()
{
    if(current_reply != nullptr)
    {
        QObject::connect(current_reply, &QNetworkReply::readyRead, [this]()
        {
            if(current_reply->error() == QNetworkReply::NoError)
            {
                received_data.append(current_reply->readAll());
            }
        });


        QObject::connect(current_reply, &QNetworkReply::sslErrors, [this](const QList<QSslError> &list_ssl_error)
        {
            error_request.addSslErrors(list_ssl_error);
        });


        QObject::connect(current_reply, &QNetworkReply::finished, this, &AbstractPusher::handleResponse);
    }
}


void AbstractPusher::writePushLog()
{
    QVariantMap map_log = std::move(error_request.map_log);


    if(error_request.type_error == Error_Request::Type_Error::Server_Error)
    {
        map_log.insert("type", "Internal Server Error");
    }
    else if(error_request.type_error == Error_Request::Type_Error::Push_Error)
    {
        map_log.insert("type", "Pushing Error");
    }
    else if(error_request.type_error == Error_Request::Type_Error::Unknow)
    {
        map_log.insert("type", "Unknown Error: no added errors in code, but they occurred");
    }

    if(current_user.user_id > 0)
    {
        map_log.insert("id_user", current_user.user_id);
    }

    if(!current_user.device_token.isEmpty())
    {
        map_log.insert("push_token", current_user.device_token);
    }


    map_log.insert("push_name", current_push.push_name);

    map_log.insert("description", QString::fromUtf8(QJsonDocument(error_request.description).toJson(QJsonDocument::JsonFormat::Compact)));

    map_log.insert("platform", platform);

    map_log.insert("push_body", QString::fromUtf8(current_push.body));


    sql_builder.insertIntoDB(map_log, "log_push");


    if(!sql_builder.isExec())
    {
        const QJsonObject error_description =
        {
            {"type", "Internal Server Error"},
            {"error_description", "can't write log in database"},
            {"query_error", sql_builder.lastError()},
            {"prepared_query", sql_builder.lastQuery()},
            {
                QStringLiteral("prepared_values"),
                QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(sql_builder.boundValues())).toJson(QJsonDocument::JsonFormat::Compact))
            },

            {"log_that_tried_to_record", QJsonObject::fromVariantMap(map_log)}
        };

        Logger::writeLogInFile(error_description, __PRETTY_FUNCTION__);
    }
}


void AbstractPusher::addRequest(const Request &request)
{
    list_request_queue.append(request);

    if(state == State::Ready)
    {
        sendPush();
    }
}



void AbstractPusher::Error_Request::addDescription(const QString &location, const QString &error_description)
{
    if(type_error == Type_Error::Unknow)
    {
        type_error = Type_Error::Push_Error;
    }

    description.insert("description", "In " + location + " error was ocucrred: " + error_description);
}

void AbstractPusher::Error_Request::addQueryError(const SqlBuilder &sql_builder)
{
    type_error = Type_Error::Server_Error;

    description.insert("query_error", sql_builder.lastError());

    map_log.insert("prepared_query", sql_builder.lastQuery());

    map_log.insert(QStringLiteral("prepared_values"),
                   QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(sql_builder.boundValues())).toJson(QJsonDocument::JsonFormat::Compact)));
}

void AbstractPusher::Error_Request::addNetworkErrorCode(const QNetworkReply::NetworkError code)
{
    if(type_error == Type_Error::Unknow)
    {
        type_error = Type_Error::Push_Error;
    }

    description.insert("error_code", QVariant::fromValue(code).toString());
}

void AbstractPusher::Error_Request::addSslErrors(const QList<QSslError> &list_ssl_error)
{
    if(type_error == Type_Error::Unknow)
    {
        type_error = Type_Error::Push_Error;
    }

    if(!list_ssl_error.isEmpty())
    {
        QString string_ssl_error;

        for(const auto &ssl_error : list_ssl_error)
        {
            string_ssl_error.append("ssl_error = ").append(ssl_error.errorString());

            string_ssl_error.append(", ");
        }

        // Cut ', ' at the end
        string_ssl_error.chop(2);

        description.insert("list_ssl_error", string_ssl_error);
    }
}

