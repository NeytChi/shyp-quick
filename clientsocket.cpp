#include "clientsocket.h"


const int ClientSocket::max_headers_size = 8000;

// default Method = Post
const QHash<QString, ClientSocket::Url_Settings> ClientSocket::available_no_auth_urls =
{
    //User
        //Post
    {
        "/login", {Url_Post::User_Login_NA, Url_Type::User}
    },
    {
        "/oauth_login", {Url_Post::User_OAuth_Login_NA, Url_Type::User}
    },
    {
        "/check_sign_up", {Url_Post::User_Check_Sign_Up_NA, Url_Type::User}
    },
    {
        "/sign_up_client", {Url_Post::User_Sign_Up_Client_NA, Url_Type::User}
    },
    {
        "/sign_up_driver", {Url_Post::User_Sign_Up_Driver_NA, Url_Type::User}
    },
    {
        "/activation_sign_up", {Url_Post::User_Activation_Sign_Up_NA, Url_Type::User}
    },
    {
        "/recovery_pass", {Url_Post::User_Recovery_Pass_NA, Url_Type::User}
    },
    {
        "/send_new_pass", {Url_Post::User_Send_New_Pass_NA, Url_Type::User}
    },
    {
        "/resend_sms_code", {Url_Post::User_Resend_Sms_Code_NA, Url_Type::User}
    },


        // Get
    {
        "/profile_picture", {Url_Get::User_Profile_Picture_NA, Url_Type::User, Method::Get}
    },
    {
        "/state_zip_code", {Url_Get::User_Get_Zip_Code_State_NA, Url_Type::User, Method::Get}
    },
    {
        "/car_model_list", {Url_Get::User_Get_Car_Model_List_NA, Url_Type::User, Method::Get}
    }

};



const QHash<QString, ClientSocket::Url_Settings> ClientSocket::available_auth_urls =
{
    //User
        //Post
    {
        "/change_profile_picture", {Url_Post::User_Change_Profile_Picture, Url_Type::User}
    },
    {
        "/change_profile", {Url_Post::User_Change_Profile, Url_Type::User}
    },
    {
        "/change_password", {Url_Post::User_Change_Password, Url_Type::User}
    },
    {
        "/contact_us", {Url_Post::User_Contact_Us, Url_Type::User}
    },
    {
        "/add_push_token", {Url_Post::User_Add_Push_Token, Url_Type::User}
    },
    {
        "/save_payment_method", {Url_Post::User_Save_Payment_Method, Url_Type::User}
    },

        //Get
    {
        "/client_menu", {Url_Get::User_Client_Menu, Url_Type::User, Method::Get}
    },
    {
        "/driver_menu", {Url_Get::User_Driver_Menu, Url_Type::User, Method::Get}
    },
    {
        "/profile", {Url_Get::User_Profile, Url_Type::User, Method::Get}
    },
    {
        "/logout", {Url_Get::User_Logout, Url_Type::User, Method::Get}
    },
    {
        "/payment_method", {Url_Get::User_Get_Payment_Method, Url_Type::User, Method::Get}
    },
    {
        "/check_auth_token", {Url_Get::User_Check_Auth_Token, Url_Type::User, Method::Get}
    },
    //---------------------

    //Order
        //Post
    {
        "/create_order", {Url_Post::Order_Client_Create_Order, Url_Type::Order}
    },
    {
        "/calc_order", {Url_Post::Order_Client_Calc_Order, Url_Type::Order}
    },
    {
        "/cancel_order", {Url_Post::Order_Client_Cancel_Order, Url_Type::Order}
    },
    {
        "/accept_order", {Url_Post::Order_Shipper_Accept_Order, Url_Type::Order}
    },
    {
        "/decline_order", {Url_Post::Order_Shipper_Decline_Order, Url_Type::Order}
    },
    {
        "/on_pick_up_point", {Url_Post::Order_Shipper_On_Pick_Up_Point, Url_Type::Order}
    },
    {
        "/on_drop_off_point", {Url_Post::Order_Shipper_On_Drop_Off_Point, Url_Type::Order}
    },
    {
        "/update_coordinates", {Url_Post::Order_Shipper_Update_Coordinates, Url_Type::Order}
    },
    {
        "/ready_to_ship", {Url_Post::Order_Shipper_Ready_To_Ship, Url_Type::Order}
    },
    {
        "/rate_user", {Url_Post::Order_Rate_User, Url_Type::Order}
    },
        //Get
    {
        "/check_exist_new_order", {Url_Get::Order_Shipper_Check_Exist_New_Order, Url_Type::Order, Method::Get}
    },
    {
        "/order_state", {Url_Get::Order_Client_Get_Order_State, Url_Type::Order, Method::Get}
    },
    {
        "/last_completed_orders", {Url_Get::Order_Client_Last_Completed_Orders, Url_Type::Order, Method::Get}
    },
    {
        "/stats", {Url_Get::Order_Shipper_Get_Stats, Url_Type::Order, Method::Get}
    },
    {
        "/get_paid", {Url_Get::Order_Shipper_Get_Paid, Url_Type::Order, Method::Get}
    },
    {
        "/order_list", {Url_Get::Order_Get_Order_List, Url_Type::Order, Method::Get}
    },
    {
        "/order_by_id", {Url_Get::Order_Get_Order_By_Id, Url_Type::Order, Method::Get}
    }


};


const QHash<QString, ClientSocket::Required_headers> ClientSocket::required_headers =
{
    {
        "authorization", Required_headers::Authorization
    },
    {
        "content-length", Required_headers::Content_Length
    },
    {
        "content-type", Required_headers::Content_Type
    }
};

const QHash<QString, ClientSocket::Supported_media_types> ClientSocket::available_media_types =
{
    {
        "application/x-www-form-urlencoded", Supported_media_types::App_FormUrlEncoded
    },
    {
        "multipart/form-data", Supported_media_types::Multipart_FormData
    },
    {
        "application/json", Supported_media_types::JSON
    }
};


ClientSocket::ClientSocket(SqlBuilder *const _sql_builder_, QObject *parent)
    : QObject(parent), sql_builder(_sql_builder_)
{

}

ClientSocket::~ClientSocket()
{

}

void ClientSocket::process(const qintptr socket_descriptor)
{
    socket = new QTcpSocket(this);


    if(socket->setSocketDescriptor(socket_descriptor))
    {
        QObject::connect(socket, &QTcpSocket::readyRead, this, &ClientSocket::onReadyRead);

        connect(socket, &QTcpSocket::disconnected, this, &ClientSocket::onDisconnect);

        connect(socket, &QTcpSocket::destroyed, this, []()
        {
            qDebug() << "socket destroyed";
        });


        connect(socket, &QTcpSocket::stateChanged, [&](QAbstractSocket::SocketState state)
        {
            qDebug() << state;
        });

        connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [&](QAbstractSocket::SocketError socketError)
        {
            qDebug() << socketError;
        });

        connect(&connection_duration, &QTimer::timeout, [&]()
        {
            qDebug() << "TIMEOUT !_!" << QDateTime::currentDateTime();
            socket->close();
        });

        connection_duration.start(60000);
    }
    else
    {
        const QJsonObject error_description =
        {
            {QStringLiteral("type"), QStringLiteral("Internal Server Error")},
            {"error_description", "can't set socket descriptor"},
            {QStringLiteral("socket_error"), socket->errorString()},
            {QStringLiteral("socket_descriptor"), socket_descriptor}
        };

        Logger::writeLogInFile(error_description, __PRETTY_FUNCTION__);

        onDisconnect();
    }

}

void ClientSocket::writeLogs(const ErrorResponse &error_response)
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    QVariantMap map_log;

    QString headers;

    for(const auto &header : request_headers.vec_received)
    {
        headers.append(header).append("\r\n");
    }

    map_log.insert("headers", headers);

    map_log.insert("url", request_headers.path);


    // Description
    {
        QString description;

        if(response.statusCode() != Response::HTTP_Status_Code::Internal_Server_Error)
        {
            description = error_response.getError();
        }
        else
        {
            description = sql_builder->lastError();

            if(description.isEmpty())
            {
                description = QStringLiteral("server error");
            }
        }

        map_log.insert(QStringLiteral("description"), description);
    }

    map_log.insert("type", response.getStatusCodeValue());
    map_log.insert("success", false);
    map_log.insert("date", QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));

    // prepared_query and prepared_values
    if(response.statusCode() == Response::HTTP_Status_Code::Internal_Server_Error)
    {
        QString prepared_query = sql_builder->lastQuery();


        if(!prepared_query.isEmpty())
        {
            map_log.insert(QStringLiteral("prepared_query"), prepared_query);

            map_log.insert(QStringLiteral("prepared_values"),
                           QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(sql_builder->boundValues())).toJson(QJsonDocument::JsonFormat::Compact)));
        }
    }


    // not_valid_headers
    {
        auto it = request_headers.map_errors.constFind(Request_Errors::Not_Valid_Header);

        if(it != request_headers.map_errors.constEnd())
        {
            map_log.insert(QStringLiteral("not_valid_headers"), it.value());
        }
    }


    // not_valid_values
    map_log.insert(QStringLiteral("not_valid_values"), error_response.getLogError());



    sql_builder->insertIntoDB(map_log, QStringLiteral("log"));


    // Write log in file
    if(!sql_builder->isExec())
    {
        qDebug() << sql_builder->lastError() << "Can't insert log in db";

        const QJsonObject error_description =
        {
            {"type", "Internal Server Error"},
            {"error_description", "can't write log in database"},
            {"query_error", sql_builder->lastError()},
            {"prepared_query", sql_builder->lastQuery()},
            {
                QStringLiteral("prepared_values"),
                QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(sql_builder->boundValues())).toJson(QJsonDocument::JsonFormat::Compact))
            },

            {"log_that_tried_to_record", QJsonObject::fromVariantMap(map_log)}
        };

        Logger::writeLogInFile(error_description, __PRETTY_FUNCTION__);
    }

}



bool ClientSocket::checkStartLine(const QString &start_line)
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    // Split the start line on three parts

    QVector<QStringRef> vec_ref = start_line.splitRef(QLatin1Char(' '));

    if(vec_ref.size() != 3)
    {
        vec_ref = start_line.splitRef(QLatin1Char('\t'));

        if(vec_ref.size() != 3)
        {
            request_headers.map_errors.insert(Request_Errors::Validation_Header_Error, QStringLiteral("Start line of header is wrong"));
            return false;
        }

    }


    // Checking the version

    if(vec_ref.last() != QLatin1String("HTTP/1.1"))
    {
        response.setStatusCode(Response::HTTP_Status_Code::HTTP_Version_Not_Supported);
        return false;
    }


    // Checking the method

    if(vec_ref.first() == QLatin1String("GET"))
    {
        method = Method::Get;
    }
    else if (vec_ref.first() == QLatin1String("POST"))
    {
        method = Method::Post;
    }
    else
    {
        response.setStatusCode(Response::HTTP_Status_Code::Not_Implemented);
        return false;
    }

    // Checking the path


    request_headers.path = QByteArray::fromPercentEncoding(vec_ref.at(1).toUtf8()).replace('+', ' ').simplified();

    if(request_headers.path.size() > 255)
    {
        response.setStatusCode(Response::HTTP_Status_Code::URI_Too_Long);

        request_headers.path.chop(request_headers.path.size() - 255);
    }

    qDebug() << "Path = " << request_headers.path;

    if(request_headers.path.at(0) != QLatin1Char('/'))
    {
        request_headers.map_errors.insert(Request_Errors::Validation_Header_Error, QStringLiteral("Wrong path"));
    }

    return true;
}

// Header size restriction = 8KB
void ClientSocket::onReadyRead()
{
    qDebug() << "\r\n------ Read request headers ------\r\n";

    if(handle_state == Handle_State::Handling)
    {
        return;
    }

    connection_duration.start(60000);



    // Read request headers

    while(state != State::ReadBody)
    {
        if(!socket->canReadLine())
        {
            return;
        }

        QString received_line;

        // Read start line

        if(state == State::ReadStartLine)
        {
            received_line = socket->readLine().simplified();

           // Ignore any empty line received where a Request-Line is expected

           if(received_line.isEmpty())
           {
               received_line = socket->readLine().simplified();
           }

           // Check all headers size
           request_headers.headers_size += received_line.size();

           if(request_headers.headers_size > max_headers_size)
           {
               response.setStatusCode(Response::HTTP_Status_Code::Payload_Too_Large);
               break;
           }

           qDebug() << received_line;

           bool is_valid_start_line = checkStartLine(received_line);

           if(!is_valid_start_line)
           {
               request_headers.map_errors.insert(Request_Errors::Not_Valid_Header, received_line + QStringLiteral("\r\n"));
           }

           state = State::ReadHeaders;

        }

        if(state == State::ReadHeaders)
        {
            qDebug() << "\r\n------ Read Headers ------\r\n";

            //Read other headers

            while(socket->canReadLine())
            {
                received_line = socket->readLine().simplified();

                qDebug() << received_line;

                if(!received_line.isEmpty())
                {
                    // Check all headers size
                    request_headers.headers_size += received_line.size();

                    if(request_headers.headers_size > max_headers_size)
                    {
                        response.setStatusCode(Response::HTTP_Status_Code::Payload_Too_Large);
                        state = State::ReadBody;
                        break;
                    }

                    request_headers.vec_received.append(received_line);
                }
                // If we reached the end of the request
                else
                {
                    state = State::ReadBody;
                    break;
                }
            }

            // If we reached the end of the request
            if(state == State::ReadBody)
            {
                break;
            }
        }
    }



    if(!is_check_headers)
    {
        qDebug() << "\r\n------ Check Headers ------\r\n";

        // Check if we have errors when validate start line, parse headers, validate headers


        bool is_valid_headers = (response.statusCode() == Response::HTTP_Status_Code::Unknown_Error) &&
                                 request_headers.map_errors.isEmpty() &&
                                 parseHeaders() && validateHeaders();

        if(is_valid_headers)
        {
            if(method == Method::Post)
            {
                int contect_length = request_headers.map_required.value(Required_headers::Content_Length).toInt();

                if(contect_length > 0)
                {
                    request_data.reserve(contect_length);
                }
            }

            is_check_headers = true;
        }
        else
        {
            if(!request_headers.map_errors.contains(Request_Errors::Validation_Header_Error))
            {
                request_headers.map_errors.insert(Request_Errors::Validation_Header_Error, response.getStatusCodeValue());
            }
            else
            {
                response.setStatusCode(Response::HTTP_Status_Code::Bad_Request);
            }

            ErrorResponse error_response;

            error_response.insertError(request_headers.map_errors.value(Request_Errors::Validation_Header_Error));

            response.setData(std::move(error_response));


            connection_duration.stop();

            sendResponse();
            return;
        }
    }



    // Read data from socket
    if(method == Method::Post)
    {
        qDebug() << "\r\n------ Read data from socket ------\r\n";


        int contect_length = request_headers.map_required.value(Required_headers::Content_Length).toInt();

        if(contect_length > 0)
        {
            if(socket->bytesAvailable() > 0)
            {
                if( (socket->bytesAvailable() + request_data.size()) <= contect_length)
                {
                    request_data.append(socket->readAll());
                }
                else
                {
                    request_data.append(socket->read(contect_length - request_data.size()));
                }
                if(request_data.size() != contect_length)
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
    }


    qDebug() << "Content length = " << request_headers.map_required.value(Required_headers::Content_Length)
             << "Data size = " << request_data.size();


    if( (method == Method::Post) || (method == Method::Get) )
    {
        connection_duration.stop();

        handle_state = Handle_State::Handling;

        handleRequest();

        return;
    }
}


bool ClientSocket::parseHeaders()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    bool is_valid_header = true;

    QString not_valid_header;

    Response::HTTP_Status_Code error_code = Response::HTTP_Status_Code::Unknown_Error;

    for(const auto &header : request_headers.vec_received)
    {
        // Content-Type: application/json;

        QStringRef name;
        QStringRef value;

        // Separate on name and value

        {
            const int sep = header.indexOf(QLatin1Char(':'));

            // Check if name have ':' and min one letter;

            is_valid_header = (sep != -1) && (header.at(0).isLetter());

            if(is_valid_header)
            {
                name = header.leftRef(sep);

                // If have value -> header_name: value
                //                               ^ - sep

                is_valid_header = (sep + 2) <= (header.size() - 1);

                if(is_valid_header)
                {
                    value = header.midRef(sep + 2);
                }
            }

            if(!is_valid_header)
            {
                not_valid_header = header.left(20);

                request_headers.map_errors.insert(Request_Errors::Validation_Header_Error,
                                                  QStringLiteral("Wrong Header = ") + header.left(20));
                break;
            }
        }


        // To lower, because name of header may be case insesitive

        Required_headers required_header = required_headers.value(name.toString().toLower(), Required_headers::Unknown);

        if(required_header != Required_headers::Unknown)
        {
            QString header_value;

            // Authorization: Bearer token
            if(required_header == Required_headers::Authorization)
            {
                QVector<QStringRef> vec_value_ref = value.split(QLatin1Char(' '));

                is_valid_header = (vec_value_ref.size() == 2);

                if(!is_valid_header)
                {
                    vec_value_ref = value.split(QLatin1Char('\t'));

                    is_valid_header = (vec_value_ref.size() == 2);
                }

                if(is_valid_header)
                {
                    is_valid_header = (vec_value_ref.first() == QLatin1String("Bearer"));

                    if(is_valid_header)
                    {
                        header_value = vec_value_ref.last().toString();
                    }
                }
            }

            // Content-Type: application/json;
            // Content-Type: text/html; charset=utf-8
            else if(required_header == Required_headers::Content_Type)
            {
                auto vec_value_ref = value.split(QLatin1Char(' '));

                bool have_two_value = (vec_value_ref.size() == 2);

                if(!have_two_value)
                {
                    vec_value_ref = value.split('\t');

                    have_two_value = (vec_value_ref.size() == 2);
                }

                // Content-Type: multipart/form-data; boundary=Asrf456BGe4h
                // Checking boundary

                if(value.startsWith(QLatin1String("multipart/form-data")))
                {
                    is_valid_header = have_two_value;

                    if(is_valid_header)
                    {
                        QStringRef boundary_value = vec_value_ref.last();

                        const int sep = boundary_value.indexOf(QLatin1Char('='));

                        is_valid_header = (sep != -1);

                        if(is_valid_header)
                        {
                            request_headers.boundary = boundary_value.mid(sep + 1).toString();

                            is_valid_header = !request_headers.boundary.isEmpty();

                            if(is_valid_header)
                            {
                                request_headers.boundary.prepend("--");
                            }
                        }
                    }
                }

                if(is_valid_header)
                {
                    header_value = vec_value_ref.first().toString().replace(";","");

                    // Check if we know the type
                    is_valid_header = (available_media_types.value(header_value, Supported_media_types::Unknown) != Supported_media_types::Unknown);

                    if(!is_valid_header)
                    {
                        error_code = Response::HTTP_Status_Code::Unsupported_Media_Type;
                    }
                }
            }

            else if(required_header == Required_headers::Content_Length)
            {
                const int content_length = value.toInt(&is_valid_header);

                if(is_valid_header)
                {
                    // Check if Content-Length < 100 mb
                    is_valid_header = (content_length <= 104857600);

                    if(is_valid_header)
                    {
                        header_value = value.toString();
                    }
                    else
                    {
                        error_code = Response::HTTP_Status_Code::Payload_Too_Large;
                    }
                }
            }

            if(is_valid_header)
            {
                request_headers.map_required.insert(required_header, header_value);
            }
            else
            {
                // Write only one header error
                if(not_valid_header.isEmpty())
                {
                    not_valid_header = header;

                    // If response_status_code Not changed
                    if(response.statusCode() == Response::HTTP_Status_Code::Unknown_Error)
                    {
                        // If error_code changed
                        if(error_code != Response::HTTP_Status_Code::Unknown_Error)
                        {
                            response.setStatusCode(error_code);
                        }
                        // We have another error
                        else
                        {
                            request_headers.map_errors.insert(Request_Errors::Validation_Header_Error, QStringLiteral("Wrong Header = ") + name);
                        }
                    }
                }
            }
        }
    }

    if(!not_valid_header.isEmpty())
    {
        if(not_valid_header.size() > 200)
        {
            not_valid_header = not_valid_header.left(200);
        }
        request_headers.map_errors.insert(Request_Errors::Not_Valid_Header, not_valid_header + QStringLiteral("\r\n"));
    }


    return not_valid_header.isEmpty();
}


bool ClientSocket::validateHeaders()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";


    if(method == Method::Post)
    {
        // Check if contains Content-Length header

        if(!request_headers.map_required.contains(Required_headers::Content_Length))
        {
            response.setStatusCode(Response::HTTP_Status_Code::Length_Required);
            return false;
        }

        // Check if contains Content-Type header
        if(!request_headers.map_required.contains(Required_headers::Content_Type))
        {
            request_headers.map_errors.insert(Request_Errors::Validation_Header_Error, QStringLiteral("Not Found Content-Type header"));
            return false;
        }
    }

    // Get url_settings
    Url_Settings url_settings;


    QStringRef origin_path = &request_headers.path;

    // Get real path
    {
        /*
         * 1. http://127.0.0.1:27099/activation -> /activation
         * 2. http://127.0.0.1:27099/activation/../ -> /activation
         * 3. http://127.0.0.1:27099/signUpActivation?hash_code= -> /signUpActivation
        */

        int index = request_headers.path.indexOf('?', 1);

        if(index == -1)
        {
            index = request_headers.path.indexOf('/', 1);
        }

        if(index != -1)
        {
            origin_path = request_headers.path.midRef(0, index);
        }

        qDebug() << "Origin path = " << origin_path;
    }

    // Check if we found such url

    url_settings = available_auth_urls.value(origin_path.toString(), {0, Url_Type::Unknown, Method::Unknown});

    bool is_need_auth = false;

    // If we found this url in auth hash table
    if(url_settings.url_index != 0)
    {
        is_need_auth = true;
    }
    else
    {
        url_settings = available_no_auth_urls.value(origin_path.toString(), {0, Url_Type::Unknown, Method::Unknown});

        if(url_settings.url_index == 0)
        {
            response.setStatusCode(Response::HTTP_Status_Code::Not_Found);
            return false;
        }
    }


    request_headers.url_settings = url_settings;


    // Check if recieved method equal required method of url

    if(method != url_settings.url_method)
    {
        response.setStatusCode(Response::HTTP_Status_Code::Method_Not_Allowed);
        return false;
    }


    // Check if this url required authorization

    if(is_need_auth)
    {
        return checkAuth();
    }


    return true;
}



bool ClientSocket::checkAuth()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    const auto [is_auth, is_have_access, auth_error] =
    user_data.checkAuth(request_headers.map_required.value(Required_headers::Authorization), *sql_builder, request_headers.url_settings.url_index);

    if(sql_builder->isExec())
    {
        if(is_auth)
        {
            if(is_have_access)
            {
                if(auth_error.isEmpty())
                {
                    return true;
                }
                else
                {
                    request_headers.map_errors.insert(Request_Errors::Validation_Header_Error, auth_error);
                    return false;
                }
            }
            else
            {
                response.setStatusCode(Response::HTTP_Status_Code::Forbidden);
                return false;
            }
        }
        else
        {
            response.setStatusCode(Response::HTTP_Status_Code::Unauthorized);
            return false;
        }
    }
    else
    {
        response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);
        return false;
    }
}


void ClientSocket::sendResponse()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    handle_state = Handle_State::Ready;

    // Log errors
    if(response.statusCode() != Response::HTTP_Status_Code::OK)
    {
        writeLogs(response.getErrorResponse());
    }

    if(client_socket_state == Client_Socket_State::Closed)
    {
        onDisconnect();

        return;
    }

    bool is_continue_connection = true;


    response.addHeader(Response::Header::Keep_Alive, "timeout=60");



    if(response.statusCode() == Response::HTTP_Status_Code::Method_Not_Allowed)
    {
        const auto &url_method = request_headers.url_settings.url_method;

        QByteArray method_value;

        if(url_method == Method::Post)
        {
            method_value = "POST";
        }
        else if(url_method == Method::Get)
        {
            method_value = "GET";
        }

        response.addHeader(Response::Header::Allow, method_value);
    }
    else if(response.statusCode() == Response::HTTP_Status_Code::Unauthorized)
    {
        response.addHeader(Response::Header::WWW_Authenticate, "Bearer");
    }


    qDebug() << "Map Errors = " << request_headers.map_errors.value(Request_Errors::Validation_Header_Error);
    qDebug() << "Content Length = " << request_headers.map_required.value(Required_headers::Content_Length);

    // If, for example, with method OPTIONS we recieved body data or
    // if there method == POST and we have errors in headers then we close connection
    if( (method != Method::Post) || request_headers.map_errors.contains(Request_Errors::Validation_Header_Error))
    {
        is_continue_connection = (request_headers.map_required.value(Required_headers::Content_Length).toInt() == 0);

    }

    const bool is_need_to_stop_server = Global::isNeedToStopServer();


    if(!is_continue_connection || is_need_to_stop_server)
    {
       response.addHeader(Response::Header::Connection, "close");
    }


    response.sendHeaders(socket);

    response.sendData(socket);

    response.clear();


    socket->flush();


    if(!is_continue_connection || is_need_to_stop_server)
    {
        qDebug() << "Close connection" << __LINE__;

        socket->close();
        return;
    }

    reset();

    connection_duration.start(60000);

    if(socket->bytesAvailable() > 0)
    {
        onReadyRead();
    }
}



void ClientSocket::reset()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    is_check_headers = false;

    state = State::ReadStartLine;

    method = Method::Unknown;

    request_data.clear();

    sql_builder->reset();

    response.clear();

    request_headers.reset();
}


void ClientSocket::handleRequest()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    RequestBody request_body;


    ErrorResponse error_response = parseRequest(request_body);



    if(!error_response.isHaveError())
    {
        request_data.clear();

        AbstractRequestHandler *request_handler = getHandler();

        bool is_have_handler = (request_handler != nullptr);

        if(is_have_handler)
        {
            request_handler->setProperties(request_headers.url_settings.url_index, std::move(user_data), std::move(request_body), request_headers.url_settings.url_method, socket->peerAddress().toString().section(":", 3));

            request_handler->handle();

            return;
        }
        else
        {
            response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

            error_response.insertError(response.getStatusCodeValue(), "error_description", "invalid type handle, need fill switch of handlers");

            delete request_handler;
        }
    }


    if(error_response.isHaveError())
    {
        if(response.statusCode() == Response::HTTP_Status_Code::Unknown_Error)
        {
            response.setStatusCode(Response::HTTP_Status_Code::Bad_Request);
        }


        response.setData(std::move(error_response));


        sendResponse();
    }
}


ErrorResponse ClientSocket::parseRequest(RequestBody &request_body)
{
    if(method == Method::Post)
    {
        const auto content_type = available_media_types.value(request_headers.map_required.value(Required_headers::Content_Type));

        if(content_type == Supported_media_types::JSON)
        {
            return request_body.parseJson(request_data);
        }

        else if(content_type == Supported_media_types::Multipart_FormData)
        {
            return request_body.parseMultiData(request_data, request_headers.boundary);
        }

        else if(content_type == Supported_media_types::App_FormUrlEncoded)
        {
            return request_body.parseAppUrlEncoded(request_data);
        }
    }
    else if(method == Method::Get)
    {
        return request_body.parseGetParameters(request_headers.path);
    }

    return ErrorResponse();
}


AbstractRequestHandler *ClientSocket::getHandler()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";


    switch (request_headers.url_settings.url_type)
    {
    case Url_Type::User:

        return new User(*sql_builder, response, std::bind(&ClientSocket::sendResponse, this));

    case Url_Type::Order:

        return new Order(*sql_builder, response, std::bind(&ClientSocket::sendResponse, this));

    case Url_Type::Unknown:

        return nullptr;
    }

    return nullptr;
}


void ClientSocket::onDisconnect()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    client_socket_state = Client_Socket_State::Closed;

    if(handle_state == Handle_State::Ready)
    {
        if(socket->state() == QTcpSocket::SocketState::UnconnectedState)
        {
            socket->deleteLater();
            this->deleteLater();
        }
        else
        {
            socket->close();
        }
    }
}

