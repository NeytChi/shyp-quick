#ifndef REQUESTBODY_H
#define REQUESTBODY_H

#include <QDebug>


#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>


#include <QMimeDatabase>
#include <QMimeType>

#include "Response/ErrorResponse/errorresponse.h"


class Checker;


class RequestBody
{
public:
    RequestBody();

    ~RequestBody();

    inline RequestBody(RequestBody &&other) noexcept;

    RequestBody& operator= (RequestBody &&other) noexcept;



private:

    RequestBody(const RequestBody&) = delete;

    RequestBody& operator = (const RequestBody&) = delete;

    static constexpr auto error_invalid_data = "received data is invalid";

    static constexpr auto error_empty_data = "received data is empty";


public:

    struct MultiData
    {
        QString filename;
        QMimeType type;
        QByteArray data;
    };

    QMap<QString, MultiData> file_map;
    QJsonObject received_json;
    QMap<QString, QString> map_get_parameter;

    QString request_url;


public:


    [[nodiscard]]
    ErrorResponse parseJson(const QByteArray &request_data);

    [[nodiscard]]
    ErrorResponse parseMultiData(QByteArray &request_data, const QString &boundary);

    [[nodiscard]]
    ErrorResponse parseAppUrlEncoded(const QByteArray &request_data);

    [[nodiscard]]
    ErrorResponse parseGetParameters(const QString &url);

    friend class Checker;
};

#endif // REQUESTBODY_H
