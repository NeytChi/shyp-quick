#ifndef ABSTRACTPUSHER_H
#define ABSTRACTPUSHER_H

#include <QObject>

#include <QDebug>

#include "SqlBuilder/sqlbuilder.h"

#include "Global/logger.h"

#include <QJsonObject>
#include <QJsonDocument>


#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QLinkedList>

#include <QSsl>
#include <QSslCertificate>
#include <QSslKey>


class AbstractPusher : public QObject
{
    Q_OBJECT

public:


    AbstractPusher(const QString &_platform_, const QSqlDatabase &data_base, const QString &_connection_db_name_);


    virtual ~AbstractPusher();

    static constexpr auto IOS_PLATFORM = "IOS";

    struct Request
    {
        struct Push
        {
            QString push_name; // for logs
            QByteArray body;
        };

        struct Query
        {
            QString select;
            QVector<QVariant> vec_value;
        };

        Push push;

        Query query;
    };

    void addRequest(const Request &request);


    virtual bool initializePusher() = 0;

protected:

    const QString platform;

    SqlBuilder sql_builder;


    QLinkedList<Request> list_request_queue;


    struct Error_Request
    {
        QVariantMap map_log;

        QJsonObject description;

        enum class Type_Error : unsigned char
        {
            Unknow,
            Server_Error,
            Push_Error

        } type_error = Type_Error::Unknow;

        void addDescription(const QString &location, const QString &error_description);

        void addQueryError(const SqlBuilder &sql_builder);

        void addNetworkErrorCode(const QNetworkReply::NetworkError code);

        void addSslErrors(const QList<QSslError> &list_ssl_error);

    } error_request;


    enum class State : unsigned char
    {
        Ready,
        Pushing

    } state = State::Ready;


    QNetworkAccessManager *manager = nullptr;

    QNetworkRequest header_request;

    QNetworkReply *current_reply = nullptr;


    QByteArray received_data; // clear in handleResponse() at the end of one push

    Request::Push current_push; // clear in handleResponse() at the end of all pushes


    struct Current_User
    {
        int user_id{};
        QString device_token;

    } current_user;




    virtual void sendPush() = 0;


    virtual void handleResponse() = 0;


    void connectReplyToSlots();

    void writePushLog();

};

#endif // ABSTRACTPUSHER_H
