#ifndef HTTPWEB_H
#define HTTPWEB_H
#include <QTcpServer>

#include <QLocalServer>
#include <QLocalSocket>

#include "worker.h"
#include "database.h"
#include "PushNotifications/pusherhandler.h"


class MyServer : public QTcpServer
{
    Q_OBJECT
public:


    explicit MyServer(QObject *parent = nullptr);

    virtual ~MyServer() override;

    void start(const quint16 port);


private:

    struct Slave_Status
    {
        bool worker_is_working = false;
        bool push_handler_is_working = false;

        bool allStopped() { return !worker_is_working && !push_handler_is_working; }

    } slave_status;

    struct Struct_Worker
    {
        Worker *ptr_worker = nullptr;
        int number_of_connection = 0;
    };

    Struct_Worker struct_worker;


    void incomingConnection(qintptr socket_descriptor) override;


    bool CheckDataBase(QString &check_db_error);

    bool CreatePushHandler();

    bool CreateWorker();

    bool startLocalServerForStopServerCommand(QString &server_error);

    void stopServer();

    std::unique_ptr<SqlBuilder> unique_ptr_sql_builder;


    void checkItems();

};

Q_DECLARE_METATYPE(qintptr)

#endif // HTTPWEB_H
