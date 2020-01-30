#include "myserver.h"

MyServer::MyServer(QObject *parent) : QTcpServer(parent)
{
    qRegisterMetaType<qintptr>("qintptr");
}

MyServer::~MyServer()
{

}


void MyServer::start(const quint16 port)
{
    if(isListening())
    {
        return;
    }

    QString server_error;

    qDebug() << "Check database...\r\n";

    // Check database

    bool is_valid = CheckDataBase(server_error);


    // Create PushHandler
    if(is_valid)
    {
        qDebug() << "\r\nCreate push handler...\r\n";

        is_valid = CreatePushHandler();
    }



    // Create Worker
    if(is_valid)
    {
        qDebug() << "\r\nCreate worker...\r\n";

        is_valid = CreateWorker();
    }


    // Check connection to db and create items if they don't exist
    if(is_valid)
    {
        qDebug() << "Check connection db, checkItems()...";

        const QString db_connection_name = QString::number(reinterpret_cast<quint64>(this));

        QSqlDatabase data_base;

        bool is_open_db =
        DataBaseConnector::connectToDB(data_base, db_connection_name, DataBaseConnector::Open_With_Schema_Name::Yes);

        if(is_open_db)
        {
            unique_ptr_sql_builder = std::make_unique<SqlBuilder>(data_base, db_connection_name);

            qDebug() << unique_ptr_sql_builder.get();


            checkItems();

            if(unique_ptr_sql_builder->isExec())
            {
                //
            }
            else
            {
                server_error = "Can't create items: " + unique_ptr_sql_builder->lastError();

                DataBaseConnector::disconnectFromDB(db_connection_name);
            }
        }
        else
        {
            server_error = "Can't open database to create items";
        }
    }



    // Start server
    if(is_valid)
    {
        qDebug() << "\r\nStart server...\r\n";

        is_valid = listen(QHostAddress::Any, port);

        if(is_valid)
        {
            qDebug() << "Listening to port " << port << "...";
        }
        else
        {
            server_error = "Could not start server: " + this->errorString();
        }
    }


    // Start local server for stop server command
    if(is_valid)
    {
        qDebug() << "\r\nStart local server for \"stop_server\" command...\r\n";

        is_valid = startLocalServerForStopServerCommand(server_error);
    }


    if(is_valid)
    {
        QMetaObject::invokeMethod(struct_worker.ptr_worker, "startTimersForSearchingShippersForOrders", Qt::QueuedConnection);
    }


    if(!server_error.isEmpty())
    {
        Logger::writeLogInFile(server_error, __PRETTY_FUNCTION__);
    }


    if(!is_valid)
    {
        stopServer();
    }
}



void MyServer::incomingConnection(qintptr socket_descriptor)
{
    qDebug() << "\r\n---------------" << __PRETTY_FUNCTION__ << "---------------\r\n";

    QMetaObject::invokeMethod(struct_worker.ptr_worker, "handleClient", Qt::QueuedConnection, Q_ARG(qintptr, socket_descriptor));

    ++struct_worker.number_of_connection;

    qDebug() << "Worker: " << struct_worker.ptr_worker << ", connections: " << struct_worker.number_of_connection;
}



bool MyServer::CheckDataBase(QString &check_db_error)
{
    bool isInit = false;
    bool isCreate = false;

    DataBase data;
    Tables tables;


    isInit = data.init(check_db_error);

    if(!isInit)
    {
        return false;
    }
    QVector<QString> name_table;
    auto ref = tables.getVector(name_table);
    for(int it = 0; it < ref.size(); it++)
    {
        isCreate = data.createTable(name_table.at(it), ref.at(it), check_db_error);
        if(!isCreate)
        {
            return false;
        }
    }


    return isCreate;

}



bool MyServer::CreatePushHandler()
{
    const bool is_init = PusherHandler::getHandler().initializeHandler(PusherHandler::Platform::IOS);

    if(is_init)
    {
        slave_status.push_handler_is_working = true;

        QObject::connect(&PusherHandler::getHandler(), &PusherHandler::stopped, [this]()
        {
            qDebug() << "PusherHandler::stopped";

            slave_status.push_handler_is_working = false;

            if(slave_status.allStopped())
            {
                this->deleteLater();
            }
        });
    }

    return is_init;
}



bool MyServer::CreateWorker()
{
    QThread *thread = new QThread();

    struct_worker =
    {
        new Worker(),
        0
    };

    struct_worker.ptr_worker->moveToThread(thread);

    slave_status.worker_is_working = true;


    // Worker connections
    connect(struct_worker.ptr_worker, &Worker::destroyed, this, []()
    {
        qDebug() << "Worker Destroyed---";

    }, Qt::QueuedConnection);

    connect(struct_worker.ptr_worker, &Worker::destroyed, thread, &QThread::quit, Qt::QueuedConnection);


    // QThread connections
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);


    connect(thread, &QThread::destroyed, [this]()
    {
        qDebug() << "---Worker QThread's Destroyed---\r\nWorker::Stopped";

        slave_status.worker_is_working = false;

        // Terminate PusherHandler
        if(slave_status.push_handler_is_working)
        {
            PusherHandler::getHandler().terminate();
        }
        else if(slave_status.allStopped())
        {
            this->deleteLater();
        }
    });


    connect(struct_worker.ptr_worker, &Worker::clientDisconnected, this, [this]()
    {
        qDebug() << "\r\n---------------Worker::clientDisconnected() ---------------\r\n";


        --struct_worker.number_of_connection;


        if(Global::isNeedToStopServer() && (struct_worker.number_of_connection == 0) )
        {
            qDebug() << "Delete Worker";

            struct_worker.ptr_worker->deleteLater();
        }

        qDebug() << "number_of_connection = " << struct_worker.number_of_connection;

    }, Qt::QueuedConnection);


    thread->start();


    bool is_init_worker = false;

    QMetaObject::invokeMethod(struct_worker.ptr_worker, &Worker::initializeWorker, Qt::BlockingQueuedConnection, &is_init_worker);

    return is_init_worker;
}



bool MyServer::startLocalServerForStopServerCommand(QString &server_error)
{
    const QString server_name = Global::getPrimaryPath() + "/Stop_Server/SocketFileForServerStopCommand";

    QLocalServer *local_server = new QLocalServer();

    local_server->removeServer(server_name);

    const bool is_listen = local_server->listen(server_name);

    if(is_listen)
    {
        qDebug() << "Listening..." << local_server->fullServerName();

        QObject::connect(local_server, &QLocalServer::newConnection, [this, local_server]()
        {
            qDebug() << "New connection";

            QLocalSocket* local_socket = local_server->nextPendingConnection();

            QObject::connect(local_socket, &QLocalSocket::disconnected, local_socket, &QLocalSocket::deleteLater);

            QObject::connect(local_socket, &QLocalSocket::readyRead, [this, local_server, local_socket]()
            {
                const QByteArray command = local_socket->readAll();

                qDebug() << "command = " << command;

                local_socket->close();
                local_socket->deleteLater();

                if(command == "stop_server")
                {
                    qDebug() << "Stop Server...";

                    local_server->close();
                    local_server->removeServer(local_server->fullServerName());
                    local_server->deleteLater();

                    stopServer();
                }
            });
        });
    }
    else
    {
        server_error = "Can't start local server = " + local_server->errorString();

        local_server->close();
        local_server->removeServer(local_server->fullServerName());
        local_server->deleteLater();
    }

    return is_listen;
}

void MyServer::stopServer()
{
    if(unique_ptr_sql_builder)
    {
        DataBaseConnector::disconnectFromDB(unique_ptr_sql_builder->getConnectionName());
    }

    // Server stop listening
    if(isListening())
    {
        close();
    }

    Global::stopServer();

    // Terminate Worker
    if(slave_status.worker_is_working && (struct_worker.number_of_connection == 0) )
    {
        struct_worker.ptr_worker->deleteLater();
    }


    if(slave_status.allStopped())
    {
        this->deleteLater();
    }
}



void MyServer::checkItems()
{
    // Select items
    {
        const QString select = "SELECT 1 "
                               "FROM item "
                               "LIMIT 1";

        unique_ptr_sql_builder->execQuery(select);
    }

    if(unique_ptr_sql_builder->isExec())
    {
        // Add items in database
        if(!unique_ptr_sql_builder->isNext())
        {
            enum Item_Size : int
            {
                Small,
                Large
            };


            const QStringList list_key =
            {
                "item_name",
                "size"
            };

            QVector<QVector<QVariant>> vec_value;

            const QVector<QString> vec_small_item =
            {
                "Dry cleaning",
                "Microwave",
                "Lamp",
                "Game consoles",
                "Computer equipment",
                "Electronics",
                "Toys",
                "Baby",
                "Auto"
            };

            for(const auto &item : vec_small_item)
            {
                vec_value.append({item, Item_Size::Small});
            }


            const QVector<QString> vec_large_item =
            {
                "TV",
                "Sofa",
                "Bed set",
                "Kitchen tables",
                "Washer & dryer",
                "Patio furniture",
                "Sports equipment",
                "Large Appliances"
            };

            for(const auto &item : vec_large_item)
            {
                vec_value.append({item, Item_Size::Large});
            }

            unique_ptr_sql_builder->insertMultiplyIntoDB(list_key, vec_value, "item");
        }
    }
}
