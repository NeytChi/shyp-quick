#include "abstractrequesthandler.h"



const int AbstractRequestHandler::activationSignUp_time_s = 900;
const int AbstractRequestHandler::recoveryPass_block_time_s = 900;
const int AbstractRequestHandler::recoveryPass_hash_time_s = 600;

QMultiMap<QString, QVariant> AbstractRequestHandler::map_captured_request;


AbstractRequestHandler::AbstractRequestHandler(SqlBuilder &_sql_builder_, Response &_response_, const Send_Response_Function _send_response_function_)
    : sql_builder(_sql_builder_),
      json(sql_builder, is_valid, error_response),
      checker(sql_builder, json, is_valid, error_response, request_body, Method::Unknown),
      response(_response_),
      send_response_function(_send_response_function_)
{
    qDebug() << "\r\n" << __FUNCTION__ << "()";
}

AbstractRequestHandler::~AbstractRequestHandler()
{
    qDebug() << "\r\n" << __PRETTY_FUNCTION__;
}





void AbstractRequestHandler::setProperties(unsigned short url_index, UserData &&user_data, RequestBody &&request_body, Method method, const QString &client_ip_address)
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";


    this->url_index = url_index;

    this->user_data = std::move(user_data);

    this->request_body = std::move(request_body);


    checker.setMethod(method);

    this->client_ip_address = client_ip_address;
}


bool AbstractRequestHandler::captureRequest(Request_To_Capture &&request_to_capture)
{
    bool is_captured = false;

    if(current_request_to_capture.isEmpty())
    {
        if(!map_captured_request.contains(request_to_capture.function_name, request_to_capture.unique_value))
        {
            is_captured = true;

            current_request_to_capture = std::move(request_to_capture);

            map_captured_request.insert(current_request_to_capture.function_name, current_request_to_capture.unique_value);
        }
    }

    return is_captured;
}

void AbstractRequestHandler::uncaptureCurrentRequest()
{
    if(!current_request_to_capture.isEmpty())
    {
        map_captured_request.remove(current_request_to_capture.function_name, current_request_to_capture.unique_value);

        current_request_to_capture = Request_To_Capture();
    }
}


void AbstractRequestHandler::finishHandle()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    if(response.getResponseType() != Response::Response_Type::File)
    {
        //
        if(response.statusCode() == Response::HTTP_Status_Code::Unknown_Error)
        {
            if(sql_builder.isExec())
            {
                if(!error_response.isHaveError())
                {
                    response.setStatusCode(Response::HTTP_Status_Code::OK);

                    response.setData(std::move(response_body));
                }
                else
                {
                    response.setStatusCode(Response::HTTP_Status_Code::Bad_Request);

                    response.setData(std::move(error_response));
                }
            }
            else
            {
                response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                error_response.insertError(response.getStatusCodeValue());

                response.setData(std::move(error_response));
            }
        }
        else
        {
            error_response.insertError(response.getStatusCodeValue());

            response.setData(std::move(error_response));
        }
    }
    else
    {
        response.setStatusCode(Response::HTTP_Status_Code::OK);
    }

    uncaptureCurrentRequest();

    send_response_function();

    delete this;
}





QByteArray AbstractRequestHandler::generateRandomString()
{
    const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    thread_local std::mt19937 randomEngine(QRandomGenerator::global()->generate());
    std::uniform_int_distribution<int> distribution(0, sizeof(characters) - 2);
    QByteArray data;
    data.reserve(16);
    for (quint8 i = 0; i < 16; ++i)
    {
        data.append(characters[distribution(randomEngine)]);
    }

    return data;
}


void AbstractRequestHandler::checkEmailExist(const QString &email)
{
    bool is_exist = false;

    sql_builder.selectFromDB("1", "user", "email", email, is_exist);

    if(sql_builder.isExec())
    {
        is_valid = !is_exist;

        if(!is_valid)
        {
            error_response.insertError("such email already exists", "email", email);
        }
    }
}



void AbstractRequestHandler::checkClientNotHaveOpenedOrder(const QString &error)
{
    const QString select = sql_builder.getSelectString("1", "orders", "client_rated IS NULL AND state != ? AND id_client");

    sql_builder.execQuery(select, {Order_State::Canceled, user_data.user_Id()});

    if(sql_builder.isExec())
    {
        is_valid = !sql_builder.isNext();

        if(!is_valid)
        {
            error_response.insertError(error);
        }
    }
}






