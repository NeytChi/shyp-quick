#ifndef ABSTRACTREQUESTHANDLER_H
#define ABSTRACTREQUESTHANDLER_H


#include <QDebug>

#include <QCryptographicHash>
#include <QRandomGenerator>
#include <qpassworddigestor.h>
#include <QDir>


#include "UserData/userdata.h"

#include "Checker/checker.h"

#include "Response/response.h"

#include "ForeignApi/foreignapihandler.h"

class AbstractRequestHandler
{
protected:

    using Send_Response_Function = std::function<void()>;

public:
    AbstractRequestHandler(SqlBuilder &_sql_builder_, Response &_response_, const Send_Response_Function _send_response_function_);

    virtual ~AbstractRequestHandler();

    virtual void handle() = 0;

    /*!
     * \brief method used for Checker.checkReceivedId()
     */
    void setProperties(unsigned short url_index, UserData &&user_data, RequestBody &&request_body, Method method, const QString &client_ip_address);

protected:

    SqlBuilder &sql_builder;

    unsigned short url_index = 0;

    bool is_valid = false;

    bool is_foreign_api_using = false;

    UserData user_data;

    RequestBody request_body;

    ErrorResponse error_response;

    Json json;

    Checker checker;

    QJsonObject response_body;

    Response &response;


    Send_Response_Function send_response_function;

    QString client_ip_address; // For creating stripe custom account - StripeApi::createAccount - tos_acceptance[ip]

    static QMultiMap<QString, QVariant> map_captured_request;

    struct Request_To_Capture
    {
        QString function_name;
        QVariant unique_value;

        bool isEmpty() { return function_name.isEmpty(); }

    } current_request_to_capture;


    bool captureRequest(Request_To_Capture &&request_to_capture);

    void uncaptureCurrentRequest();



    inline bool is_all_valid() const { return (is_valid && sql_builder.isExec()); }


    using Map_json_check = Json::Map_json_check;

    using Map_required_json_check = Json::Map_required_json_check;



    template<class MapFunction, class Child>
    void additionalHandle(const MapFunction &map_function, unsigned short url_index, Child *child)
    {
        const auto handle = map_function.value(url_index, nullptr);

        const bool is_get_function = handle.operator bool();

        if(is_get_function)
        {
            handle(child);

            if(!is_foreign_api_using)
            {
                finishHandle();
            }
        }
        else
        {
            response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

            const QJsonObject log_error =
            {
                {"error_description", "invalid type handle, need fill handle_func_map"}
            };

            error_response.insertLogError(log_error);

            finishHandle();
        }
    }


    void finishHandle();


    static const int activationSignUp_time_s;
    static const int recoveryPass_block_time_s;
    static const int recoveryPass_hash_time_s;



    QByteArray generateRandomString();



    //---Other---//

    QString getUrlPicturePath(const QString &picture_name) const
    {
        return Global::getServerUrl() + "/profile_picture/" + picture_name;
    }

    void checkEmailExist(const QString &email);


public:
    struct Order_State
    {
        enum : unsigned char
        {
            Finding_Shipper,
            Accepted,
            In_Shipping,
            Completed, // = closed order
            Canceled // = closed order
        };
    };


    struct Shipper_Order_State
    {
        enum : unsigned char
        {
            Does_Not_Has_Order,
            Getted_Order, // So that the shipper does not receive two orders
            Has_Accepted_Order,
            Has_Completed_Order_But_Not_Rated // If he has an order in which he did not put a rating, then he should not receive orders
        };
    };

    void checkClientNotHaveOpenedOrder(const QString &error);

    struct Order_Charge_Status
    {
        enum : unsigned char
        {
            Authorize,
            Captured
        };
    };

};

#endif // ABSTRACTREQUESTHANDLER_H
