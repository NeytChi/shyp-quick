#include "sqlbuilder.h"


SqlBuilder::SqlBuilder(const QSqlDatabase &_data_base_, const QString &_connection_db_name_) noexcept
    : sql_query(_data_base_), connection_db_name(_connection_db_name_)
{

}


SqlBuilder::SqlBuilder(const SqlBuilder &other) noexcept
    : sql_query(QSqlDatabase::database(other.connection_db_name)),
      connection_db_name(other.connection_db_name),
      is_exec_query(true)
{

}

SqlBuilder::~SqlBuilder()
{

}


QSqlDatabase SqlBuilder::beginTransaction()
{
    QSqlDatabase db = QSqlDatabase::database(connection_db_name);

    is_exec_query = db.transaction();

    return db;
}


void SqlBuilder::endTransaction(QSqlDatabase &db)
{
    if(is_exec_query)
    {
        sql_query.clear();
        is_exec_query = db.commit();
    }

    if(!is_exec_query)
    {
        db.rollback();
    }
}

void SqlBuilder::rollbackTransaction(QSqlDatabase &db)
{
    db.rollback();
}


void SqlBuilder::deleteFrom(const QString &table, const QString &key_condition, const int value_condition)
{
    const QString del = QStringLiteral("DELETE FROM ") + table + " WHERE " + key_condition + " = ?;";

    execQuery(del, value_condition);
}



void SqlBuilder::insertIntoDB(const QVariantMap &map_col_value, const QString &table)
{
    /*
     * "INSERT INTO table(Column1, Column2, Column3) "
     * "VALUES(?, ?, ?, ?);
    */


    QString insert = QStringLiteral("INSERT INTO ") + table + " (";

    QString values = QStringLiteral("VALUES(");


    for(auto it = map_col_value.constBegin(); it != map_col_value.constEnd(); it++)
    {
        insert.append(it.key());

        values.append("?");

        if( (it + 1) != map_col_value.end() )
        {
            insert.append(", ");

            values.append(", ");
        }
        else
        {
            insert.append(") ");

            values.append(");");
        }
    }


    sql_query.prepare(insert + values);

    for(auto it = map_col_value.constBegin(); it != map_col_value.constEnd(); it++)
    {
        sql_query.addBindValue(it.value());
    }

    is_exec_query = sql_query.exec();
}




void SqlBuilder::insertMultiplyIntoDB(const QStringList &list_key, const QVector<QVector<QVariant> > &vec_value, const QString &table)
{
    /*
     * INSERT INTO MyTable
       ( Column1, Column2, Column3 )

       VALUES
      (?, ?, ?),
      (?, ?, ?),
      (?, ?, ?),
      (?, ?, ?);
    */

    QString insert = QStringLiteral("INSERT INTO ") + table + "(";

    insert.append(list_key.join(","));
    insert.append(") ");

    QString values = QStringLiteral("VALUES");

    {
        QStringList list_values;

        for(const auto &it : vec_value)
        {
            QStringList temp;
            for(int i = 0; i < it.size(); i++)
            {
                temp.append("?");
            }

            list_values.append(QString("(" + temp.join(",") + ")"));
        }

        values.append(list_values.join(","));
        values.append(";");
    }


    sql_query.prepare(insert + values);

    for(const auto &it : vec_value)
    {
        for(const auto &value : it)
        {
            sql_query.addBindValue(value);
        }
    }

    is_exec_query = sql_query.exec();
}
