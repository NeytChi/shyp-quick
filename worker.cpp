#include "worker.h"

Worker::Worker(QObject *parent) : QObject(parent)
{

}


Worker::~Worker()
{
    if(sql_builder != nullptr)
    {
        const QString db_connection_name = sql_builder->getConnectionName();

        delete sql_builder;

        DataBaseConnector::disconnectFromDB(db_connection_name);
    }
}


bool Worker::initializeWorker()
{
    const QString db_connection_name = QString::number(reinterpret_cast<quint64>(this));

    QSqlDatabase data_base;

    bool is_open_db =
    DataBaseConnector::connectToDB(data_base, db_connection_name, DataBaseConnector::Open_With_Schema_Name::Yes);


    if(is_open_db)
    {
        is_open_db = ForeignApiHandler::getDistanceMatrixGoogle()->initApi() &&
                     ForeignApiHandler::getFacebookOAuth()->initApi() &&
                     ForeignApiHandler::getGoogleOAuth()->initApi() &&
                     ForeignApiHandler::getReverseGeocoding()->initApi() &&
                     ForeignApiHandler::getStripeApi()->initApi() &&
                     ForeignApiHandler::getTwilio()->initApi();

        if(is_open_db)
        {
            sql_builder = new SqlBuilder(data_base, db_connection_name);
            sql_builder->setForwardOnly(true);
        }
    }
    else
    {
        Logger::writeLogInFile("can't connect to database, error = " + data_base.lastError().text(), __PRETTY_FUNCTION__);
    }

    if(!is_open_db)
    {
        DataBaseConnector::disconnectFromDB(db_connection_name);
    }



    return is_open_db;
}


void Worker::handleClient(qintptr socket_descriptor)
{
    ClientSocket *clientsocket = new ClientSocket(sql_builder, this);

    connect(clientsocket, &QObject::destroyed, [this]()
    {
        emit clientDisconnected();
    });

    clientsocket->process(socket_descriptor);
}



void Worker::startTimersForSearchingShippersForOrders()
{
    qDebug() << "\r\n---------------" << __PRETTY_FUNCTION__ << "---------------\r\n";

    SqlBuilder sql_builder(QSqlDatabase::database(this->sql_builder->getConnectionName()), this->sql_builder->getConnectionName());

    // Select orders in state = `Finding_Shipper`
    {
        const QString select = "SELECT id_order, id_shipper, pick_up_longitude, pick_up_latitude "
                               "FROM orders "
                               "WHERE state = ? "
                               "ORDER BY id_order;";

        sql_builder.execQuery(select, Order::Order_State::Finding_Shipper);
    }

    if(sql_builder.isExec())
    {
        while(sql_builder.isNext())
        {
            const int id_order = sql_builder.getValue("id_order").toInt();

            Order::Order_For_Searching order_for_searching =
            {
                sql_builder.getValue("id_shipper").toInt(),
                sql_builder.getValue("pick_up_longitude").toDouble(),
                sql_builder.getValue("pick_up_latitude").toDouble(),
            };

            QTimer *timer = Order::createTimerForOrderAndInsertOrderToMap(id_order, sql_builder.getConnectionName(), std::move(order_for_searching));

            timer->start(Global::getOrderSettings().start_searching_for_other_shippers_every_specified_ms);
        }
    }


    if(!sql_builder.isExec())
    {
        const QJsonObject error =
        {
            {"description", "Can't get orders in 'Finding_Shipper' state"},
            {"prepared_query", sql_builder.lastQuery()},
            {"query_error", sql_builder.lastError()},
            {"bound_values", QJsonObject::fromVariantMap(sql_builder.boundValues())}
        };

        Logger::writeLogInFile(error, __PRETTY_FUNCTION__);
    }
}






