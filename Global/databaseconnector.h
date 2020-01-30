#ifndef DATABASECONNECTOR_H
#define DATABASECONNECTOR_H


#include <QString>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QDebug>


class DataBaseConnector
{

private:
    DataBaseConnector() = delete;


    static QString schema;

    static QString username;

    static QString password;


public:

    enum class Open_With_Schema_Name : bool
    {
        Yes = true,
        No = false
    };


    static bool connectToDB(QSqlDatabase &data_base, const QString &db_connection_name, const Open_With_Schema_Name is_with_db_name);


    static void setConfigs(const QString &_schema_, const QString &_username_, const QString &_password_);


    static void disconnectFromDB(const QString &db_connection_name);

    static QString getSchemaName() { return schema; }
};

#endif // DATABASECONNECTOR_H
