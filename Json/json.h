#ifndef ABSTRACTJSON_H
#define ABSTRACTJSON_H

#include <QDebug>

#include <QJsonObject>
#include <QJsonArray>

#include <QDateTime>


#include "SqlBuilder/sqlbuilder.h"

#include "validator.h"

#include "Response/ErrorResponse/errorresponse.h"

class Json
{
public:
    Json(SqlBuilder &_sql_builder_, bool &_is_valid_, ErrorResponse &_error_response_);

    ~Json();

private:

    SqlBuilder &sql_builder;

    bool &is_valid;

    QJsonObject getObjectFromRecord(std::function<void(QJsonObject &)> function = nullptr);

    ErrorResponse &error_response;

public:

    template<class Value = QVector<QVariant>>
    QJsonObject getObjectFromQuery(const QString &query, const Value &value);


    template<class Value = QVector<QVariant>>
    QJsonArray getObjectListFromQuery(const QString &query, const Value &value, std::function<void(QJsonObject &)> function = nullptr);


    enum class Validation_Type : char
    {
        Common_String_Max_45,
        Common_String_Max_255,
        Common_String_Max_500,
        Id,
        U_Int,
        Bool,
        JsonArray,
        Password,
        Email,
        Longitude,
        Latitude
    };

    enum class Is_Required_Validation : bool
    {
        Required = true,
        Not_Required = false
    };


    struct Validation_Logic_Structure
    {
        Validation_Type json_validation_type;

        Is_Required_Validation is_required_validation = Is_Required_Validation::Required;
    };


    typedef const QMap<QString, Validation_Logic_Structure> Map_json_check;

    typedef const QMap<QString, Validation_Type> Map_required_json_check;



    std::tuple<bool, QString, bool>
    jsonValueValidation(const QString &key_name, const Validation_Type json_validation_type, const QJsonValue &json_value);


    template<class Json_Template = QPair<QString, Validation_Type>, class Result>
    bool validateJson(const Json_Template &json_template, Result &result, const QJsonObject &json_data, Is_Required_Validation json_is_required_validation, const QString &values_location = "");

};

#include "json.tpp"

#endif // ABSTRACTJSON_H
