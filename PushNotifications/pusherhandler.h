#ifndef PUSHERHANDLER_H
#define PUSHERHANDLER_H

#include <functional>
#include <queue>

#include <QJsonObject>
#include <QJsonDocument>

#include <QMutex>
#include <QMutexLocker>


#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "Global/databaseconnector.h"
#include "Global/logger.h"

#include "Pusher/iospusher.h"

#include <QThread>


class MyServer;

class PusherHandler : public QObject
{

    Q_OBJECT
private:

    PusherHandler();

    ~PusherHandler();


    PusherHandler(const PusherHandler&) = delete;
    PusherHandler(PusherHandler &&) = delete;

    PusherHandler& operator= (const PusherHandler&) = delete;
    PusherHandler& operator= (PusherHandler&&) = delete;


public:

    static PusherHandler& getHandler()
    {
        static PusherHandler pusher_handler;

        return pusher_handler;
    }


    enum class Platform
    {
        IOS = 0x1
    };
    Q_DECLARE_FLAGS(Platforms, Platform)


private:

    bool is_init = false;

    const QString db_connection_name = QString::number(reinterpret_cast<quint64>(this));

    std::unique_ptr<IosPusher> p_ios_pusher;

    Platforms platforms;

    struct Push_Value
    {
        QJsonObject body;
        QString query;
        QVector<QVariant> vec_value;
    };

    using Push_Function = std::function<void(PusherHandler*, Push_Value &&)>;


    struct Push_Property
    {        
        Push_Function push_function;
        Push_Value push_value;
    };

    std::queue<Push_Property> list_push_queue;

    QMutex mutex_list_push;


    bool initializeHandler(const Platforms platforms);

    void terminate();

    void addPushForIOS(const QString &function_name, const QString &title, Push_Value &&push_value);

private slots:


    void wakeUp();


public:


    void sendPush(Push_Function &&push_function, QString &&query, QVector<QVariant> &&vec_value, const QJsonObject &body = QJsonObject());

    void pushToShipperNewOrder(Push_Value &&push_value);

    void pushToClientShipperOnPickUpPoint(Push_Value &&push_value);

    void pushToClientShipperCompleteOrder(Push_Value &&push_value);

signals:

    void stopped();

    friend class MyServer;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PusherHandler::Platforms)


#endif // PUSHERHANDLER_H
