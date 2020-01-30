#ifndef ERRORRESPONSE_H
#define ERRORRESPONSE_H

#include <QDebug>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class Response;

class ErrorResponse
{
public:
    ErrorResponse();


private:

    QJsonObject response_error;


    QJsonObject log_error;


    static constexpr auto error_key = "error";

    static constexpr auto description_key = "description";

    static constexpr auto message_key = "message";


    void insertErrorDescriptionInternal(const QJsonObject &object);

    void clear()
    {
        response_error = QJsonObject();
        log_error = QJsonObject();
    }

public:

    bool isHaveError() const
    {
        return !response_error.isEmpty();
    }

    bool isHaveLogError() const
    {
        return !log_error.isEmpty();
    }

    QString getError() const
    {
        return response_error.value(error_key).toString();
    }

    QString getLogError() const
    {
        return QString::fromUtf8(QJsonDocument(log_error).toJson());
    }

    void insertError(const QString &error);

    void insertError(const QString &error, const QString &invalid_key_log, const QVariant &invalid_value_log);

    void insertError(const QString &error, const QJsonObject &log_error);

    void insertLogError(const QString &invalid_key_log, const QJsonObject &log_error);

    void insertLogError(const QJsonObject &log_error);


    void insertErrorDescription(const QString &message);

    void insertErrorDescription(const QString &message, const QString &key_name, const QJsonValue &value);


    friend class Response;
};

#endif // ERRORRESPONSE_H
