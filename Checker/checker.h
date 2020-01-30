#ifndef CHECKER_H
#define CHECKER_H

#include "Json/json.h"

#include "RequestBody/requestbody.h"

#include "url_list.h"

#include <optional>

class Checker
{
public:
    Checker(SqlBuilder &_sql_builder_, Json &_json_, bool &_is_valid_,
            ErrorResponse &_error_response_, RequestBody &_request_body_, Method _method_);

    ~Checker();

private:

    SqlBuilder &sql_builder;

    Json &json;

    bool &is_valid;

    ErrorResponse &error_response;

    RequestBody &request_body;

    Method method = Method::Unknown;

    using Map_json_check = Json::Map_json_check;

    using Map_required_json_check = Json::Map_required_json_check;

public:

    void setMethod(Method method) { this->method = method; }


    template<class Container>
    QString getListId(const Container &container, const QString &field_in);


    template<class Container>
    QString getListId_Parse_(const Container &container, const QString &field_in, const QString &array_name, bool &is_valid, bool is_check_duplicate = true);


    enum class Received_Id_Type : unsigned char
    {
        Bool,
        Id,
        U_Int
    };

    int checkReceivedId(const QString &parameter, const Received_Id_Type id_type);


    enum class Check_Is_Deleted : bool
    {
        Yes = true,
        No = false
    };


    /*!
     * \brief
     * const QJsonArray array,
     * const QString array_name,
     * const QString field_in,
     * const Check_Is_Deleted check_is_deleted,
     * QString select
     */

    template<class Container = QJsonArray>
    struct Check_Array
    {
        const Container &array;
        const QString &array_name;
        const QString &field_in;
        const Check_Is_Deleted check_is_deleted;
        QString select = "";
    };


    template<class Container = QJsonArray>
    void checkArrayOfId(const Check_Array<Container> &check_array, const std::optional<QVariant> &value, const QString &belong_table, const QString &id_name = "");

};

#include "checker.tpp"

#endif // CHECKER_H
