#include "Checker/checker.h"



template<class Container>
void Checker::checkArrayOfId(const Check_Array<Container> &check_array, const std::optional<QVariant> &value, const QString &belong_table, const QString &id_name)
{
    constexpr bool is_json_array = std::is_same<typename std::decay<decltype (check_array.array)>::type, QJsonArray>::value;

    const QString real_id_name = (!id_name.isEmpty() ? (id_name + "s") : "ids");


    if(value.has_value())
    {
        sql_builder.execQuery(check_array.select, value.value());
    }
    else
    {
        sql_builder.execQuery(check_array.select);
    }

    if(sql_builder.isExec())
    {
        // id - (is_deleted || false)
        QMap<int, bool> map_belong;

        while(sql_builder.isNext())
        {
            map_belong.insert(sql_builder.getValue(0).toInt(), ((check_array.check_is_deleted == Check_Is_Deleted::Yes) ? sql_builder.getValue(1).toBool() : false));
        }

        is_valid = !map_belong.isEmpty();

        if(is_valid)
        {
            QJsonArray not_belong_list, already_deleted_list;


            // Check is all id's belong and not deleted
            for(auto it = check_array.array.constBegin(); it != check_array.array.constEnd(); it++)
            {
                int id_number = 0;

                if constexpr(is_json_array)
                {
                    id_number = (*it).toInt();
                }
                else
                {
                    id_number = it.key();
                }


                auto it_belong = map_belong.constFind(id_number);

                if(it_belong != map_belong.constEnd())
                {
                    if((check_array.check_is_deleted == Check_Is_Deleted::Yes) && it_belong.value())
                    {
                        already_deleted_list.append(id_number);
                    }
                }
                else
                {
                    not_belong_list.append(id_number);
                }
            }

            is_valid = not_belong_list.isEmpty() && already_deleted_list.isEmpty();

            // If error occurred
            if(!is_valid)
            {
                error_response.insertError("invalid " + check_array.array_name);

                if(!not_belong_list.isEmpty())
                {
                    error_response.insertErrorDescription("some " + real_id_name + " not belong to " + belong_table, "id_list", not_belong_list);
                }

                if(!already_deleted_list.isEmpty())
                {
                    error_response.insertErrorDescription("some " + real_id_name + " already deleted", "id_list", already_deleted_list);
                }
            }
        }
        else
        {
            error_response.insertError("invalid " + check_array.array_name + ": all " + real_id_name + " not belong to " + belong_table);
        }
    }

}



template<class Container>
QString Checker::getListId(const Container &container, const QString &field_in)
{
    QString list_id = field_in + " IN (";


    for(auto it = container.constBegin(); it != container.constEnd(); it++)
    {
        if constexpr(std::is_same<Container, QJsonArray>::value)
        {
            auto value = *it;

            int id_value = 0;

            if(value.isDouble())
            {
                id_value = value.toInt();
            }
            else if(value.isObject())
            {
                const auto object = value.toObject();

                id_value = object.value(field_in).toInt();
            }

            list_id.append(QString::number(id_value));
        }
        else if constexpr(std::is_same<Container, QVector<int>>::value)
        {
            list_id.append(QString::number(*it));
        }
        else
        {
            list_id.append(QString::number(it.key()));
        }

        if( (it + 1) != container.constEnd() )
        {
            list_id.append(", ");
        }
    }

    list_id.append(");");


    return list_id;
}


template<class Container>
QString Checker::getListId_Parse_(const Container &container, const QString &field_in, const QString &array_name, bool &is_valid, bool is_check_duplicate)
{
    QString list_id = field_in + " IN (";

    QMap<int, bool> map_id;


    QJsonArray invalid_list, duplicate_list;

    bool have_not_int_value = false;


    // Check array of id
    for(auto it = container.constBegin(); it != container.constEnd(); it++)
    {
        int id_number{};

        bool is_number_valid = false, is_value_int = false;

        if constexpr(std::is_same<Container, QJsonArray>::value)
        {
            is_value_int = (*it).isDouble();

            if(is_value_int)
            {
                id_number = (*it).toInt();
            }
        }
        else if constexpr(std::is_same<Container, QVector<int>>::value)
        {
            is_value_int = true;

            id_number = *it;
        }
        else
        {
            is_value_int = true;

            id_number = it.key();
        }

        // Check if we have value type = int
        if(is_value_int)
        {
            // Check if we have valid id
            is_number_valid = (id_number > 0);

            if(is_number_valid)
            {
                bool have_duplicate = false;

                // Check if we have duplicate id
                if(is_check_duplicate)
                {
                    have_duplicate = map_id.contains(id_number);
                }

                if(have_duplicate)
                {
                    duplicate_list.append(id_number);
                }
                else
                {
                    map_id.insert(id_number, true);

                    list_id.append(QString::number(id_number));

                    if( (it + 1) != container.constEnd() )
                    {
                        list_id.append(", ");
                    }
                    else
                    {
                        list_id.append(");");
                    }
                }
            }
            else
            {
                invalid_list.append(id_number);
            }
        }
        else
        {
            have_not_int_value = true;
            break;
        }
    }


    is_valid = !have_not_int_value && invalid_list.isEmpty() && duplicate_list.isEmpty();


    // If error occurred
    if(!is_valid)
    {
        if(have_not_int_value)
        {
            error_response.insertError("invalid " + array_name + ": have element in array which not int");
        }
        else
        {
            error_response.insertError("invalid " + array_name);

            if(!invalid_list.isEmpty())
            {
                error_response.insertErrorDescription("have invalid ids", "id_list", invalid_list);
            }

            if(!duplicate_list.isEmpty())
            {
                error_response.insertErrorDescription("have duplicate ids", "id_list", duplicate_list);
            }
        }
    }

    return list_id;
}

