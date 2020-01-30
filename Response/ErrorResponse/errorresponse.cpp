#include "errorresponse.h"

ErrorResponse::ErrorResponse()
{

}


void ErrorResponse::insertError(const QString &error)
{
    response_error.insert(error_key, error);
}


void ErrorResponse::insertError(const QString &error, const QString &invalid_key_log, const QVariant &invalid_value_log)
{
    insertError(error);

    log_error.insert(invalid_key_log, invalid_value_log.toJsonValue());
}


void ErrorResponse::insertError(const QString &error, const QJsonObject &log_error)
{
    insertError(error, "log_error", log_error);
}

void ErrorResponse::insertLogError(const QString &invalid_key_log, const QJsonObject &log_error)
{
    this->log_error.insert(invalid_key_log, log_error);
}

void ErrorResponse::insertLogError(const QJsonObject &log_error)
{
    this->log_error.insert("log_error", log_error);
}


void ErrorResponse::insertErrorDescriptionInternal(const QJsonObject &object)
{
    auto it = response_error.constFind(description_key);

    if(it != response_error.constEnd())
    {
        QJsonArray array = it.value().toArray();

        array.append(object);

        response_error.insert(description_key, array);
    }
    else
    {
        response_error.insert(description_key, QJsonArray{object});
    }
}


void ErrorResponse::insertErrorDescription(const QString &message)
{
    const QJsonObject object =
    {
        {message_key, message}
    };

    insertErrorDescriptionInternal(object);
}


void ErrorResponse::insertErrorDescription(const QString &message, const QString &key_name, const QJsonValue &value)
{
    const QJsonObject object =
    {
        {message_key, message},
        {key_name, value}
    };

    insertErrorDescriptionInternal(object);
}
