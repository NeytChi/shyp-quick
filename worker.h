#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QDebug>

#include "SqlBuilder/sqlbuilder.h"
#include "clientsocket.h"

#include "Global/databaseconnector.h"
#include "Global/logger.h"


class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);


    ~Worker();

private:

    SqlBuilder *sql_builder = nullptr;



signals:
    void clientDisconnected();

public slots:


    void handleClient(qintptr socket_descriptor);

    bool initializeWorker();

    void startTimersForSearchingShippersForOrders();
};

#endif // WORKER_H
