#ifndef SQLBUILDER_H
#define SQLBUILDER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

#include <QDebug>

class SqlBuilder
{
public:
    SqlBuilder(const QSqlDatabase &_data_base_, const QString &_connection_db_name_) noexcept;

    SqlBuilder(const SqlBuilder &other) noexcept;

    SqlBuilder& operator= (const SqlBuilder &other) = delete;


    SqlBuilder(SqlBuilder &&other) = delete;
    SqlBuilder& operator= (SqlBuilder &&other) = delete;

    ~SqlBuilder();



private:
    QSqlQuery sql_query;

    const QString connection_db_name;

    bool is_exec_query = true;

public:

    QString getConnectionName() const { return connection_db_name; }

    bool isExec() const { return is_exec_query; }


    bool isNext() { return sql_query.next(); }


    void setForwardOnly(bool forward) { sql_query.setForwardOnly(forward); }


    QVariant lastInsertId() const { return sql_query.lastInsertId(); }


    QVariant getValue(const int index) const { return sql_query.value(index); }


    QVariant getValue(const QString &name) const { return sql_query.value(name); }


    QString lastError() const { return sql_query.lastError().text(); }


    QString lastQuery() const { return sql_query.lastQuery(); }


    QVariantMap boundValues() const { return sql_query.boundValues(); }


    QSqlRecord record() const { return sql_query.record(); }


    void reset() { sql_query.clear(); is_exec_query = true; }


    template<class Value>
    void queryBindValue(const Value &value);

    template<class Value = QVector<QVariant>>
    void execQuery(const QString &query, const Value &value);


    void execQuery(const QString &query)
    {
        sql_query.prepare(query);

        is_exec_query = sql_query.exec();
    }

    [[nodiscard]]
    QSqlDatabase beginTransaction();

    void endTransaction(QSqlDatabase &db);

    void rollbackTransaction(QSqlDatabase &db);



    template<class Column = QVector<QString>, class Condition = QVector<QString>>
    QString getSelectString(const Column &column, const QString &table, const Condition &condition, const QString &in_str = "");



    template<class Return = QVariant, class Column = QVector<QString>, class Condition_Key = QVector<QString>, class Condition_Value = QVector<QVariant>>
    Return selectFromDB(const Column &column, const QString &table, const Condition_Key &condition_key, const Condition_Value &condition_value, bool &is_exists);


    void deleteFrom(const QString &table, const QString &key_condition, const int value_condition);


    void insertIntoDB(const QVariantMap &map_col_value, const QString &table);

    void insertMultiplyIntoDB(const QStringList &list_key, const QVector<QVector<QVariant> > &vec_value, const QString &table);


    template<class Column = QPair<QString, QVariant>, class Condition = QPair<QString, QVariant>>
    void updateDB(const Column &column, const QString &table, const Condition &condition);


};

#include "sqlbuilder.tpp"

#endif // SQLBUILDER_H
