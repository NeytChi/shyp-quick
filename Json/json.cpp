#include "json.h"

Json::Json(SqlBuilder &_sql_builder_, bool &_is_valid_, ErrorResponse &_error_response_)
    : sql_builder(_sql_builder_), is_valid(_is_valid_), error_response(_error_response_)
{

}

Json::~Json()
{

}



QJsonObject Json::getObjectFromRecord(std::function<void(QJsonObject &)> function)
{
    QJsonObject json_object;

    const auto &rec = sql_builder.record();


    for(int i = 0; i < rec.count(); i++)
    {
        const auto value = rec.value(i);

        if(value.type() != QVariant::DateTime)
        {
            json_object.insert(rec.fieldName(i), value.toJsonValue());
        }
        else
        {
            const auto date = value.toDateTime();

            if(date.isValid())
            {
                json_object.insert(rec.fieldName(i), date.toSecsSinceEpoch());
            }
            else
            {
                json_object.insert(rec.fieldName(i), 0);
            }
        }
    }

    if(function.operator bool())
    {
        function(json_object);
    }

    return json_object;
}




std::tuple<bool, QString, bool>
Json::jsonValueValidation(const QString &key_name, const Validation_Type json_validation_type, const QJsonValue &json_value)
{
    bool have_custom_error = false;

    bool is_valid = false;

    QString error;

    enum class Type : char
    {
        Bool,
        Int,
        U_Int,
        Double,
        String,
        Json_Array
    };

    auto check_type = [&is_valid, &error](const QJsonValue &json_value, const Type type)
    {
        QString type_name;

        switch (type)
        {

        case Type::Bool:
            is_valid = json_value.isBool();

            type_name = "bool";
            break;

        case Type::Int:
            is_valid = json_value.isDouble();

            type_name = "int";
            break;

        case Type::U_Int:
            is_valid = json_value.isDouble();

            type_name = "uint";
            break;

        case Type::Double:
            is_valid = json_value.isDouble();

            type_name = "double";
            break;

        case Type::String:
            is_valid = json_value.isString();

            type_name = "string";
            break;

        case Type::Json_Array:
            is_valid = json_value.isArray();

            type_name = "array";
            break;

        }

        if(!is_valid)
        {
            error = ": key type must be " + type_name;
        }
    };


    static const QMap<Validation_Type, Type> map_type =
    {
        {Validation_Type::Common_String_Max_45, Type::String},
        {Validation_Type::Common_String_Max_255, Type::String},
        {Validation_Type::Common_String_Max_500, Type::String},
        {Validation_Type::Id, Type::U_Int},
        {Validation_Type::U_Int, Type::U_Int},
        {Validation_Type::Bool, Type::Bool},
        {Validation_Type::JsonArray, Type::Json_Array},
        {Validation_Type::Password, Type::String},
        {Validation_Type::Email, Type::String},
        {Validation_Type::Longitude, Type::Double},
        {Validation_Type::Latitude, Type::Double}
    };

    check_type(json_value, map_type.value(json_validation_type));


    if(is_valid)
    {
        switch (json_validation_type)
        {
        case Validation_Type::Common_String_Max_45:

            have_custom_error = true;

            std::tie(is_valid, error) = Validator::checkCommonString(key_name, json_value.toString(), 45);

            break;

        case Validation_Type::Common_String_Max_255:

            have_custom_error = true;

            std::tie(is_valid, error) = Validator::checkCommonString(key_name, json_value.toString(), 255);

            break;

        case Validation_Type::Common_String_Max_500:

            have_custom_error = true;

            std::tie(is_valid, error) = Validator::checkCommonString(key_name, json_value.toString(), 500);

            break;

        case Validation_Type::Id:

        {
            const double id_value = json_value.toDouble();

            is_valid = (static_cast<uint>(id_value) == id_value);

            if(is_valid)
            {
                is_valid = (id_value > 0);

                if(!is_valid)
                {
                    error = ": it must be > 0";
                }
            }
            else
            {
                error = ": value is not unsigned or value is out of range";
            }

            break;
        }

        case Validation_Type::U_Int:

        {
            const double u_value = json_value.toDouble();

            is_valid = (static_cast<uint>(u_value) == u_value);

            if(is_valid)
            {
                is_valid = (u_value >= 0);

                if(!is_valid)
                {
                    error = ": it must be >= 0";
                }
            }
            else
            {
                error = ": value is not unsigned or value is out of range";
            }

            break;
        }


        case Validation_Type::Bool:

            is_valid = true;
            break;

        case Validation_Type::JsonArray:

        {
            const auto json_array = json_value.toArray();

            is_valid = !json_array.isEmpty();

            if(!is_valid)
            {
                error = ": array is empty";
            }
            break;
        }

        case Validation_Type::Password:

            have_custom_error = true;

            std::tie(is_valid, error) = Validator::checkPassword(key_name, json_value.toString());
            break;

        case Validation_Type::Email:

            have_custom_error = true;

            std::tie(is_valid, error) = Validator::checkEmail(json_value.toString());
            break;


        case Validation_Type::Longitude:

            std::tie(is_valid, error) = Validator::checkLongitude(json_value.toDouble());

            break;

        case Validation_Type::Latitude:

            std::tie(is_valid, error) = Validator::checkLatitude(json_value.toDouble());

            break;



        }
    }


    return {is_valid, error, have_custom_error};
}

