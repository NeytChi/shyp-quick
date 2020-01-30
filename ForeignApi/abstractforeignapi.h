#ifndef ABSTRACTFOREIGNAPI_H
#define ABSTRACTFOREIGNAPI_H

#include <functional>
#include <queue>

#include <QJsonDocument>
#include <QJsonObject>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QUrlQuery>

#include <QFile>

#include <QCoreApplication>

#include "Response/ErrorResponse/errorresponse.h"

#include "Global/logger.h"
#include "Global/global.h"




class AbstractForeignApi
{

protected:
    AbstractForeignApi();

    virtual ~AbstractForeignApi();

    std::pair<bool, QMap<QString, QString>>
    readConfigFile(const QString &file_name, const QSet<QString> &set_need_config);

    using Calling_Api_Function = std::function<void()>;

    using Response_Function = std::function<void(QJsonObject &&, ErrorResponse &&)>; // response body, error


    struct ApiRequestProperty
    {
        Calling_Api_Function calling_api_function;

        Response_Function response_function;

        QString function_name;
    };


    std::queue<ApiRequestProperty> list_queue;

    enum class State : unsigned char
    {
        Not_Init,
        Ready,
        Handling,
        Server_Error_Occurred
    };

    State state;


    QNetworkAccessManager *manager = nullptr;

    QNetworkRequest default_request;

    QNetworkRequest header_request;


    QNetworkReply *current_reply = nullptr;

    QByteArray received_data;

    void addRequest(ApiRequestProperty &&api_request_property);

    void sendRequest();

    void handleResponse();


protected:

    void connectReplyToSlots();

    friend class Worker;
};

#endif // ABSTRACTFOREIGNAPI_H
