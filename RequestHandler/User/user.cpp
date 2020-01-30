#include "user.h"

User::User(SqlBuilder &_sql_builder_, Response &_response_, const Send_Response_Function _send_response_function_)
    : AbstractRequestHandler (_sql_builder_, _response_, _send_response_function_)
{

}

User::~User()
{

}


const QHash<unsigned char, std::function<void(User*)>> User::static_handle_request_map =
{
    {
        Url_Post::User_Login_NA,
        &User::POST_Login
    },
    {
        Url_Post::User_OAuth_Login_NA,
        &User::POST_OAuthLogin
    },
    {
        Url_Post::User_Check_Sign_Up_NA,
        &User::POST_CheckSignUp
    },
    {
        Url_Post::User_Sign_Up_Client_NA,
        &User::POST_SignUp
    },
    {
        Url_Post::User_Sign_Up_Driver_NA,
        &User::POST_SignUp
    },
    {
        Url_Post::User_Activation_Sign_Up_NA,
        &User::POST_ActivationSignUp
    },
    {
        Url_Post::User_Recovery_Pass_NA,
        &User::POST_RecoveryPass
    },
    {
        Url_Post::User_Send_New_Pass_NA,
        &User::POST_SendNewPass
    },
    {
        Url_Post::User_Resend_Sms_Code_NA,
        &User::POST_ResendSmsCode
    },
    {
        Url_Get::User_Client_Menu,
        &User::GET_Menu
    },
    {
        Url_Get::User_Driver_Menu,
        &User::GET_Menu
    },
    {
        Url_Get::User_Profile_Picture_NA,
        &User::GET_Profile_Picture
    },
    {
        Url_Post::User_Change_Profile_Picture,
        &User::POST_ChangeProfilePicture
    },
    {
        Url_Get::User_Profile,
        &User::GET_Profile
    },
    {
        Url_Post::User_Change_Profile,
        &User::POST_ChangeProfile
    },
    {
        Url_Post::User_Change_Password,
        &User::POST_ChangePassword
    },
    {
        Url_Get::User_Logout,
        &User::GET_Logout
    },
    {
        Url_Post::User_Contact_Us,
        &User::POST_Contact_Us
    },
    {
        Url_Post::User_Add_Push_Token,
        &User::POST_AddPushToken
    },
    {
        Url_Get::User_Get_Zip_Code_State_NA,
        &User::GET_ZipCodeState
    },
    {
        Url_Get::User_Get_Car_Model_List_NA,
        &User::GET_CarModelList
    },
    {
        Url_Post::User_Save_Payment_Method,
        &User::POST_SavePaymentMethod
    },
    {
        Url_Get::User_Get_Payment_Method,
        &User::GET_PaymentMethod
    },
    {
        Url_Get::User_Check_Auth_Token,
        &User::GET_CheckAuthToken
    }
};



void User::handle()
{
    AbstractRequestHandler::additionalHandle(static_handle_request_map, url_index, this);
}


void User::GET_Menu()
{
    QJsonObject menu;


    // Get email and address
    {
        QVector<QString> vec_key;

        if(url_index == Url_Get::User_Client_Menu)
        {
            vec_key = {"email", "client_home_address", "client_home_latitude", "client_home_longitude"};
        }
        else if(url_index == Url_Get::User_Driver_Menu)
        {
            vec_key = {"email"};
        }

        const auto select = sql_builder.getSelectString(vec_key, "user", "id_user");

        menu = json.getObjectFromQuery(select, user_data.user_Id());

        if(sql_builder.isExec() && (url_index == Url_Get::User_Client_Menu))
        {
            const QJsonObject home_object =
            {
                {"name", "home"},
                {"drop_off_address", menu.take("client_home_address").toString()},
                {"drop_off_longitude", menu.take("client_home_longitude").toDouble()},
                {"drop_off_latitude", menu.take("client_home_latitude").toDouble()}
            };

            menu.insert("home_object", home_object);
        }
    }


    // Get past orders
    if(sql_builder.isExec())
    {
        const QString user_str = (user_data.isDriver() ? "shipper" : "client");

        const QString select = "SELECT COUNT(id_order) "
                               "FROM `orders` "
                               "WHERE id_" + user_str + " = ? AND state = ?;";

        sql_builder.execQuery(select, {user_data.user_Id(), Order_State::Completed});

        if(sql_builder.isExec())
        {
            int past_orders{};

            if(sql_builder.isNext())
            {
                past_orders = sql_builder.getValue(0).toInt();
            }

            menu.insert("past_orders", past_orders);
        }
    }


    // Get last_drop_off_address_list
    if(sql_builder.isExec() && (url_index == Url_Get::User_Client_Menu))
    {
        const QString select = "SELECT drop_off_address, drop_off_longitude, drop_off_latitude "
                               "FROM `orders` "
                               "WHERE id_client = ? AND state = ? "
                               "ORDER BY id_order DESC "
                               "LIMIT 2;";

        auto last_drop_off_address_list = json.getObjectListFromQuery(select, {user_data.user_Id(), Order_State::Completed},
        [](QJsonObject &result)
        {
            const QString drop_off_address = result.value("drop_off_address").toString();

            result.insert("name", drop_off_address.section(',', 2).simplified());
            result.insert("drop_off_address", drop_off_address.section(',', 0, 2));
        });

        if(sql_builder.isExec())
        {
            last_drop_off_address_list.insert(0, menu.take("home_object"));

            menu.insert("last_drop_off_address_list", last_drop_off_address_list);
        }
    }


    // Get driver driver_total_earnings
    if(sql_builder.isExec() && (url_index == Url_Get::User_Driver_Menu))
    {
        const QString select = "SELECT SUM(order_price_without_tax) "
                               "FROM `orders` "
                               "WHERE id_shipper = ? AND state = ?;";

        sql_builder.execQuery(select, {user_data.user_Id(), Order_State::Completed});

        if(sql_builder.isExec())
        {
            double driver_total_earnings{};

            if(sql_builder.isNext())
            {
                driver_total_earnings = sql_builder.getValue(0).toDouble();
            }

            menu.insert("driver_total_earnings", driver_total_earnings);
        }
    }

    // If exists opened order, get it
    if(sql_builder.isExec())
    {
        const QString user_str = (user_data.isDriver() ? "shipper" : "client");

        const QString select = "SELECT id_order "
                               "FROM orders "
                               "WHERE id_" + user_str + " = ? "
                               "AND " + user_str + "_rated IS NULL "
                               "AND state != ?;";

        sql_builder.execQuery(select, {user_data.user_Id(), Order_State::Canceled});


        if(sql_builder.isExec())
        {
            if(sql_builder.isNext())
            {
                menu.insert("open_order_id", sql_builder.getValue(0).toInt());
            }
        }
    }


    if(sql_builder.isExec())
    {
        response_body = menu;
    }
}


void User::GET_Logout()
{
    // Delete auth_token

    const QString del = "DELETE FROM auth_token "
                        "WHERE id_user = ? AND auth_token = ?;";

    sql_builder.execQuery(del, {user_data.user_Id(), user_data.user_Token()});
}


void User::GET_Profile()
{
    QJsonObject profile;

    // Select profile
    {
        QVector<QString> vec_key =
        {
            "full_name",
            "email",
            "profile_picture_path",
            "phone_number",
            "state"
        };

        if(user_data.isDriver())
        {
            vec_key.append("zip_code");
        }
        else
        {
            vec_key.append("client_home_address AS address");
        }

        const QString select = sql_builder.getSelectString(vec_key, "user", "id_user");

        profile = json.getObjectFromQuery(select, user_data.user_Id());

        profile.insert("profile_picture_path", getUrlPicturePath(profile.value("profile_picture_path").toString()));

    }

    // Select Rate
    if(sql_builder.isExec())
    {
        const QString user_str = (user_data.isDriver() ? "shipper" : "client");

        const QString user_rated_str = (user_data.isDriver() ? "client" : "shipper");

        const QVariant avg_rate =
        sql_builder.selectFromDB("ROUND(AVG(" + user_rated_str + "_rated), 1)", "orders", {"id_" + user_str, "state"}, {user_data.user_Id(), Order_State::Completed}, is_valid);

        if(sql_builder.isExec())
        {
            profile.insert("avg_rate", avg_rate.toDouble());
        }
    }


    // Get last4 numbers and card type
    if(sql_builder.isExec())
    {
        const QVariantMap map_select =
        sql_builder.selectFromDB<QVariantMap>({"last_four_numbers", "card_type"}, "payment_method", "id_user", user_data.user_Id(), is_valid);

        if(sql_builder.isExec())
        {
            profile.insert("last_four_numbers", map_select.value("last_four_numbers").toString());
            profile.insert("card_type", map_select.value("card_type").toString());
        }
    }

    if(sql_builder.isExec())
    {
        response_body = profile;
    }
}


// http://159.203.113.210:27099/profile_picture/54654.png
// http://159.203.113.210:27099/profile_picture/../.png

void User::GET_Profile_Picture()
{
    Response::Supported_Image_Ext image_ext = Response::Supported_Image_Ext::Unknow;

    // Find extension
    {
        const int ext_index = request_body.request_url.lastIndexOf(QLatin1Char('.'));

        qDebug() << request_body.request_url;

        is_valid = (ext_index != -1);

        if(is_valid)
        {
            const QString extension = request_body.request_url.mid(ext_index + 1);

            image_ext = Response::static_map_available_image_ext.value(extension, Response::Supported_Image_Ext::Unknow);

            qDebug() << "Extension = " << extension << "Path = " << request_body.request_url;
        }
        else
        {
            response.setStatusCode(Response::HTTP_Status_Code::Not_Found);
        }
    }

    // If extension is supported
    if(is_valid)
    {
        is_valid = (image_ext != Response::Supported_Image_Ext::Unknow);

        if(!is_valid)
        {
            error_response.insertError("not supported image extension");
        }
    }

    if(is_valid)
    {
        QString file_path;

        {
            const int file_name_index = request_body.request_url.lastIndexOf("/");

            if(file_name_index != -1)
            {
                file_path = request_body.request_url.mid(file_name_index).replace(QLatin1String("../"), QLatin1String(""));
            }
        }

        QFile file_profile_picture(Global::getPictureDirPath() + file_path);

        is_valid = file_profile_picture.exists();

        if(is_valid)
        {
            if(file_profile_picture.open(QIODevice::ReadOnly))
            {
                response.setData(file_profile_picture.readAll(), image_ext, Response::Content_Disposition::Inline);

                file_profile_picture.close();
            }
            else
            {
                const QJsonObject log_error =
                {
                    {"description", "Can't open file"},
                    {"file_name", file_profile_picture.fileName()},
                    {"file_error", file_profile_picture.errorString()}
                };

                error_response.insertLogError(log_error);

                response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);
            }
        }
        else
        {
            response.setStatusCode(Response::HTTP_Status_Code::Not_Found);
        }
    }
}


void User::POST_Login()
{
    QVariantMap map_result;

    // Check received json
    {
        Map_required_json_check map_key_required =
        {
            {
                "email", Json::Validation_Type::Email
            },
            {
                "password", Json::Validation_Type::Password
            },
            {
                "is_driver", Json::Validation_Type::Bool
            }
        };

        is_valid = json.validateJson(map_key_required, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }

    if(is_valid)
    {
        const bool is_driver = map_result.value("is_driver").toBool();

        // Check does exist user with specified email and get salt, hash_password

        const QVariantMap map_select =
        sql_builder.selectFromDB<QVariantMap>({"id_user", "salt", "hash_password"}, "user", {"email", "is_driver"}, {map_result.value("email"), is_driver}, is_valid);


        if(sql_builder.isExec())
        {
            // Check does user exist
            if(!is_valid)
            {
                bool is_in_activation_stage = false;

                sql_builder.selectFromDB("1", "not_registered", {"email", "is_driver"}, {map_result.value("email"), is_driver}, is_in_activation_stage);

                if(sql_builder.isExec())
                {
                    if(is_in_activation_stage)
                    {
                        response_body.insert("need_activate", true);
                    }
                    else
                    {
                        error_response.insertError("user not found", "email", map_result.value("email"));
                    }
                }
            }


            // Check received password
            if(is_valid)
            {
                is_valid = checkHashPassword(map_result.value("password").toByteArray(), map_select.value("salt").toByteArray(), map_select.value("hash_password").toByteArray());

                if(!is_valid)
                {
                    error_response.insertError("invalid password");
                }
            }


            // Return auth_token
            if(is_valid)
            {
                addAuthToken(response_body, map_select.value("id_user").toInt(), is_driver);
            }
        }
    }
}


void User::POST_OAuthLogin()
{
    enum Platform : int
    {
        Google = 0,
        Facebook = 1
    };


    QVariantMap map_result;

    // Check received json
    {
        Map_required_json_check map_required =
        {
            {"access_token", Json::Validation_Type::Common_String_Max_500},
            {"platform", Json::Validation_Type::U_Int},
            {"is_driver", Json::Validation_Type::Bool}
        };

        is_valid = json.validateJson(map_required, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }


    // Check platform
    if(is_valid)
    {
        const int platform = map_result.value("platform").toInt();

        is_valid = (platform == Platform::Google) ||
                   (platform == Platform::Facebook);

        if(!is_valid)
        {
            error_response.insertError("invalid platform", "platform", platform);
        }
    }


    if(is_valid)
    {
        const auto access_token = map_result.value("access_token");

        const int platform = map_result.value("platform").toInt();

        const bool is_driver = map_result.value("is_driver").toBool();


        // Check does google_access_token exist in database
        {
            QString where_value;

            if(platform == Platform::Google)
            {
                where_value = "google_access_token";
            }
            else if(platform == Platform::Facebook)
            {
                where_value = "facebook_access_token";
            }

            const QVariant select_key =
            sql_builder.selectFromDB("id_user", "user", {where_value, "is_driver"}, {access_token, is_driver}, is_valid);

            if(sql_builder.isExec())
            {
                if(is_valid)
                {
                    addAuthToken(response_body, select_key.toInt(), is_driver);
                }
            }
        }


        if(sql_builder.isExec())
        {
            if(!is_valid)
            {
                is_valid = true; // Because json.getObjectFromQuery() use is_valid

                is_foreign_api_using = true;

                auto response_func = [this, platform, is_driver](QJsonObject &&response_body, ErrorResponse &&api_error_response)
                {
                    error_response = std::move(api_error_response);

                    if(!error_response.isHaveLogError())
                    {
                        QString where_value, response_user_id_name;

                        if(platform == Platform::Google)
                        {
                            where_value = "google_user_id";
                            response_user_id_name = "sub";
                        }
                        else if(platform == Platform::Facebook)
                        {
                            where_value = "facebook_user_id";
                            response_user_id_name = "id";
                        }

                        {
                            const QString select = "SELECT id_user "
                                                   "FROM user "
                                                   "WHERE (email = ? OR " + where_value + " = ?) AND is_driver = ?;";

                            sql_builder.execQuery(select, {response_body.value("email").toString(), response_body.value(response_user_id_name).toString(), is_driver});
                        }


                        if(sql_builder.isExec())
                        {
                            is_valid = sql_builder.isNext();

                            if(is_valid)
                            {
                                addAuthToken(this->response_body, sql_builder.getValue(0).toInt(), is_driver);
                            }
                            else
                            {
                                this->response_body.insert("need_register", true);
                            }
                        }
                    }
                    else
                    {
                        error_response.insertError("invalid access_token");
                    }

                    finishHandle();
                };

                if(platform == Platform::Google)
                {
                    ForeignApiHandler::getGoogleOAuth()->getUserInfo(access_token.toString(), std::move(response_func));
                }
                else if(platform == Platform::Facebook)
                {
                    ForeignApiHandler::getFacebookOAuth()->getUserInfo(access_token.toString(), std::move(response_func));
                }
            }
        }
    }
}



void User::POST_CheckSignUp()
{
    QVariantMap map_result;

    // Check received json
    {
        QMap<QString, Json::Validation_Logic_Structure> map_key_required =
        {
            {
                "full_name", {Json::Validation_Type::Common_String_Max_255}
            },
            {
                "password", {Json::Validation_Type::Password}
            },
            {
                "email", {Json::Validation_Type::Email}
            }
        };

        is_valid = json.validateJson(map_key_required, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }


    // Check full_name
    if(is_valid)
    {
        map_result.insert("full_name", map_result.value("full_name").toString().trimmed());

        const Validator::Result result = Validator::checkFullName("full_name", map_result.value("full_name").toString());

        is_valid = result.is_valid;

        if(!is_valid)
        {
            error_response.insertError(result.error);
        }
    }


    // Check that email doesn't exist among activated accounts
    if(is_valid)
    {
        checkEmailExist(map_result.value("email").toString());
    }

    // Check that email doesn't exist among non-activated accounts
    if(is_all_valid())
    {
        bool is_exist = false;

        sql_builder.selectFromDB("1", "not_registered", "email", map_result.value("email").toString(), is_exist);

        if(sql_builder.isExec())
        {
            is_valid = !is_exist;

            if(!is_valid)
            {
                error_response.insertError("this email already in activation stage", "email", map_result.value("email").toString());
            }
        }
    }
}


void User::POST_SignUp()
{
    QVariantMap map_result;

    // Check received json
    {
        QMap<QString, Json::Validation_Logic_Structure> map_key_required =
        {
            {
                "full_name", {Json::Validation_Type::Common_String_Max_255}
            },
            {
                "password", {Json::Validation_Type::Password}
            },
            {
                "email", {Json::Validation_Type::Email}
            },
            {
                "google_access_token", {Json::Validation_Type::Common_String_Max_500, Json::Is_Required_Validation::Not_Required}
            },
            {
                "facebook_access_token", {Json::Validation_Type::Common_String_Max_500, Json::Is_Required_Validation::Not_Required}
            },
            {
                "state", {Json::Validation_Type::Common_String_Max_255}
            },
            {
                "zip_code", {Json::Validation_Type::Common_String_Max_45}
            },
            {
                "phone_number", {Json::Validation_Type::Common_String_Max_45}
            }
        };

        if(url_index == Url_Post::User_Sign_Up_Driver_NA)
        {
            map_key_required.insert("driver_license_number", {Json::Validation_Type::Common_String_Max_255});
            map_key_required.insert("car_model", {Json::Validation_Type::Common_String_Max_255});
            map_key_required.insert("plate_number", {Json::Validation_Type::Common_String_Max_255});
        }

        is_valid = json.validateJson(map_key_required, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }

    // Check state and zip_code
    if(is_valid)
    {
        checkZipCodeAndState(map_result.value("zip_code").toString(), map_result.value("state").toString());
    }

    // Check full_name
    if(is_valid)
    {
        map_result.insert("full_name", map_result.value("full_name").toString().trimmed());

        const Validator::Result result = Validator::checkFullName("full_name", map_result.value("full_name").toString());

        is_valid = result.is_valid;

        if(!is_valid)
        {
            error_response.insertError(result.error);
        }
    }



    QString email = (is_valid ? map_result.value("email").toString() : "");

    // Check that email and phone number don't exist among activated accounts and non-activated accounts
    if(is_valid)
    {
        const QString phone_number = map_result.value("phone_number").toString();

        enum class Table : unsigned char
        {
            User,
            Not_Registered
        };

        auto check_email_and_phone = [this, email, phone_number](const Table table)
        {
            {
                QString table_name;

                if(table == Table::User)
                {
                    table_name = "user";
                }
                else if(table == Table::Not_Registered)
                {
                    table_name = "not_registered";
                }

                QString select = "SELECT email, phone_number "
                                 "FROM " + table_name + " "
                                 "WHERE email = ? OR phone_number = ?;";

                sql_builder.execQuery(select, {email, phone_number});
            }

            if(sql_builder.isExec())
            {
                is_valid = !sql_builder.isNext();

                if(!is_valid)
                {
                    if(sql_builder.getValue("email").toString() == email)
                    {
                        if(table == Table::User)
                        {
                            error_response.insertError("such email already exists", "email", email);
                        }
                        else if(table == Table::Not_Registered)
                        {
                            error_response.insertError("this email already in activation stage", "email", email);
                        }
                    }
                    else if(sql_builder.getValue("phone_number").toString() == phone_number)
                    {
                        if(table == Table::User)
                        {
                            error_response.insertError("such phone number already exists", "phone_number", phone_number);
                        }
                        else if(table == Table::Not_Registered)
                        {
                            error_response.insertError("this phone number already in activation stage", "phone_number", phone_number);
                        }
                    }
                }
            }
        };

        check_email_and_phone(Table::User);

        if(is_all_valid())
        {
            check_email_and_phone(Table::Not_Registered);
        }
    }



    auto goAhead = [this](QVariantMap &&map_result)
    {
        if(is_all_valid())
        {
            auto insert_values_in_db = [this](QVariantMap &map_result)
            {
                QByteArray salt;

                const QByteArray hash_password = getHashPassword(map_result.value("password").toByteArray(), salt);

                map_result.remove("password");

                map_result.insert("salt", salt);
                map_result.insert("hash_password", hash_password);
                map_result.insert("expires_in", QDateTime::currentDateTime().addSecs(activationSignUp_time_s));


                if(url_index == Url_Post::User_Sign_Up_Client_NA)
                {
                    map_result.insert("is_driver", false);
                }
                else if(url_index == Url_Post::User_Sign_Up_Driver_NA)
                {
                    map_result.insert("is_driver", true);
                }


                sql_builder.insertIntoDB(map_result, "not_registered");
            };


            auto send_sms_and_insert_values = [this, insert_values_in_db = std::move(insert_values_in_db)](QVariantMap &&map_result)
            {
                QString sms_code = generateUniqueSmsCode();

                if(sql_builder.isExec())
                {
                    QString to_phone_number = map_result.value("phone_number").toString();

                    map_result.insert("sms_code", sms_code);

                    is_foreign_api_using = true;

                    auto response_func = [this, map_result = std::move(map_result), insert_values_in_db = std::move(insert_values_in_db)]
                    (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
                    {
                        error_response = std::move(api_error_response);

                        if(!error_response.isHaveLogError())
                        {
                            insert_values_in_db(map_result);
                        }
                        else
                        {
                            error_response.insertError("No test number found. Use test number to create an account.");
                        }

                        finishHandle();
                    };


                    ForeignApiHandler::getTwilio()->sendSMS(std::move(sms_code), std::move(to_phone_number), std::move(response_func));
                }
            };


            enum class Platform : unsigned char
            {
                Unknow,
                Google,
                Facebook
            } platform = Platform::Unknow;

            auto checkGoogleAndFacebookTokensAndSendSms = [this]
            (const Platform platform, const QVariant &oauth_token, QVariantMap &&map_result, decltype (send_sms_and_insert_values) &&send_sms_and_insert_values)
            {
                QString access_token_name;

                if(platform == Platform::Google)
                {
                    access_token_name = "google_access_token";
                }
                else if(platform == Platform::Facebook)
                {
                    access_token_name = "facebook_access_token";
                }

                bool is_exist = false;

                sql_builder.selectFromDB("1", "user", access_token_name, oauth_token, is_exist);

                if(sql_builder.isExec() && is_exist)
                {
                    error_response.insertError("such user already exists", access_token_name, oauth_token);
                }
                else if(sql_builder.isExec() && !is_exist)
                {
                    is_foreign_api_using = true;

                    auto response_func = [this, platform, map_result = std::move(map_result), send_sms_and_insert_values = std::move(send_sms_and_insert_values)]
                    (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
                    {
                        error_response = std::move(api_error_response);

                        bool finish_handle = false;

                        if(!error_response.isHaveLogError())
                        {
                            QString user_id_name, response_user_id_name;

                            if(platform == Platform::Google)
                            {
                                user_id_name = "google_user_id";
                                response_user_id_name = "sub";
                            }
                            else if(platform == Platform::Facebook)
                            {
                                user_id_name = "facebook_user_id";
                                response_user_id_name = "id";
                            }

                            const auto oauth_user_id = response_body.value(response_user_id_name).toString();

                            bool is_exist = false;

                            sql_builder.selectFromDB("1", "user", user_id_name, oauth_user_id, is_exist);

                            if(sql_builder.isExec())
                            {
                                if(is_exist)
                                {
                                    finish_handle = true;

                                    error_response.insertError("such user already exists");
                                }
                                else
                                {
                                    map_result.insert(user_id_name, oauth_user_id);

                                    send_sms_and_insert_values(std::move(map_result));
                                }
                            }
                            else
                            {
                                finish_handle = true;
                            }
                        }
                        else
                        {
                            finish_handle = true;

                            error_response.insertError("invalid access_token");
                        }

                        if(finish_handle)
                        {
                            finishHandle();
                        }
                    };


                    if(platform == Platform::Google)
                    {
                        ForeignApiHandler::getGoogleOAuth()->getUserInfo(oauth_token.toString(), std::move(response_func));
                    }
                    else if(platform == Platform::Facebook)
                    {
                        ForeignApiHandler::getFacebookOAuth()->getUserInfo(oauth_token.toString(), std::move(response_func));
                    }
                }
            };


            // If we receive google_access_token or facebook_access_token, then check it.
            if(is_all_valid())
            {
                auto it_token_value = map_result.constFind("google_access_token");

                if(it_token_value != map_result.constEnd())
                {
                    platform = Platform::Google;
                }
                else
                {
                    it_token_value = map_result.constFind("facebook_access_token");

                    if(it_token_value != map_result.constEnd())
                    {
                        platform = Platform::Facebook;
                    }
                }

                if(platform == Platform::Unknow)
                {
                    send_sms_and_insert_values(std::move(map_result));
                }
                else
                {
                    checkGoogleAndFacebookTokensAndSendSms(platform, it_token_value.value(), std::move(map_result), std::move(send_sms_and_insert_values));
                }

            }
        }
    };


    if(is_all_valid())
    {
        if(url_index == Url_Post::User_Sign_Up_Driver_NA)
        {
            goAhead(std::move(map_result));
        }
        else if(url_index == Url_Post::User_Sign_Up_Client_NA)
        {
            QString address = map_result.value("state").toString() + " " + map_result.value("zip_code").toString();

            auto function = [map_result = std::move(map_result), goAhead = std::move(goAhead)](Address &&address_struct) mutable
            {
                map_result.insert("client_home_address", address_struct.address);
                map_result.insert("client_home_latitude", address_struct.latitude);
                map_result.insert("client_home_longitude", address_struct.longitude);

                goAhead(std::move(map_result));

                return Is_Finish_Handle::No;
            };

            getLatLongFromAddress(std::move(address), std::move(function));
        }
    }

}


void User::POST_ActivationSignUp()
{
    QVariant sms_code;

    // Check received json
    is_valid = json.validateJson({"sms_code", Json::Validation_Type::Common_String_Max_45}, sms_code, request_body.received_json, Json::Is_Required_Validation::Required);


    if(is_valid)
    {
        QVariantMap map_received;

        // Check if this hash exists
        {
            const QVector<QString> vec_key =
            {
                "id_not_registered",
                "full_name",
                "email",
                "salt",
                "hash_password",
                "expires_in",
                "google_access_token",
                "google_user_id",
                "facebook_access_token",
                "facebook_user_id",
                "state",
                "zip_code",
                "phone_number",
                "client_home_address",
                "client_home_latitude",
                "client_home_longitude",
                "is_driver",
                "driver_license_number",
                "car_model",
                "plate_number"
            };

            map_received = sql_builder.selectFromDB<QVariantMap>(vec_key, "not_registered", "sms_code", sms_code, is_valid);


            if(sql_builder.isExec())
            {
                if(!is_valid)
                {
                    error_response.insertError("sms_code doesn't found", "sms_code", sms_code);
                }
            }
        }


        // Check if this hash expired
        if(is_all_valid())
        {
            is_valid = (map_received.value("expires_in").toDateTime() > QDateTime::currentDateTime());

            if(!is_valid)
            {
                const QJsonObject error_description =
                {
                    {
                        "sms_code", sms_code.toString()
                    },
                    {
                        "expires_in", map_received.value("expires_in").toDateTime().toString("yyyy-MM-dd HH:mm:ss")
                    }
                };

                error_response.insertError("sms_code expired", error_description);
            }
        }



        // Insert into database
        if(is_all_valid())
        {
            auto insert_values_in_db = [this](QVariantMap &map_received)
            {
                QSqlDatabase db = sql_builder.beginTransaction();

                const int id_not_registered = map_received.value("id_not_registered").toInt();

                const bool is_driver = map_received.value("is_driver").toBool();

                // Insert user
                if(sql_builder.isExec())
                {
                    // Check google_token and facebook_token
                    {
                        auto it = map_received.find("google_access_token");

                        if(it != map_received.end())
                        {
                            if(it.value().toString().isEmpty())
                            {
                                map_received.remove("google_access_token");
                                map_received.remove("google_user_id");
                            }
                        }

                        it = map_received.find("facebook_access_token");

                        if(it != map_received.end())
                        {
                            if(it.value().toString().isEmpty())
                            {
                                map_received.remove("facebook_access_token");
                                map_received.remove("facebook_user_id");
                            }
                        }
                    }


                    if(is_driver)
                    {
                        map_received.insert("ready_to_ship", true);
                        map_received.insert("shipper_order_state", Shipper_Order_State::Does_Not_Has_Order);

                        map_received.remove("client_home_address");
                        map_received.remove("client_home_latitude");
                        map_received.remove("client_home_longitude");
                    }
                    else
                    {
                        map_received.remove("driver_license_number");
                        map_received.remove("car_model");
                        map_received.remove("plate_number");
                    }

                    map_received.remove("expires_in");

                    map_received.remove("id_not_registered");


                    map_received.insert("profile_picture_path", "placeholder_profile.jpg");

                    map_received.insert("registration_date", QDateTime::currentDateTime());


                    sql_builder.insertIntoDB(map_received, "user");
                }

                const int id_user = (sql_builder.isExec() ? sql_builder.lastInsertId().toInt() : 0);


                // Remove from not_registered
                if(sql_builder.isExec())
                {
                    sql_builder.deleteFrom("not_registered", "id_not_registered", id_not_registered);
                }


                // Return auth_token
                if(sql_builder.isExec())
                {
                    addAuthToken(response_body, id_user, is_driver);
                }

                sql_builder.endTransaction(db);
            };

            const bool is_driver = map_received.value("is_driver").toBool();

            is_foreign_api_using = true;

            QString name = map_received.value("full_name").toString();
            QString email = map_received.value("email").toString();

            auto response_func = [this, map_received = std::move(map_received), insert_values_in_db = std::move(insert_values_in_db)]
            (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
            {
                error_response = std::move(api_error_response);

                if(!error_response.isHaveLogError())
                {
                    const auto customer_account_id = response_body.value("id").toString();

                    map_received.insert("customer_account_id", customer_account_id);

                    insert_values_in_db(map_received);
                }
                else
                {
                    response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);
                }

                finishHandle();
            };

            if(is_driver)
            {
                StripeApi::Account account =
                {
                    email,
                    name.split(" ").first(),
                    name.split(" ").last(),
                    client_ip_address
                };

                ForeignApiHandler::getStripeApi()->createAccount(std::move(account), std::move(response_func));
            }
            else
            {
                ForeignApiHandler::getStripeApi()->createCustomer(std::move(email), std::move(name), std::move(response_func));
            }
        }
    }
}


void User::POST_RecoveryPass()
{
    QVariant email;

    // Check received json
    {
        is_valid = json.validateJson({"email", Json::Validation_Type::Email}, email, request_body.received_json, Json::Is_Required_Validation::Required);
    }

    QVariantMap map_select;


    // Check does specified email exist
    if(is_valid)
    {
        map_select = sql_builder.selectFromDB<QVariantMap>({"id_user", "phone_number"}, "user", "email", email, is_valid);

        if(sql_builder.isExec())
        {
            if(!is_valid)
            {
                error_response.insertError("email doesn't found", "email", email);
            }
        }
    }

    const int id_user = (is_all_valid() ? map_select.take("id_user").toInt() : 0);

    if(is_all_valid())
    {
        QVariantMap map_col_value;

        bool were_earlier_attempts_to_recover_pass = false;

        //
        {
            const QVariantMap map_received =
            sql_builder.selectFromDB<QVariantMap>({"attempts", "block_time"}, "restore_pass", "id_user", id_user, were_earlier_attempts_to_recover_pass);
        }

        QString sms_code;

        if(is_all_valid())
        {
            sms_code = generateUniqueSmsCode();
        }

        // Insert data into database
        if(is_all_valid())
        {
            QString to_phone_number = map_select.value("phone_number").toString();

            map_col_value.insert("phone_number", map_select.value("phone_number"));
            map_col_value.insert("sms_code", sms_code);
            map_col_value.insert("expires_in", QDateTime::currentDateTime().addSecs(recoveryPass_hash_time_s));
            map_col_value.insert("attempts", 0);

            is_foreign_api_using = true;

            auto response_func = [this, map_col_value = std::move(map_col_value), id_user, were_earlier_attempts_to_recover_pass]
            (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
            {
                error_response = std::move(api_error_response);

                if(!error_response.isHaveLogError())
                {
                    // INSERT
                    if(!were_earlier_attempts_to_recover_pass)
                    {
                        map_col_value.insert("id_user", id_user);

                        sql_builder.insertIntoDB(map_col_value, "restore_pass");
                    }
                    else
                    // UPDATE
                    {
                        sql_builder.updateDB(map_col_value, "restore_pass", {"id_user", id_user});
                    }
                }
                else
                {
                    error_response.insertError("server could not send sms");
                }

                finishHandle();
            };


            ForeignApiHandler::getTwilio()->sendSMS(std::move(sms_code), std::move(to_phone_number), std::move(response_func));
        }
    }
}


void User::POST_SendNewPass()
{
    QVariantMap map_result;

    // Check received json
    {
        Map_required_json_check map_key_required =
        {
            {
                "sms_code", Json::Validation_Type::Common_String_Max_45
            },
            {
                "new_password", Json::Validation_Type::Password
            },
            {
                "confirm_password", Json::Validation_Type::Password
            }
        };

        is_valid = json.validateJson(map_key_required, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }

    // Check does new_password equal confirm_password
    if(is_valid)
    {
        is_valid = (map_result.value("new_password").toString() == map_result.value("confirm_password").toString());

        if(!is_valid)
        {
            const QJsonObject error_description =
            {
                {
                    "new_password", map_result.value("new_password").toString()
                },
                {
                    "confirm_password", map_result.value("confirm_password").toString()
                }
            };

            error_response.insertError("new_password and confirm_password do not match", "error_description", error_description);
        }
    }

    if(is_valid)
    {
        const QVariantMap map_select = sql_builder.selectFromDB<QVariantMap>({"id_user", "expires_in"}, "restore_pass", "sms_code", map_result.value("sms_code"), is_valid);

        if(sql_builder.isExec())
        {
            // Does specified sms_code exist
            if(!is_valid)
            {
                error_response.insertError("sms_code doesn't found", "sms_code", map_result.value("sms_code"));
            }


            // Check does sms_code not expired
            if(is_valid)
            {
                is_valid = (map_select.value("expires_in").toDateTime() > QDateTime::currentDateTime());

                if(!is_valid)
                {
                    const QJsonObject error_description =
                    {
                        {
                            "sms_code", map_result.value("sms_code").toString()
                        },
                        {
                            "expires_in", map_result.value("expires_in").toDateTime().toString("yyyy-MM-dd HH:mm:ss")
                        }
                    };

                    error_response.insertError("expired sms_code", error_description);
                }
            }



            if(is_valid)
            {
                QSqlDatabase db = sql_builder.beginTransaction();

                // Delete sms_code
                if(sql_builder.isExec())
                {
                    sql_builder.deleteFrom("restore_pass", "id_user", map_select.value("id_user").toInt());
                }


                // Insert new password
                if(sql_builder.isExec())
                {
                    QByteArray salt;

                    const auto hash_password = getHashPassword(map_result.value("new_password").toByteArray(), salt);

                    const QVariantMap map_col_value =
                    {
                        {
                            "salt", salt
                        },
                        {
                            "hash_password", hash_password
                        }
                    };

                    sql_builder.updateDB(map_col_value, "user", {"id_user", map_select.value("id_user")});
                }

                sql_builder.endTransaction(db);
            }
        }
    }

}


void User::POST_ResendSmsCode()
{
    enum Type_Of_Sms : int
    {
        Activation,
        Recovery_Pass
    };


    QVariantMap map_result;

    // Check received json
    {
        Map_required_json_check map_required =
        {
            {"phone_number", Json::Validation_Type::Common_String_Max_45},
            {"type_of_sms", Json::Validation_Type::U_Int}
        };

        is_valid = json.validateJson(map_required, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }


    // Check type of sms
    if(is_valid)
    {
        const int type_of_sms = map_result.value("type_of_sms").toInt();

        is_valid = (type_of_sms == Type_Of_Sms::Activation) ||
                   (type_of_sms == Type_Of_Sms::Recovery_Pass);

        if(!is_valid)
        {
            error_response.insertError("invalid type_of_sms", "type_of_sms", type_of_sms);
        }
    }


    if(is_valid)
    {
        QString phone_number = map_result.value("phone_number").toString();

        const int type_of_sms = map_result.value("type_of_sms").toInt();

        QString id_name, table_name;

        if(type_of_sms == Type_Of_Sms::Activation)
        {
            id_name = "id_not_registered";
            table_name = "not_registered";
        }
        else if(type_of_sms == Type_Of_Sms::Recovery_Pass)
        {
            id_name = "id_restore_pass";
            table_name = "restore_pass";
        }

        const QVariant id_record =
        sql_builder.selectFromDB(id_name, table_name, "phone_number", phone_number, is_valid);


        if(sql_builder.isExec())
        {
            QString sms_code;

            if(is_valid)
            {
                sms_code = generateUniqueSmsCode();
            }
            else
            {
                if(type_of_sms == Type_Of_Sms::Activation)
                {
                    error_response.insertError("you not in activation stage", "phone_number", phone_number);
                }
                else if(type_of_sms == Type_Of_Sms::Recovery_Pass)
                {
                    error_response.insertError("you not in recover password", "phone_number", phone_number);
                }
            }

            if(is_all_valid())
            {
                QVariantMap map_col_value =
                {
                    {"sms_code", sms_code}
                };

                if(type_of_sms == Type_Of_Sms::Activation)
                {
                    map_col_value.insert("expires_in", QDateTime::currentDateTime().addSecs(activationSignUp_time_s));
                }
                else if(type_of_sms == Type_Of_Sms::Recovery_Pass)
                {
                    map_col_value.insert("expires_in", QDateTime::currentDateTime().addSecs(recoveryPass_hash_time_s));
                }

                is_foreign_api_using = true;

                auto response_func = [this, map_col_value = std::move(map_col_value), table_name = std::move(table_name), id_name = std::move(id_name), id_record = std::move(id_record)]
                (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
                {
                    error_response = std::move(api_error_response);

                    if(!error_response.isHaveLogError())
                    {
                        sql_builder.updateDB(map_col_value, table_name, {id_name, id_record});
                    }
                    else
                    {
                        error_response.insertError("server could not send sms");
                    }

                    finishHandle();
                };


                ForeignApiHandler::getTwilio()->sendSMS(std::move(sms_code), std::move(phone_number), std::move(response_func));
            }
        }
    }
}



void User::POST_ChangeProfilePicture()
{
    auto it = request_body.file_map.find("profile_picture");

    is_valid = (it != request_body.file_map.end());


    if(is_valid)
    {
        RequestBody::MultiData &multi_data = it.value();

        QString extension = ".";

        // Check file type
        {
            const QString file_type_name = multi_data.type.name();

            const QMap<QString, QString> map_extension =
            {
                {"image/jpeg", "jpg"},
                {"image/png", "png"},
                {"image/svg+xml", "svg"}
            };

            auto it = map_extension.constFind(file_type_name);

            is_valid = (it != map_extension.constEnd());

            if(is_valid)
            {
                extension.append(it.value());
            }
            else
            {
                error_response.insertError("invalid file type", "file_type", file_type_name);
            }
        }


        if(is_valid)
        {
            bool is_picture_exist = false;

            const QVariant profile_picture = sql_builder.selectFromDB("profile_picture_path", "user", "id_user", user_data.user_Id(), is_picture_exist);

            if(sql_builder.isExec())
            {
                const QString new_file_path = getHashToken() + extension;

                QSqlDatabase db = sql_builder.beginTransaction();

                if(sql_builder.isExec())
                {
                    sql_builder.updateDB({"profile_picture_path", new_file_path}, "user", {"id_user", user_data.user_Id()});
                }

                bool is_created_file = false;

                if(sql_builder.isExec())
                {
                    QFile file_profile_picture(Global::getPictureDirPath() + new_file_path);

                    is_created_file = file_profile_picture.open(QIODevice::WriteOnly);

                    if(is_created_file)
                    {
                        file_profile_picture.write(multi_data.data);

                        file_profile_picture.flush();
                        file_profile_picture.close();
                    }
                    else
                    {
                        const QJsonObject log_error =
                        {
                            {"description", "Can't create file"},
                            {"file_name", new_file_path},
                            {"file_error", file_profile_picture.errorString()}
                        };

                        error_response.insertLogError(log_error);

                        response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);
                    }
                }

                if(sql_builder.isExec() && is_created_file && is_picture_exist)
                {
                    if(profile_picture.toString() != QLatin1String("placeholder_profile.jpg"))
                    {
                        QFile(Global::getPictureDirPath() + profile_picture.toString()).remove();
                    }
                }

                if(is_created_file)
                {
                    sql_builder.endTransaction(db);
                }
                else
                {
                    sql_builder.rollbackTransaction(db);
                }
            }
        }
    }
    else
    {
        error_response.insertError("profile_picture not found");
    }
}



void User::POST_ChangeProfile()
{
    QVariantMap map_result;


    // Check received json
    {
        Map_required_json_check map_key =
        {
            {
                "full_name", Json::Validation_Type::Common_String_Max_255
            },
            {
                "email", Json::Validation_Type::Email
            },
            {
                "phone_number", Json::Validation_Type::Common_String_Max_45
            },
            {
                "zip_code", Json::Validation_Type::Common_String_Max_45
            },
            {
                "state", Json::Validation_Type::Common_String_Max_255
            }
        };

        is_valid = json.validateJson(map_key, map_result, request_body.received_json, Json::Is_Required_Validation::Not_Required);
    }


    // Check full_name
    if(is_valid)
    {
        auto it = map_result.find("full_name");

        if(it != map_result.end())
        {
            it.value() = it.value().toString().trimmed();

            const Validator::Result result = Validator::checkFullName("full_name", it.value().toString());

            is_valid = result.is_valid;

            if(!is_valid)
            {
                error_response.insertError(result.error);
            }
        }
    }


    QString zip_code, state;

    // Check zip_code, state
    if(is_valid)
    {
        {
            auto it = map_result.constFind("zip_code");

            if(it != map_result.constEnd())
            {
                zip_code = it.value().toString();
            }

            it = map_result.constFind("state");

            if(it != map_result.constEnd())
            {
                state = it.value().toString();
            }
        }


        if(!zip_code.isEmpty() || !state.isEmpty())
        {
            is_valid = (!zip_code.isEmpty() && !state.isEmpty());

            if(is_valid)
            {
                checkZipCodeAndState(zip_code, state);
            }
            else
            {
                error_response.insertError("you can't change the zip_code or state separately");
            }
        }
    }


    // Check that email and phone number don't exist among activated accounts and non-activated accounts
    if(is_valid)
    {
        const QString email = map_result.value("email").toString();

        const QString phone_number = map_result.value("phone_number").toString();

        if(!email.isEmpty() || !phone_number.isEmpty())
        {
            enum class Table : unsigned char
            {
                User,
                Not_Registered
            };

            auto check_email_and_phone = [this, email, phone_number](const Table table)
            {
                {
                    QString table_name;

                    if(table == Table::User)
                    {
                        table_name = "user";
                    }
                    else if(table == Table::Not_Registered)
                    {
                        table_name = "not_registered";
                    }

                    QString select = "SELECT email, phone_number "
                                     "FROM " + table_name + " "
                                     "WHERE ";

                    QVector<QVariant> vec_value;

                    if(!email.isEmpty())
                    {
                        select.append("email = ?");

                        vec_value.append(email);

                        if(!phone_number.isEmpty())
                        {
                            select.append(" OR ");
                        }
                    }
                    if(!phone_number.isEmpty())
                    {
                        select.append("phone_number = ?");

                        vec_value.append(phone_number);
                    }

                    select.append(";");


                    sql_builder.execQuery(select, vec_value);
                }

                if(sql_builder.isExec())
                {
                    is_valid = !sql_builder.isNext();

                    if(!is_valid)
                    {
                        if(!email.isEmpty() && (sql_builder.getValue("email").toString() == email))
                        {
                            error_response.insertError("such email already exists", "email", email);
                        }
                        else if(!phone_number.isEmpty() && (sql_builder.getValue("phone_number").toString() == phone_number))
                        {
                            error_response.insertError("such phone number already exists", "phone_number", phone_number);
                        }
                    }
                }
            };

            check_email_and_phone(Table::User);

            if(is_all_valid())
            {
                check_email_and_phone(Table::Not_Registered);
            }
        }
    }


    // Insert values into db
    if(is_all_valid())
    {
        if(!map_result.isEmpty())
        {
            auto updateDb = [this](const QVariantMap &map_result)
            {
                sql_builder.updateDB(map_result, "user", {"id_user", user_data.user_Id()});
            };

            if(!user_data.isDriver() && !zip_code.isEmpty())
            {
                QString address = state + " " + zip_code;

                auto function = [map_result = std::move(map_result), updateDb = std::move(updateDb)](Address &&address_struct) mutable
                {
                    map_result.insert("client_home_address", address_struct.address);
                    map_result.insert("client_home_latitude", address_struct.latitude);
                    map_result.insert("client_home_longitude", address_struct.longitude);

                    updateDb(map_result);

                    return Is_Finish_Handle::Yes;
                };

                getLatLongFromAddress(std::move(address), std::move(function));
            }
            else
            {
                updateDb(map_result);
            }
        }
    }

}



void User::POST_ChangePassword()
{
    QVariantMap map_result;


    // Check received json
    {
        Map_required_json_check map_key =
        {
            {
                "current_password", Json::Validation_Type::Password
            },
            {
                "new_password", Json::Validation_Type::Password
            },
            {
                "repeat_password", Json::Validation_Type::Password
            }
        };

        is_valid = json.validateJson(map_key, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }


    // Check does new_password equal repeat_password
    if(is_valid)
    {
        is_valid = (map_result.value("new_password").toString() == map_result.value("repeat_password").toString());

        if(!is_valid)
        {
            error_response.insertError("new_password and repeat_password do not match");
        }
    }


    // Check current_password
    if(is_valid)
    {
        const QVariantMap map_select =
        sql_builder.selectFromDB<QVariantMap>({"salt", "hash_password"}, "user", "id_user", user_data.user_Id(), is_valid);


        if(sql_builder.isExec())
        {
            // Check received password
            is_valid = checkHashPassword(map_result.value("current_password").toByteArray(), map_select.value("salt").toByteArray(), map_select.value("hash_password").toByteArray());

            if(!is_valid)
            {
                error_response.insertError("invalid current_password");
            }
        }
    }



    // Insert values into db
    if(is_all_valid())
    {
        QVariantMap map_col_value;

        QByteArray salt;

        const auto hash_password = getHashPassword(map_result.value("new_password").toByteArray(), salt);


        map_col_value.insert("salt", salt);
        map_col_value.insert("hash_password", hash_password);

        sql_builder.updateDB(map_col_value, "user", {"id_user", user_data.user_Id()});
    }
}



void User::POST_Contact_Us()
{
    QVariantMap map_result;


    // Check received json
    {
        Map_required_json_check map_key =
        {
            {
                "subject", Json::Validation_Type::Common_String_Max_255
            },
            {
                "message", Json::Validation_Type::Common_String_Max_255
            }
        };

        is_valid = json.validateJson(map_key, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }

    if(is_valid)
    {
        const QVector<QString> vec_key =
        {
            "full_name",
            "email",
            "phone_number",
            "is_driver"
        };

        const QVariantMap map_select =
        sql_builder.selectFromDB<QVariantMap>(vec_key, "user", "id_user", user_data.user_Id(), is_valid);

        if(sql_builder.isExec() && is_valid)
        {
            qDebug() << Global::getEmailContactUs();

            EmailData email_data(Global::getEmailContactUs().toUtf8(), map_result.value("subject").toByteArray(), __PRETTY_FUNCTION__);

            QByteArray message = "From: " + map_select.value("full_name").toByteArray();

            message.append("\r\nUser Type: " + QByteArray(map_select.value("is_driver").toBool() ? "Driver" : "Client"));

            message.append("\r\nEmail: " + map_select.value("email").toByteArray());
            message.append("\r\nPhone Number: " + map_select.value("phone_number").toByteArray());
            message.append("\r\nMessage: " + map_result.value("message").toByteArray());

            email_data.addText(std::move(message));

            EmailSender::sendContactUs(std::move(email_data));
        }
    }

}



void User::POST_AddPushToken()
{
    QVariant push_token;

    is_valid = json.validateJson({"push_token", Json::Validation_Type::Common_String_Max_255}, push_token, request_body.received_json, Json::Is_Required_Validation::Required);


    if(is_valid)
    {
        const QString insert_sql = "INSERT INTO push_token(id_user, push_token, platform) "
                                   "VALUES(?, ?, ?) "
                                   "ON DUPLICATE KEY UPDATE "
                                   "id_user = VALUES(id_user), "
                                   "push_token = VALUES(push_token), "
                                   "platform = VALUES(platform);";

        sql_builder.execQuery(insert_sql, {user_data.user_Id(), push_token, "IOS"});
    }
}



void User::GET_ZipCodeState()
{
    const QJsonObject object = getZipCodeByStateJson();

    if(is_valid)
    {
        response_body = object;
    }
}


void User::GET_CarModelList()
{
    const QJsonObject car_model_list = getCarModelListJson();

    if(is_valid)
    {
        response_body = car_model_list;
    }
}


void User::GET_PaymentMethod()
{
    const QString select = sql_builder.getSelectString({"last_four_numbers, exp_month, exp_year"}, "payment_method", "id_user");

    const auto payment_method = json.getObjectFromQuery(select, user_data.user_Id());

    if(sql_builder.isExec())
    {
        response_body = payment_method;
    }
}


void User::POST_SavePaymentMethod()
{
    QVariant card_token;

    is_valid = json.validateJson({"card_token", Json::Validation_Type::Common_String_Max_255}, card_token, request_body.received_json, Json::Is_Required_Validation::Required);


    // Check that client doesn't have opened order
    if(is_valid && !user_data.isDriver())
    {
        checkClientNotHaveOpenedOrder("you can't change the card while the order is open");
    }

    QString customer_account_id;

    QString old_card_token;

    // Get customer_account_id and current card_token
    if(is_all_valid())
    {
        const QString select = "SELECT u.customer_account_id, "
                               "p.card_token "
                               "FROM user AS u "
                               "LEFT JOIN payment_method AS p "
                               "ON u.id_user = p.id_user "
                               "WHERE u.id_user = ?;";

        sql_builder.execQuery(select, user_data.user_Id());

        if(sql_builder.isExec())
        {
            sql_builder.isNext();

            customer_account_id = sql_builder.getValue("customer_account_id").toString();
            old_card_token = sql_builder.getValue("card_token").toString();
        }
    }


    if(is_all_valid())
    {
        auto removeCard = [this](QString &&customer_account_id, QString &&old_card_token) mutable
        {
            qDebug() << "removeCard";

            is_foreign_api_using = true;

            auto response_func = [this]
            (QJsonObject &&response_body, ErrorResponse &&api_error_response)
            {
                error_response = std::move(api_error_response);

                finishHandle();
            };


            if(user_data.isDriver())
            {
                ForeignApiHandler::getStripeApi()->deleteCardFromAccount(std::move(customer_account_id), std::move(old_card_token), std::move(response_func));
            }
            else
            {
                ForeignApiHandler::getStripeApi()->deleteCardFromCustomer(std::move(customer_account_id), std::move(old_card_token), std::move(response_func));
            }
        };

        auto addCardAndRemoveOld = [this, card_token = card_token.toString(), old_card_token = std::move(old_card_token)]
        (QString &&customer_account_id, decltype (removeCard) &&removeCard) mutable
        {
            qDebug() << "addCardAndRemoveOld";

            is_foreign_api_using = true;

            auto response_func = [this, customer_account_id,  old_card_token = std::move(old_card_token), removeCard = std::move(removeCard)]
            (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
            {
                error_response = std::move(api_error_response);

                if(!error_response.isHaveLogError())
                {
                    const QString insert = "INSERT INTO payment_method(id_user, card_token, last_four_numbers, card_type, exp_month, exp_year) "
                                           "VALUES(?, ?, ?, ?, ?, ?) "
                                           "ON DUPLICATE KEY UPDATE "
                                           "card_token = VALUES(card_token), "
                                           "last_four_numbers = VALUES(last_four_numbers), "
                                           "card_type = VALUES(card_type), "
                                           "exp_month = VALUES(exp_month), "
                                           "exp_year = VALUES(exp_year);";

                    const QVector<QVariant> vec_value =
                    {
                        user_data.user_Id(),
                        response_body.value("id").toString(),
                        response_body.value("last4").toString(),
                        response_body.value("brand").toString(),
                        response_body.value("exp_month").toInt(),
                        response_body.value("exp_year").toInt()
                    };

                    sql_builder.execQuery(insert, vec_value);

                    if(sql_builder.isExec())
                    {
                        if(!old_card_token.isEmpty())
                        {
                            removeCard(std::move(customer_account_id), std::move(old_card_token));
                        }
                        else
                        {
                            finishHandle();
                        }
                    }
                    else
                    {
                        finishHandle();
                    }
                }
                else
                {
                    error_response.insertError("invalid card_token");

                    finishHandle();
                }
            };

            if(user_data.isDriver())
            {
                ForeignApiHandler::getStripeApi()->createCardToAccount(std::move(customer_account_id), std::move(card_token), std::move(response_func));
            }
            else
            {
                ForeignApiHandler::getStripeApi()->createCardToCustomer(std::move(customer_account_id), std::move(card_token), std::move(response_func));
            }
        };


        addCardAndRemoveOld(std::move(customer_account_id), std::move(removeCard));
    }
}


void User::GET_CheckAuthToken()
{
    addAuthToken(response_body, user_data.user_Id(), user_data.isDriver());

    if(sql_builder.isExec())
    {
        response_body.insert("is_driver", user_data.isDriver());
    }
}



//---Other---//
QByteArray User::getHashToken()
{
    QByteArray token = QByteArray::number(QDateTime::currentMSecsSinceEpoch()) + generateRandomString();

    token = QCryptographicHash::hash(token, QCryptographicHash::Sha3_256).toHex();

    return token;
}



QByteArray User::getHashPassword(const QByteArray &password, QByteArray &salt)
{
    salt = QByteArray::number(QDateTime::currentMSecsSinceEpoch()) + generateRandomString();

    salt = QCryptographicHash::hash(salt, QCryptographicHash::Sha3_256).toHex();

    return QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha3_256, password, salt, 10000, 50).toHex();
}



bool User::checkHashPassword(const QByteArray &password, const QByteArray &salt, const QByteArray hash_password)
{
    return (hash_password == QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha3_256, password, salt, 10000, 50).toHex());
}

void User::addAuthToken(QJsonObject &response_body, const int id_user, const bool is_driver)
{
    const QString auth_token = QString::fromUtf8(getHashToken());

    const QVariantMap map_col_value =
    {
        {
            "id_user", id_user
        },
        {
            "auth_token", auth_token
        },
        {
            "is_driver", is_driver
        }
    };

    sql_builder.insertIntoDB(map_col_value, "auth_token");

    if(sql_builder.isExec())
    {
        response_body.insert(QStringLiteral("auth_token"), auth_token);
    }
}

QString User::generateUniqueSmsCode()
{
    const QString sms_code = QString::number(QRandomGenerator::global()->bounded(1000, 9999));

    const QString select = "SELECT 1 FROM not_registered WHERE sms_code = ? "
                           "UNION ALL "
                           "SELECT 1 FROM restore_pass WHERE sms_code = ?;";

    sql_builder.execQuery(select, {sms_code, sms_code});

    if(sql_builder.isExec())
    {
        if(sql_builder.isNext())
        {
            return generateUniqueSmsCode();
        }
    }

    return sms_code;
}



QJsonObject User::getZipCodeByStateJson()
{
    QJsonObject object;

    QFile zip_code_by_state(Global::getPrimaryPath() + "/Configs/zip_code_by_state.json");

    if(zip_code_by_state.open(QIODevice::ReadOnly))
    {
        is_valid = true;

        object = QJsonDocument::fromJson(zip_code_by_state.readAll()).object();


        zip_code_by_state.close();
    }
    else
    {
        response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

        is_valid = false;

        const QJsonObject log_error =
        {
            {"description", "Can't open file " + zip_code_by_state.fileName()},
            {"file_error", zip_code_by_state.errorString()}
        };

        error_response.insertLogError(log_error);
    }

    return object;
}


QJsonObject User::getCarModelListJson()
{
    QJsonObject car_model_list;

    QFile file_car_model(Global::getPrimaryPath() + "/Configs/car_model.json");

    if(file_car_model.open(QIODevice::ReadOnly))
    {
        is_valid = true;

        car_model_list = QJsonDocument::fromJson(file_car_model.readAll()).object();

        file_car_model.close();
    }
    else
    {
        response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

        is_valid = false;

        const QJsonObject log_error =
        {
            {"description", "Can't open file " + file_car_model.fileName()},
            {"file_error", file_car_model.errorString()}
        };

        error_response.insertLogError(log_error);
    }

    return car_model_list;
}



void User::checkZipCodeAndState(const QString &zip_code, const QString &state)
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";


    const QJsonObject object = getZipCodeByStateJson();

    int zip_code_number{};

    if(is_valid)
    {
        zip_code_number = zip_code.toInt(&is_valid);

        if(!is_valid)
        {
            error_response.insertError("invalid zip_code", "zip_code", zip_code);
        }
    }

    if(is_valid)
    {
        auto it = object.constFind(state);

        is_valid = (it != object.constEnd());

        if(is_valid)
        {
            const QJsonArray array = it.value().toArray();

            bool valid_zip_code = false;

            for(const auto &object : array)
            {
                const int zip_min = object.toObject().value("Zip Min").toInt();
                const int zip_max = object.toObject().value("Zip Max").toInt();


                if(!valid_zip_code)
                {
                    valid_zip_code = (zip_code_number >= zip_min) && (zip_code_number <= zip_max);
                }

                if(valid_zip_code)
                {
                    break;
                }
            }

            is_valid = valid_zip_code;

            if(!is_valid)
            {
                error_response.insertError("invalid zip_code", "zip_code", zip_code);
            }
        }
        else
        {
            error_response.insertError("invalid state", "state", state);
        }
    }
}


template<class Function>
void User::getLatLongFromAddress(QString &&address, Function &&function)
{
    is_foreign_api_using = true;

    auto response_func = [this, address, function = std::move(function)]
    (QJsonObject &&api_response_body, ErrorResponse &&api_error_response) mutable
    {
        error_response = std::move(api_error_response);

        Is_Finish_Handle finish_handle = Is_Finish_Handle::No;


        if(!error_response.isHaveLogError())
        {
            const QString status_code = api_response_body.value("status").toString();

            if(status_code == QLatin1String("OK"))
            {
                const QJsonObject object = api_response_body.value("results").toArray().first().toObject();

                const QJsonObject location = object.value("geometry").toObject().value("location").toObject();

                Address address_struct =
                {
                    object.value("formatted_address").toString(),
                    location.value("lat").toDouble(),
                    location.value("lng").toDouble()
                };

                finish_handle = function(std::move(address_struct));
            }
            else if(status_code == QLatin1String("ZERO_RESULTS"))
            {
                error_response.insertError("invalid state and zip_code", "state_and_zip_code", address);

                finish_handle = Is_Finish_Handle::Yes;
            }
            else
            {
                response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                const QJsonObject log_error =
                {
                    {"description", "Geocoding API error: can't get lat/long from address"},
                    {"api_response_body", api_response_body},
                    {"address", address}
                };

                error_response.insertLogError(log_error);

                finish_handle = Is_Finish_Handle::Yes;
            }
        }
        else
        {
            error_response.insertError("invalid state and zip_code", "state_and_zip_code", address);

            finish_handle = Is_Finish_Handle::Yes;
        }


        if(finish_handle == Is_Finish_Handle::Yes)
        {
            finishHandle();
        }
    };

    ForeignApiHandler::getReverseGeocoding()->getLatLongFromAddress(std::move(address), std::move(response_func));
}
