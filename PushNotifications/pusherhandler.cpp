#include "pusherhandler.h"


PusherHandler::PusherHandler()
{

}

PusherHandler::~PusherHandler()
{
    if(is_init)
    {
        terminate();
    }
}


bool PusherHandler::initializeHandler(const Platforms platforms)
{
    // Prevent double initialization
    if(is_init)
    {
        return is_init;
    }

    QSqlDatabase data_base;

    is_init = DataBaseConnector::connectToDB(data_base, db_connection_name, DataBaseConnector::Open_With_Schema_Name::Yes);


    if(is_init)
    {
        this->platforms = platforms;

        if(this->platforms.testFlag(Platform::IOS))
        {
            p_ios_pusher = std::make_unique<IosPusher>(data_base, db_connection_name);

            QObject::connect(p_ios_pusher.get(), &IosPusher::finished, [this]()
            {
                is_init = false;

                // Close database
                DataBaseConnector::disconnectFromDB(db_connection_name);

                p_ios_pusher.release()->deleteLater();

                emit stopped();
            });

            is_init = p_ios_pusher->initializePusher();
        }
    }
    else
    {
        Logger::writeLogInFile("can't open database for PusherHandler", __PRETTY_FUNCTION__);
    }

    if(!is_init)
    {
        terminate();
    }

    return is_init;
}



void PusherHandler::terminate()
{
    is_init = false;

    if(p_ios_pusher)
    {
        p_ios_pusher->terminate();
    }
    else
    {
        // Close database
        DataBaseConnector::disconnectFromDB(db_connection_name);

        emit stopped();
    }
}


// Calling in another Threads
// Thread 2
void PusherHandler::sendPush(Push_Function &&push_function, QString &&query, QVector<QVariant> &&vec_value, const QJsonObject &body)
{
    {
        QMutexLocker locker(&mutex_list_push);

        list_push_queue.push({std::move(push_function), {body, std::move(query), std::move(vec_value)} });
    }

    QMetaObject::invokeMethod(this, &PusherHandler::wakeUp, Qt::QueuedConnection);
}



// Thread 1
void PusherHandler::wakeUp()
{
    Push_Property push_property;

    {
        QMutexLocker locker(&mutex_list_push);

        push_property = std::move(list_push_queue.front());

        list_push_queue.pop();
    }

    if(is_init)
    {
        push_property.push_function(this, std::move(push_property.push_value));
    }
}



void PusherHandler::addPushForIOS(const QString &function_name, const QString &title, Push_Value &&push_value)
{
    IosPusher::Request::Query query_struct = {std::move(push_value.query), std::move(push_value.vec_value)};

    QJsonObject alert =
    {
        {"title", title}
    };

    if(!push_value.body.isEmpty())
    {
        alert.insert("body", push_value.body);
    }

    const QJsonObject push_body =
    {
        {"aps", QJsonObject{{"alert", alert}} }
    };

    IosPusher::Request::Push push_struct = {function_name, QJsonDocument(push_body).toJson(QJsonDocument::JsonFormat::Compact)};

    p_ios_pusher->addRequest({std::move(push_struct), std::move(query_struct)});
}

void PusherHandler::pushToShipperNewOrder(Push_Value &&push_value)
{
    if(platforms.testFlag(Platform::IOS) && p_ios_pusher)
    {
        addPushForIOS(__FUNCTION__, "New Order", std::move(push_value));
    }
}

void PusherHandler::pushToClientShipperOnPickUpPoint(Push_Value &&push_value)
{
    if(platforms.testFlag(Platform::IOS) && p_ios_pusher)
    {
        addPushForIOS(__FUNCTION__, "Shipper On Pick Up Point", std::move(push_value));
    }
}

void PusherHandler::pushToClientShipperCompleteOrder(Push_Value &&push_value)
{
    if(platforms.testFlag(Platform::IOS) && p_ios_pusher)
    {
        addPushForIOS(__FUNCTION__, "Shipper Complete Order", std::move(push_value));
    }
}











