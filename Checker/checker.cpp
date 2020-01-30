#include "checker.h"


Checker::Checker(SqlBuilder &_sql_builder_, Json &_json_, bool &_is_valid_,
                 ErrorResponse &_error_response_, RequestBody &_request_body_, Method _method_)

    : sql_builder(_sql_builder_),
      json(_json_),
      is_valid(_is_valid_),
      error_response(_error_response_),
      request_body(_request_body_),
      method(_method_)
{
    qDebug() << "Checker()";
}

Checker::~Checker()
{
    qDebug() << "~Checker()";
}



int Checker::checkReceivedId(const QString &parameter, const Received_Id_Type id_type)
{
    int id_value = 0;

    auto validate_number = [this, &id_value, &parameter, id_type](auto iterator)
    {
        constexpr bool is_object = std::is_same<decltype (iterator), QJsonObject::const_iterator>::value;

        if constexpr(!is_object)
        {
            id_value = iterator.value().toInt(&is_valid);
        }
        else
        {
            if(id_type == Received_Id_Type::Bool)
            {
                is_valid = iterator.value().isBool();
            }
            else
            {
                is_valid = iterator.value().isDouble();
            }
        }

        if(is_valid)
        {
            if constexpr(is_object)
            {
                if(id_type == Received_Id_Type::Bool)
                {
                    id_value = iterator.value().toBool(0);
                }
                else
                {
                    id_value = iterator.value().toInt(0);
                }
            }

            QString error;

            switch (id_type)
            {
            case Received_Id_Type::Bool:

                is_valid = (id_value == 0) || (id_value == 1);

                error = "== 0 or == 1";

                break;

            case Received_Id_Type::Id:

                is_valid = (id_value > 0);

                error = "> 0";

                break;

            case Received_Id_Type::U_Int:

                is_valid = (id_value >= 0);

                error = ">= 0";

                break;
            }

            if(!is_valid)
            {
                error_response.insertError("invalid " + parameter + ": it must be " + error, parameter, iterator.value());
            }
        }
        else
        {
            error_response.insertError("invalid " + parameter + ": key type must be int", parameter, iterator.value());
        }
    };

    if(method == Method::Get)
    {
        auto it = request_body.map_get_parameter.constFind(parameter);

        is_valid = (it != request_body.map_get_parameter.constEnd());

        if(is_valid)
        {
            validate_number(it);
        }
        else
        {
            error_response.insertError("key <" + parameter + "> not found, but required");
        }
    }
    else if(method == Method::Post)
    {
        auto it = request_body.received_json.constFind(parameter);

        is_valid = (it != request_body.received_json.constEnd());

        if(is_valid)
        {
            validate_number(it);
        }
        else
        {
            error_response.insertError("key <" + parameter + "> not found, but required");
        }
    }

    return id_value;
}


