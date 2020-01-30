#include "sqlbuilder.h"





template<class Value>
void SqlBuilder::queryBindValue(const Value &value)
{
    static_assert ((!std::is_same<Value, QJsonArray>::value) && (!std::is_same<Value, QJsonObject>::value), "Invalid bind type value");


    constexpr bool is_simple = (std::is_same<Value, QVariant>::value) ||
                               (std::is_same<Value, bool>::value) ||
                               (std::is_same<Value, QString>::value) ||
                               (std::is_same<Value, int>::value) ||
                               (std::is_same<Value, double>::value);

    if constexpr(is_simple)
    {
        sql_query.addBindValue(value);
    }
    else if constexpr(std::is_enum<Value>::value)
    {
        sql_query.addBindValue(static_cast<int>(value));
    }
    else
    {
        for(const auto &it : value)
        {
            sql_query.addBindValue(it);
        }
    }
}



template<class Value>
void SqlBuilder::execQuery(const QString &query, const Value &value)
{
    sql_query.prepare(query);

    queryBindValue(value);

    is_exec_query = sql_query.exec();
}



template<class Column, class Condition>
QString SqlBuilder::getSelectString(const Column &column, const QString &table, const Condition &condition, const QString &in_str)
{
    qDebug() << "\r\n------ " << __FUNCTION__ << " ------\r\n";

    // Column
    bool constexpr is_vec_column = std::is_same<Column, QVector<QString>>::value;

    bool constexpr is_map_column = std::is_same<Column, QVariantMap>::value;


    // Condition
    bool constexpr is_vec_condition = std::is_same<Condition, QVector<QString>>::value;

    bool constexpr is_map_condition = std::is_same<Condition, QVariantMap>::value;


    //static_assert (is_vec_column || is_same<Column, QString>::value, "Invalid column type");

    //static_assert (is_vec_condition || is_convertible<Condition, QString>::value, "Invalid condition type");

    QString select = QStringLiteral("SELECT ");


    // Append column
    if constexpr(is_vec_column || is_map_column)
    {
        for(auto it = column.constBegin(); it != column.constEnd(); it++)
        {
            if constexpr(is_vec_column)
            {
                select.append(*it);
            }
            else
            {
                select.append(it.key());
            }

            if( (it + 1) != column.constEnd() )
            {
                select.append(", ");
            }
        }
    }
    else
    {
        select.append(column);
    }

    select.append(" FROM ").append("`" + table + "`");

    select.append(" WHERE ");

    // Append condition
    if constexpr(is_vec_condition || is_map_condition)
    {
        for(auto it = condition.constBegin(); it != condition.constEnd(); it++)
        {
            if constexpr(is_vec_condition)
            {
                select.append(*it).append(" = ?");
            }
            else
            {
                select.append(it.key()).append(" = ?");
            }


            if( (it + 1) != condition.constEnd() )
            {
                select.append(" AND ");
            }
        }
    }
    else
    {
        select.append(condition).append(" = ?");
    }

    if(in_str.isEmpty())
    {
        select.append(";");
    }
    else
    {
        select.append(" AND ").append(in_str);
    }

    qDebug() << select;


    return select;
}



template<class Return, class Column, class Condition_Key, class Condition_Value>
Return SqlBuilder::selectFromDB(const Column &column, const QString &table, const Condition_Key &condition_key, const Condition_Value &condition_value, bool &is_exists)
{
    qDebug() << "\r\n------ " << __FUNCTION__ << " ------\r\n";


    // Check return value
    constexpr bool is_return_one_value = std::is_same<Return, QVariant>::value;

    constexpr bool is_return_many_values = std::is_same<Return, QVariantMap>::value;

    static_assert (is_return_one_value || is_return_many_values, "Invalid return type");


    // Check Column size = Return value
    bool constexpr is_vec_column = std::is_same<Column, QVector<QString>>::value;

    bool constexpr is_map_column = std::is_same<Column, QVariantMap>::value;

    if constexpr(is_vec_column || is_map_column)
    {
        static_assert (is_return_many_values, "Return type doesn't equal column keys");
    }
    else
    {
        static_assert (is_return_one_value, "Return type doesn't equal column keys");
    }

    // Check condition
    {
        if constexpr(std::is_same<Condition_Key, QVector<QString>>::value)
        {
            static_assert (std::is_same<Condition_Value, QVector<QVariant>>::value, "Invalid Condition");
        }
        else if constexpr(std::is_same<Condition_Key, QVariantMap>::value)
        {
            static_assert (std::is_same<Condition_Value, QVariantMap>::value, "Invalid Condition");
        }
    }


    const auto select = getSelectString(column, table, condition_key);

    Return return_value;

    execQuery(select, condition_value);

    if(is_exec_query)
    {
        is_exists = sql_query.next();

        if(is_exists)
        {
            if constexpr(is_return_one_value)
            {
                return_value = {sql_query.value(0)};
            }
            else
            {
                const auto &rec = sql_query.record();

                for(int i = 0; i < rec.count(); i++)
                {
                    return_value.insert(rec.fieldName(i), rec.value(i));
                }
            }
        }
    }

    qDebug() << select;

    qDebug() << sql_query.boundValues();

    qDebug() << return_value;

    return return_value;
}



template<class Column, class Condition>
void SqlBuilder::updateDB(const Column &column, const QString &table, const Condition &condition)
{
    qDebug() << "\r\n------ " << __FUNCTION__ << " ------\r\n";

    constexpr bool is_column_pair = std::is_same<Column, QPair<QString, QVariant>>::value;

    constexpr bool is_condition_pair = std::is_same<Condition, QPair<QString, QVariant>>::value;


    static_assert (is_column_pair || std::is_same<Column, QVariantMap>::value, "Invalid type");

    static_assert (is_condition_pair || std::is_same<Condition, QVariantMap>::value, "Invalid type");


    QString update = QStringLiteral("UPDATE ") + table + " SET ";


    // Get column keys
    if constexpr(is_column_pair)
    {
        update.append(column.first).append(" = ?");
    }
    else
    {
        for(auto it = column.constBegin(); it != column.constEnd(); it++)
        {
            update.append(it.key()).append(" = ?");

            if( (it + 1) != column.constEnd() )
            {
                update.append(", ");
            }
        }
    }

    update.append(" WHERE ");


    // Get condition keys
    if constexpr(is_condition_pair)
    {
        update.append(condition.first).append(" = ?");
    }
    else
    {
        for(auto it = condition.constBegin(); it != condition.constEnd(); it++)
        {
            update.append(it.key()).append(" = ?");

            if( (it + 1) != condition.constEnd() )
            {
                update.append(" AND ");
            }
            else
            {
                update.append(";");
            }
        }
    }

    qDebug() << "\r\nUPDATE = " << update;

    sql_query.prepare(update);


    // Add column values
    if constexpr(is_column_pair)
    {
        sql_query.addBindValue(column.second);
    }
    else
    {
        for(auto it = column.constBegin(); it != column.constEnd(); it++)
        {
            sql_query.addBindValue(it.value());
        }
    }


    // Add condition values
    if constexpr(is_condition_pair)
    {
        sql_query.addBindValue(condition.second);
    }
    else
    {
        for(auto it = condition.constBegin(); it != condition.constEnd(); it++)
        {
            sql_query.addBindValue(it.value());
        }
    }


    is_exec_query = sql_query.exec();



    qDebug() << "is_exec_query = " << is_exec_query;
}







