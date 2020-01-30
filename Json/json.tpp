#include "json.h"



template<class Value>
QJsonObject Json::getObjectFromQuery(const QString &query, const Value &value)
{
    QJsonObject json_object;

    sql_builder.execQuery(query, value);


    if(sql_builder.isExec())
    {
        is_valid = sql_builder.isNext();

        if(is_valid)
        {
            json_object = getObjectFromRecord();
        }
    }

    return json_object;
}



template<class Value>
QJsonArray Json::getObjectListFromQuery(const QString &query, const Value &value, std::function<void(QJsonObject &)> function)
{
    QJsonArray json_array;

    sql_builder.execQuery(query, value);

    if(sql_builder.isExec())
    {
        while(sql_builder.isNext())
        {
            json_array.append(getObjectFromRecord(function));
        }
    }

    return json_array;
}





template<class Json_Template, class Result>
bool Json::validateJson(const Json_Template &json_template, Result &result, const QJsonObject &json_data, Is_Required_Validation json_is_required_validation, const QString &values_location)
{
    qDebug() << "\r\n------ " << __FUNCTION__ << " ------\r\n";

    bool is_valid = true;

    // {key, Validation_Type::Id}                                           -> pair - Validation_Type
    // map of {key, Validation_Type::Id}                                    -> map -  Validation_Type
    // map of {key, Validation_Type::Id, Is_Required_Validation::Required}  -> map -  Json_Validation


    constexpr bool is_one_json_pair = (std::is_same<Json_Template, QPair<QString, Validation_Type>>::value && std::is_same<Result, QVariant>::value);



    constexpr bool is_value_validation_logic_structure = (std::is_same<Json_Template, QMap<QString, Validation_Logic_Structure>>::value && std::is_same<Result, QVariantMap>::value);

    // Check key and result
    {
        constexpr bool is_value_validation_type = (std::is_same<Json_Template, QMap<QString, Validation_Type>>::value && std::is_same<Result, QVariantMap>::value);

        static_assert (is_one_json_pair || is_value_validation_logic_structure || is_value_validation_type, "Invalid Type");
    }


    auto checkValue = [this, &values_location](bool &is_valid, const QString &key_name, const QJsonValue &json_value, const Validation_Type json_validation_type)
    {
        if(is_valid)
        {
            bool have_custom_error = false;

            QString validation_error;

            is_valid = !json_value.isNull();

            if(is_valid)
            {
                std::tie(is_valid, validation_error, have_custom_error) = jsonValueValidation(key_name, json_validation_type, json_value);
            }
            else
            {
                validation_error = ": it's NULL";
            }


            if(!is_valid)
            {
                QString error_message;

                if(have_custom_error)
                {
                    error_message = validation_error;
                }
                else
                {
                    error_message = "invalid " + key_name;

                    if(!values_location.isEmpty())
                    {
                        error_message.append(" in ").append(values_location);
                    }

                    error_message.append(validation_error);
                }

                error_response.insertError(error_message, key_name, json_value);
            }

            qDebug() << "json_value = " << json_value;
        }
        else
        {
            QString error_message = "key <" + key_name + "> not found";

            if(!values_location.isEmpty())
            {
                error_message.append(" in ").append(values_location);
            }

            error_message.append(", but required");

            error_response.insertError(error_message);
        }
    };


    if constexpr(is_one_json_pair)
    {
        const QString key_name = json_template.first;

        qDebug() << "Key = " << key_name;

        // Check does specified key exist in json_data

        auto it = json_data.constFind(key_name);

        is_valid = (it != json_data.constEnd());

        checkValue(is_valid, key_name, it.value(), json_template.second);

        if(is_valid)
        {
            result = it.value().toVariant();
        }

        qDebug() << " is_valid = " << is_valid;
    }
    else
    {
        bool have_one_key = false;

        for(auto it = json_template.constBegin(); it != json_template.constEnd(); it++)
        {
            const QString key_name = it.key();

            qDebug() << "Key = " << key_name;

            // Check does specified key exist in json_data

            auto it_value = json_data.constFind(key_name);

            const bool is_key_exist = (it_value != json_data.constEnd());

            Validation_Type json_validation_type;

            if constexpr(is_value_validation_logic_structure)
            {
                const Validation_Logic_Structure validation_logic_structure = it.value();


                if(!is_key_exist && ( (json_is_required_validation == Is_Required_Validation::Not_Required) ||
                                   (validation_logic_structure.is_required_validation == Is_Required_Validation::Not_Required) ))
                {
                    continue;
                }

                json_validation_type = validation_logic_structure.json_validation_type;
            }
            else
            {
                if(!is_key_exist && (json_is_required_validation == Is_Required_Validation::Not_Required))
                {
                    continue;
                }

                json_validation_type = it.value();
            }

            is_valid = is_key_exist;

            checkValue(is_valid, key_name, it_value.value(), json_validation_type);

            if(is_valid)
            {
                result.insert(key_name, it_value.value().toVariant());
            }

            have_one_key = true;

            qDebug() << " is_valid = " << is_valid;

            if(!is_valid)
            {
                break;
            }
        }

        if(is_valid && !have_one_key)
        {
            is_valid = false;

            error_response.insertError("no required keys found");
        }
    }


    return is_valid;
}
