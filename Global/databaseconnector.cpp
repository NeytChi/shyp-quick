#include "databaseconnector.h"


QString DataBaseConnector::schema;

QString DataBaseConnector::username;

QString DataBaseConnector::password;



void DataBaseConnector::setConfigs(const QString &_schema_, const QString &_username_, const QString &_password_)
{
    if(schema.isEmpty() && username.isEmpty() && password.isEmpty())
    {
        schema = _schema_;
        username = _username_;
        password = _password_;
    }
}




bool DataBaseConnector::connectToDB(QSqlDatabase &data_base, const QString &db_connection_name, const Open_With_Schema_Name is_with_db_name)
{
    data_base = QSqlDatabase::addDatabase(QStringLiteral("QMYSQL"), db_connection_name);

    data_base.setHostName(QStringLiteral("127.0.0.1"));

    if(is_with_db_name == Open_With_Schema_Name::Yes)
    {
        data_base.setDatabaseName(schema);
    }

    data_base.setUserName(username);
    data_base.setPassword(password);

    // If closed database connection - reconnect
    data_base.setConnectOptions("MYSQL_OPT_RECONNECT=1");


    return data_base.open();
}




void DataBaseConnector::disconnectFromDB(const QString &db_connection_name)
{
    {
        QSqlDatabase data_base = QSqlDatabase::database(db_connection_name);

        if(data_base.isValid())
        {
            data_base.close();
        }
    }

    QSqlDatabase::removeDatabase(db_connection_name);
}
