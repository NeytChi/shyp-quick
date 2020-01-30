#include "order.h"

QMap<int, Order::Order_For_Searching> Order::static_map_order;


Order::Order(SqlBuilder &_sql_builder_, Response &_response_, const Send_Response_Function _send_response_function_)
    : AbstractRequestHandler (_sql_builder_, _response_, _send_response_function_)
{

}

Order::~Order()
{

}



const QHash<unsigned char, std::function<void(Order*)>> Order::static_handle_request_map =
{
    {
        Url_Post::Order_Client_Create_Order,
        &Order::POST_Client_CreateOrder
    },
    {
        Url_Post::Order_Client_Calc_Order,
        &Order::POST_Client_CalcOrder
    },
    {
        Url_Post::Order_Client_Cancel_Order,
        &Order::POST_Client_CancelOrder
    },
    {
        Url_Get::Order_Client_Get_Order_State,
        &Order::GET_Client_OrderState
    },
    {
        Url_Get::Order_Client_Last_Completed_Orders,
        &Order::GET_Client_LastCompletedOrders
    },
    {
        Url_Get::Order_Shipper_Check_Exist_New_Order,
        &Order::GET_Shipper_CheckExistNewOrder
    },
    {
        Url_Get::Order_Shipper_Get_Stats,
        &Order::GET_Shipper_Stats
    },
    {
        Url_Post::Order_Shipper_Accept_Order,
        &Order::POST_Shipper_AcceptOrder
    },
    {
        Url_Post::Order_Shipper_Decline_Order,
        &Order::POST_Shipper_DeclineOrder
    },
    {
        Url_Post::Order_Shipper_On_Pick_Up_Point,
        &Order::POST_Shipper_OnPickUpPoint
    },
    {
        Url_Post::Order_Shipper_On_Drop_Off_Point,
        &Order::POST_Shipper_OnDropOffPoint
    },
    {
        Url_Post::Order_Shipper_Update_Coordinates,
        &Order::POST_Shipper_UpdateCoordinates
    },
    {
        Url_Post::Order_Shipper_Ready_To_Ship,
        &Order::POST_Shipper_ReadyToShip
    },
    {
        Url_Get::Order_Shipper_Get_Paid,
        &Order::GET_Shipper_GetPaid
    },
    {
        Url_Get::Order_Get_Order_List,
        &Order::GET_OrderList
    },
    {
        Url_Get::Order_Get_Order_By_Id,
        &Order::GET_OrderById
    },
    {
        Url_Post::Order_Rate_User,
        &Order::POST_RateUser
    }

};


void Order::handle()
{
    AbstractRequestHandler::additionalHandle(static_handle_request_map, url_index, this);
}


void Order::GET_OrderList()
{
    {
        const QString user_str = (user_data.isDriver() ? "id_shipper" : "id_client");

        const QString price_str = (user_data.isDriver() ? "order_price_without_tax" : "ROUND(order_price_without_tax + tax, 2)");

        const QString select = "SELECT id_order, " + price_str + " AS price, time_of_delivery, "
                               "pick_up_longitude, pick_up_latitude, "
                               "drop_off_address, drop_off_longitude, drop_off_latitude "
                               "FROM orders "
                               "WHERE " + user_str + " = ? AND state = ? "
                               "ORDER BY creation_date DESC;";

        const auto order_list = json.getObjectListFromQuery(select, {user_data.user_Id(), Order_State::Completed});

        if(sql_builder.isExec())
        {
            response_body.insert("order_list", order_list);
        }
    }
}


void Order::GET_OrderById()
{
    const int id_order = checker.checkReceivedId("id_order", Checker::Received_Id_Type::Id);

    if(is_valid)
    {
        QJsonObject order;

        {
            const QString another_user_str = (user_data.isDriver() ? "client" : "shipper");

            const QString where_user_str = (user_data.isDriver() ? "shipper" : "client");

            const QString select = "SELECT o.id_order, o.state, "
                                   "u.id_user, u.full_name, u.profile_picture_path, "
                                   "o.last_four_numbers, o.card_type, "
                                   "o." + where_user_str + "_rated AS you_rated, "
                                   "o.pick_up_address, o.pick_up_longitude, o.pick_up_latitude, "
                                   "o.drop_off_address, o.drop_off_longitude, o.drop_off_latitude, "
                                   "o.order_price_without_tax, o.tax, o.distance_in_miles, o.time_to_deliver, o.time_of_delivery "
                                   "FROM orders AS o "
                                   "INNER JOIN user AS u "
                                   "ON o.id_" + another_user_str + " = u.id_user "
                                   "WHERE o.id_order = ? AND o.id_" + where_user_str + " = ?;";

            order = json.getObjectFromQuery(select, {id_order, user_data.user_Id()});

        }


        if(sql_builder.isExec())
        {
            checkDoesOrderExist(id_order);

            if(is_valid)
            {
                order.insert("profile_picture_path", getUrlPicturePath(order.value("profile_picture_path").toString()));
            }

            if(is_valid && user_data.isDriver())
            {
                order.remove("last_four_numbers");
                order.remove("card_type");
            }

            // Check order state
            if(is_valid)
            {
                const int current_order_state = order.take("state").toInt();

                is_valid = (current_order_state == Order_State::Completed);

                if(!is_valid)
                {
                    error_response.insertError("order not completed", "id_order", id_order);
                }
            }

            // Get item_list
            if(is_valid)
            {
                const QString select = "SELECT i.id_item, i.item_name "
                                       "FROM item AS i "
                                       "INNER JOIN item_for_order AS o "
                                       "ON i.id_item = o.id_item "
                                       "WHERE o.id_order = ?;";

                const auto item_list = json.getObjectListFromQuery(select, id_order);

                if(sql_builder.isExec())
                {
                    order.insert("item_list", item_list);

                    response_body = order;
                }
            }
        }
    }
}


void Order::POST_RateUser()
{
    QVariantMap map_result;

    // Check received json
    {
        Map_required_json_check map_key_required =
        {
            {
                "id_order", Json::Validation_Type::Id
            },
            {
                "rate", Json::Validation_Type::U_Int
            }
        };

        is_valid = json.validateJson(map_key_required, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }


    // Check rate
    if(is_valid)
    {
        const int rate = map_result.value("rate").toInt();

        is_valid = (rate >= 0) && (rate <= 5);

        if(!is_valid)
        {
            error_response.insertError("invalid rate", "rate", rate);
        }
    }


    if(is_valid)
    {
        const int id_order = map_result.value("id_order").toInt();

        const QString user_str = (user_data.isDriver() ? "shipper" : "client");

        const QVariantMap map_select =
        sql_builder.selectFromDB<QVariantMap>({user_str + "_rated", "state"}, "orders", {"id_order", "id_" + user_str}, {id_order, user_data.user_Id()}, is_valid);


        if(sql_builder.isExec())
        {
            checkDoesOrderExist(id_order);

            // Check order state
            if(is_valid)
            {
                const int current_order_state = map_select.value("state").toInt();

                is_valid = (current_order_state == Order_State::Completed);

                if(!is_valid)
                {
                    error_response.insertError("order doesn't completed", "id_order", id_order);
                }
            }


            // Check did user already rate
            if(is_valid)
            {
                const QVariant rate = map_select.value(user_str + "_rated");

                is_valid = rate.isNull();

                if(!is_valid)
                {
                    error_response.insertError("you already rated this order", "id_order", id_order);
                }
            }

            if(is_valid)
            {
                QSqlDatabase db = sql_builder.beginTransaction();

                if(sql_builder.isExec())
                {
                    sql_builder.updateDB({user_str + "_rated", map_result.value("rate")}, "orders", {"id_order", id_order});
                }


                if(sql_builder.isExec() && user_data.isDriver())
                {
                    sql_builder.updateDB({"shipper_order_state", Shipper_Order_State::Does_Not_Has_Order}, "user", {"id_user", user_data.user_Id()});
                }

                sql_builder.endTransaction(db);
            }
        }
    }
}



// Client


void Order::POST_Client_CreateOrder()
{
    QVariantMap map_result;

    // Check received json
    {
        Map_required_json_check map_key_required =
        {
            {
                "pick_up_longitude", Json::Validation_Type::Longitude
            },
            {
                "pick_up_latitude", Json::Validation_Type::Latitude
            },
            {
                "drop_off_longitude", Json::Validation_Type::Longitude
            },
            {
                "drop_off_latitude", Json::Validation_Type::Latitude
            },
            {
                "item_list", Json::Validation_Type::JsonArray
            }
        };

        is_valid = json.validateJson(map_key_required, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }


    // Check coordinates
    if(is_valid)
    {
        // Check pick_up_longitude and drop_off_longitude
        {
            is_valid = (!qFuzzyCompare(map_result.value("pick_up_longitude").toDouble(), map_result.value("drop_off_longitude").toDouble()));

            if(!is_valid)
            {
               error_response.insertError("pick_up_longitude and drop_off_longitude are same");
            }
        }


        // Check pick_up_latitude and drop_off_latitude
        if(is_valid)
        {
            is_valid = (!qFuzzyCompare(map_result.value("pick_up_latitude").toDouble(), map_result.value("drop_off_latitude").toDouble()));

            if(!is_valid)
            {
               error_response.insertError("pick_up_latitude and drop_off_latitude are same");
            }
        }
    }


    QString card_token;

    // Check does client have saved card
    if(is_valid)
    {
        const QVariantMap map_select =
        sql_builder.selectFromDB<QVariantMap>({"card_token", "last_four_numbers", "card_type"}, "payment_method", "id_user", user_data.user_Id(), is_valid);

        if(sql_builder.isExec())
        {
            if(is_valid)
            {
                card_token = map_select.value("card_token").toString();

                map_result.insert("last_four_numbers", map_select.value("last_four_numbers"));
                map_result.insert("card_type", map_select.value("card_type"));
            }
            else
            {
                error_response.insertError("to create an order, you need to save the card");
            }
        }
    }

    QString customer_account_id;


    // Get customer_account_id
    if(is_all_valid())
    {
        const QVariant select = sql_builder.selectFromDB("customer_account_id", "user", "id_user", user_data.user_Id(), is_valid);

        if(sql_builder.isExec())
        {
            if(is_valid)
            {
                customer_account_id = select.toString();
            }
            else
            {
                response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                const QJsonObject log_error =
                {
                    {"description", "Can't get customer_account_id"},
                    {"id_user", user_data.user_Id()}
                };

                error_response.insertLogError(log_error);
            }
        }
    }

    const Order_Price order_price =
    (is_all_valid() ? checkItemsAndCalcOrderPrice(map_result.value("item_list").toJsonArray()) : Order_Price());

    // Check that user doesn't have opened order
    if(is_all_valid())
    {
        checkClientNotHaveOpenedOrder("you already have opened order");
    }


    if(is_all_valid())
    {
        struct Additional_Values
        {
            Order_Price order_price;
            Distance distance;
        };

        auto insert_values_in_db = [this](const QVariantMap &map_result, Additional_Values &additional_values)
        {
            qDebug() << "----FUNC insert_values_in_db----";

            int id_order{};

            //---- Insert values into db
            if(is_all_valid())
            {
                QSqlDatabase db = sql_builder.beginTransaction();


                // Add order
                if(sql_builder.isExec())
                {
                    const QVariantMap map_col_value =
                    {
                        {"id_client", user_data.user_Id()},

                        {"stripe_charge_id", map_result.value("stripe_charge_id").toString()},
                        {"stripe_charge_status", Order_Charge_Status::Authorize},
                        {"last_four_numbers", map_result.value("last_four_numbers").toString()},
                        {"card_type", map_result.value("card_type").toString()},

                        {"state", Order_State::Finding_Shipper},
                        {"is_open", true},

                        {"pick_up_address", formatAddress(additional_values.distance.origin_address)},
                        {"pick_up_longitude", map_result.value("pick_up_longitude").toDouble()},
                        {"pick_up_latitude", map_result.value("pick_up_latitude").toDouble()},

                        {"drop_off_address", formatAddress(additional_values.distance.destination_address)},
                        {"drop_off_longitude", map_result.value("drop_off_longitude").toDouble()},
                        {"drop_off_latitude", map_result.value("drop_off_latitude").toDouble()},

                        {"order_price_without_tax", additional_values.order_price.order_price_without_tax},
                        {"tax", additional_values.order_price.tax},
                        {"distance_in_miles", additional_values.distance.distance_miles},
                        {"time_to_deliver", additional_values.distance.time_to_deliver_in_minutes},
                        {"arrive_by", QDateTime::currentDateTime().addSecs(additional_values.distance.time_to_deliver_in_minutes * 60)},
                        {"creation_date", QDateTime::currentDateTime()}
                    };


                    sql_builder.insertIntoDB(map_col_value, "orders");
                }


                id_order = (sql_builder.isExec() ? sql_builder.lastInsertId().toInt() : 0);

                // Add items
                if(sql_builder.isExec())
                {
                    const auto array_item_list = map_result.value("item_list").toJsonArray();

                    const QStringList list_key =
                    {
                        "id_item",
                        "id_order"
                    };

                    QVector<QVector<QVariant>> vec_value;

                    for(const auto &value : array_item_list)
                    {
                        vec_value.append({value.toInt(), id_order});
                    }

                    sql_builder.insertMultiplyIntoDB(list_key, vec_value, "item_for_order");
                }

                if(sql_builder.isExec())
                {
                    Order_For_Searching order_for_searching =
                    {
                        0,
                        map_result.value("pick_up_longitude").toDouble(),
                        map_result.value("pick_up_latitude").toDouble()
                    };

                    createTimerForOrderAndInsertOrderToMap(id_order, sql_builder.getConnectionName(), std::move(order_for_searching));

                    findShipperForOrder(id_order, sql_builder.getConnectionName());
                }

                sql_builder.endTransaction(db);
            }


            if(is_all_valid())
            {
                this->response_body.insert("id_order", id_order);
                this->response_body.insert("order_price", Global::roundDoubleToTwoPointsDecimal(additional_values.order_price.order_price_without_tax + additional_values.order_price.tax));
                this->response_body.insert("distance_in_miles", additional_values.distance.distance_miles);
                this->response_body.insert("time_to_deliver", additional_values.distance.time_to_deliver_in_minutes);
            }

        };


        auto authorizeCharge = [this, card_token = std::move(card_token), customer_account_id = std::move(customer_account_id), insert_values_in_db = std::move(insert_values_in_db)]
        (QVariantMap &&map_result, Additional_Values &&additional_values)
        {
            is_foreign_api_using = true;


            auto response_func = [this, map_result = std::move(map_result), additional_values = std::move(additional_values), insert_values_in_db = std::move(insert_values_in_db)]
            (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
            {
                error_response = std::move(api_error_response);

                if(!error_response.isHaveLogError())
                {
                    map_result.insert("stripe_charge_id", response_body.value("id").toString());

                    insert_values_in_db(map_result, additional_values);
                }
                else
                {
                    error_response.insertError("you do not have enough funds on the card");
                }

                finishHandle();
            };


            StripeApi::Charge charge =
            {
                customer_account_id,
                additional_values.order_price.order_price_without_tax + additional_values.order_price.tax,
                card_token
            };

            ForeignApiHandler::getStripeApi()->authorizeCharge(std::move(charge), std::move(response_func));
        };



        DistanceMatrixGoogle::Coordinates coordinates =
        {
            map_result.value("pick_up_latitude").toDouble(),
            map_result.value("pick_up_longitude").toDouble(),

            map_result.value("drop_off_latitude").toDouble(),
            map_result.value("drop_off_longitude").toDouble()
        };


        auto function = [this, additional_values = Additional_Values({order_price, {}}), map_result = std::move(map_result), authorizeCharge = std::move(authorizeCharge)](Distance &&distance) mutable
        {
            additional_values.distance = std::move(distance);

            calcOrderPriceDistance(additional_values.order_price, additional_values.distance.distance_miles);

            authorizeCharge(std::move(map_result), std::move(additional_values));

            return Is_Finish_Handle::No;
        };


        getDistanceAnd(std::move(coordinates), std::move(function));
    }
}


void Order::POST_Client_CalcOrder()
{
    QVariantMap map_result;

    // Check received json
    {
        Map_required_json_check map_key_required =
        {
            {
                "pick_up_longitude", Json::Validation_Type::Longitude
            },
            {
                "pick_up_latitude", Json::Validation_Type::Latitude
            },
            {
                "drop_off_longitude", Json::Validation_Type::Longitude
            },
            {
                "drop_off_latitude", Json::Validation_Type::Latitude
            },
            {
                "item_list", Json::Validation_Type::JsonArray
            }
        };

        is_valid = json.validateJson(map_key_required, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }


    // Check coordinates
    if(is_valid)
    {
        // Check pick_up_longitude and drop_off_longitude
        {
            is_valid = (!qFuzzyCompare(map_result.value("pick_up_longitude").toDouble(), map_result.value("drop_off_longitude").toDouble()));

            if(!is_valid)
            {
               error_response.insertError("pick_up_longitude and drop_off_longitude are same");
            }
        }


        // Check pick_up_latitude and drop_off_latitude
        if(is_valid)
        {
            is_valid = (!qFuzzyCompare(map_result.value("pick_up_latitude").toDouble(), map_result.value("drop_off_latitude").toDouble()));

            if(!is_valid)
            {
               error_response.insertError("pick_up_latitude and drop_off_latitude are same");
            }
        }
    }

    const Order_Price order_price =
    (is_valid ? checkItemsAndCalcOrderPrice(map_result.value("item_list").toJsonArray()) : Order_Price());


    // Check that user doesn't have opened order
    if(is_all_valid())
    {
        checkClientNotHaveOpenedOrder("you already have opened order");
    }


    if(is_all_valid())
    {
        auto function = [this, order_price = std::move(order_price)](Distance &&distance) mutable
        {
            calcOrderPriceDistance(order_price, distance.distance_miles);

            this->response_body =
            {
                {"order_price", Global::roundDoubleToTwoPointsDecimal(order_price.order_price_without_tax + order_price.tax)},
                {"distance_in_miles", distance.distance_miles},
                {"time_to_deliver", distance.time_to_deliver_in_minutes}
            };

            return Is_Finish_Handle::Yes;
        };

        DistanceMatrixGoogle::Coordinates coordinates =
        {
            map_result.value("pick_up_latitude").toDouble(),
            map_result.value("pick_up_longitude").toDouble(),

            map_result.value("drop_off_latitude").toDouble(),
            map_result.value("drop_off_longitude").toDouble()
        };

        getDistanceAnd(std::move(coordinates), std::move(function));
    }

}


void Order::POST_Client_CancelOrder()
{
    const int id_order = checker.checkReceivedId("id_order", Checker::Received_Id_Type::Id);


    if(is_valid)
    {
        const QVariantMap map_select =
        sql_builder.selectFromDB<QVariantMap>({"id_shipper", "state", "is_open", "stripe_charge_id"}, "orders", {"id_order", "id_client"}, {id_order, user_data.user_Id()}, is_valid);

        if(sql_builder.isExec())
        {
            checkDoesOrderExist(id_order);

            if(is_valid)
            {
                checkIsOrderOpen(id_order, map_select.value("is_open").toBool());
            }
        }



        // Check order state
        if(is_all_valid())
        {
            const int current_order_state = (is_all_valid() ? map_select.value("state").toInt() : 0);

            is_valid = (current_order_state == Order_State::Finding_Shipper) ||
                       (current_order_state == Order_State::Accepted);

            if(!is_valid)
            {
                error_response.insertError("you can't cancel the order in shipping state", "id_order", id_order);
            }
        }


        if(is_all_valid())
        {
            const int id_shipper = map_select.value("id_shipper").toInt();

            auto update_db = [this, id_order, id_shipper]()
            {
                qDebug() << "--FUNC = update_db()--";

                QSqlDatabase db = sql_builder.beginTransaction();

                // Update Order
                if(sql_builder.isExec())
                {
                    const QVariantMap map_col_value =
                    {
                        {"id_shipper", {}},
                        {"is_open", false},
                        {"state", Order_State::Canceled}
                    };

                    sql_builder.updateDB(map_col_value, "orders", {"id_order", id_order});
                }


                // Set that shipper has no order (shipper_order_state)
                if(sql_builder.isExec() && (id_shipper > 0))
                {
                    sql_builder.updateDB({"shipper_order_state", Shipper_Order_State::Does_Not_Has_Order}, "user", {"id_user", id_shipper});
                }

                sql_builder.endTransaction(db);
            };

            update_db();

            if(sql_builder.isExec())
            {
                deleteTimerAndRemoveOrderFromMap(id_order);
            }


            auto refundUncapturedCharge = [this](QString &&stripe_charge_id) mutable
            {
                qDebug() << "--FUNC = refundUncapturedCharge()--";

                is_foreign_api_using = true;

                auto response_func = [this]
                (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
                {
                    qDebug() << "--RESPONSE FUNC = refundUncapturedCharge()--";

                    error_response = std::move(api_error_response);

                    if(error_response.isHaveLogError())
                    {
                        response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                        const QJsonObject log_error =
                        {
                            {"description", "Stripe API error: can't refund uncaptured charge"}
                        };

                        error_response.insertLogError(log_error);
                    }


                    finishHandle();
                };


                ForeignApiHandler::getStripeApi()->refundUncapturedCharge(std::move(stripe_charge_id), std::move(response_func));
            };

            refundUncapturedCharge(map_select.value("stripe_charge_id").toString());
        }
    }
}


void Order::GET_Client_OrderState()
{
    const int id_order = checker.checkReceivedId("id_order", Checker::Received_Id_Type::Id);

    if(is_valid)
    {
        QJsonObject order;

        {
            const QVector<QString> vec_col =
            {
                "id_order", // All
                "id_shipper", // remove below

                "last_four_numbers", // Finding_Shipper
                "card_type", // Finding_Shipper

                "state AS order_state", // All

                "pick_up_longitude", // Finding_Shipper, Accepted
                "pick_up_latitude", // Finding_Shipper, Accepted
                "pick_up_address", // All

                "drop_off_address", // Finding_Shipper, Completed
                "drop_off_longitude", // Finding_Shipper, In_Shipping
                "drop_off_latitude", // Finding_Shipper, In_Shipping

                "ROUND(order_price_without_tax + tax, 2) AS order_price", // Finding_Shipper
                "arrive_by" // Finding_Shipper
            };

            const QString select = sql_builder.getSelectString(vec_col, "orders", {"id_order", "id_client"});

            order = json.getObjectFromQuery(select, {id_order, user_data.user_Id()});
        }


        if(sql_builder.isExec())
        {
            checkDoesOrderExist(id_order);
        }


        if(is_all_valid())
        {
            const int current_order_state = order.value("order_state").toInt();

            const int id_shipper = order.take("id_shipper").toInt();

            if( (current_order_state == Order_State::Accepted) ||
                (current_order_state == Order_State::In_Shipping) ||
                (current_order_state == Order_State::Completed))
            {

                // Remove unnecessary
                {
                    if(current_order_state == Order_State::Accepted)
                    {
                        order =
                        {
                            {"order_state", order.take("order_state")},
                            {"pick_up_address", order.take("pick_up_address")},
                            {"pick_up_longitude", order.take("pick_up_longitude")},
                            {"pick_up_latitude", order.take("pick_up_latitude")}
                        };
                    }
                    else if(current_order_state == Order_State::In_Shipping)
                    {
                        order =
                        {
                            {"order_state", order.take("order_state")},
                            {"drop_off_longitude", order.take("drop_off_longitude")},
                            {"drop_off_latitude", order.take("drop_off_latitude")},
                            {"pick_up_address", order.take("pick_up_address")}
                        };
                    }
                    else if(current_order_state == Order_State::Completed)
                    {
                        order =
                        {
                            {"order_state", order.take("order_state")},
                            {"pick_up_address", order.take("pick_up_address")},
                            {"drop_off_address", order.take("drop_off_address")}
                        };
                    }
                }

                QJsonObject shipper;

                // Select shipper
                {
                    QVector<QString> vec_key =
                    {
                        "full_name",
                        "profile_picture_path",
                        "shipper_longitude",
                        "shipper_latitude",
                        "phone_number"
                    };

                    if(current_order_state != Order_State::Completed)
                    {
                        vec_key.append("car_model");
                        vec_key.append("plate_number");
                    }

                    const QString select = sql_builder.getSelectString(vec_key, "user", "id_user");

                    shipper = json.getObjectFromQuery(select, id_shipper);

                    if(sql_builder.isExec())
                    {
                        shipper.insert("profile_picture_path", getUrlPicturePath(shipper.value("profile_picture_path").toString()));
                    }
                }

                // Select shipper rating
                if(sql_builder.isExec())
                {
                    const QVariant shipper_rating =
                    sql_builder.selectFromDB("ROUND(AVG(client_rated), 1)", "orders", {"id_shipper", "state"}, {id_shipper, Order_State::Completed}, is_valid);

                    shipper.insert("shipper_rating", shipper_rating.toDouble());
                }


                if(sql_builder.isExec())
                {
                    order.insert("shipper", shipper);
                }
            }

            if(sql_builder.isExec())
            {
                response_body = order;
            }
        }
    }
}


void Order::GET_Client_LastCompletedOrders()
{

    {
        const QString select = "SELECT id_order, "
                               "pick_up_address, pick_up_longitude, pick_up_latitude, "
                               "drop_off_address, drop_off_longitude, drop_off_latitude "
                               "FROM `orders` "
                               "WHERE id_client = ? AND state = ? "
                               "ORDER BY id_order DESC "
                               "LIMIT 5;";

        sql_builder.execQuery(select, {user_data.user_Id(), Order_State::Completed});
    }

    if(sql_builder.isExec())
    {
        QJsonArray last_id_order_list;

        QJsonArray last_pick_up_address_list, last_drop_off_address_list;

        while(sql_builder.isNext())
        {
            last_id_order_list.append(sql_builder.getValue("id_order").toInt());

            const QJsonObject last_pick_up =
            {
                {"pick_up_address", sql_builder.getValue("pick_up_address").toString()},
                {"pick_up_longitude", sql_builder.getValue("pick_up_longitude").toDouble()},
                {"pick_up_latitude", sql_builder.getValue("pick_up_latitude").toDouble()}
            };

            last_pick_up_address_list.append(last_pick_up);


            const QJsonObject last_drop_off =
            {
                {"drop_off_address", sql_builder.getValue("drop_off_address").toString()},
                {"drop_off_longitude", sql_builder.getValue("drop_off_longitude").toDouble()},
                {"drop_off_latitude", sql_builder.getValue("drop_off_latitude").toDouble()}
            };

            last_drop_off_address_list.append(last_drop_off);
        }

        QJsonArray last_item_list, item_list;

        // Get last item list
        if(!last_id_order_list.isEmpty())
        {
            QString select = "SELECT o.id_order, GROUP_CONCAT(i.item_name SEPARATOR ', ') "
                             "FROM item_for_order AS o "
                             "INNER JOIN item AS i "
                             "ON o.id_item = i.id_item "
                             "WHERE " + checker.getListId(last_id_order_list, "o.id_order");

            // Cut ';' at the end
            select.chop(1);

            select.append("GROUP BY o.id_order;");

            sql_builder.execQuery(select);

            if(sql_builder.isExec())
            {
                while(sql_builder.isNext())
                {
                    last_item_list.append(sql_builder.getValue(1).toString());
                }
            }
        }


        if(sql_builder.isExec())
        {
            const QString select = "SELECT id_item, item_name "
                                   "FROM item;";

            sql_builder.execQuery(select);

            if(sql_builder.isExec())
            {
                while(sql_builder.isNext())
                {
                    const QJsonObject temp =
                    {
                        {"id_item", sql_builder.getValue(0).toInt()},
                        {"item_name", sql_builder.getValue(1).toString()}
                    };

                    item_list.append(temp);
                }
            }
        }


        if(sql_builder.isExec())
        {
            response_body.insert("last_pick_up_address_list", last_pick_up_address_list);
            response_body.insert("last_drop_off_address_list", last_drop_off_address_list);
            response_body.insert("last_item_list", last_item_list);
            response_body.insert("item_list", item_list);
        }
    }
}



// Shipper


void Order::GET_Shipper_Stats()
{
    QJsonObject shipper_stats;

    // Select stats
    {
        const QString select = "SELECT COUNT(id_order) AS total_orders, "
                               "SUM(order_price_without_tax) AS total_earnings, "
                               "SUM(distance_in_miles) AS total_miles "
                               "FROM orders "
                               "WHERE id_shipper = ? AND state = ?;";

        shipper_stats = json.getObjectFromQuery(select, {user_data.user_Id(), Order_State::Completed});
    }

    // Select total_days
    if(sql_builder.isExec())
    {
        const QVariant registration_date =
        sql_builder.selectFromDB("registration_date", "user", "id_user", user_data.user_Id(), is_valid);

        if(sql_builder.isExec())
        {
            shipper_stats.insert("total_days", registration_date.toDateTime().toUTC().daysTo(QDateTime::currentDateTimeUtc()));
        }
    }

    // We take data on earned money for 7 days
    if(sql_builder.isExec())
    {
        // 1=Sunday, 2=Monday, 3=Tuesday, 4=Wednesday, 5=Thursday, 6=Friday, 7=Saturday.

        int current_day = 0;

        {
            // 1 = Monday to 7 = Sunday
            current_day = QDate::currentDate().dayOfWeek();

            if(current_day == 7)
            {
                current_day = 1;
            }
            else
            {
                ++current_day;
            }
        }

        QVector<double> vec_day(7);

        {
            const QString select = "SELECT DAYOFWEEK(time_of_delivery), SUM(order_price_without_tax) "
                                   "FROM orders "
                                   "WHERE time_of_delivery >= DATE(NOW() - INTERVAL 7 DAY) AND time_of_delivery <= NOW() "
                                   "AND id_shipper = ? AND state = ? "
                                   "GROUP BY DAYOFWEEK(time_of_delivery), DATE(time_of_delivery) "
                                   "ORDER BY DATE(time_of_delivery) DESC;";

            sql_builder.execQuery(select, {user_data.user_Id(), Order_State::Completed});

            if(sql_builder.isExec())
            {
                while(sql_builder.isNext())
                {
                    int index = 0;

                    const int day_of_week = sql_builder.getValue(0).toInt();
                    const double total_price = sql_builder.getValue(1).toDouble();

                    if(current_day >= day_of_week)
                    {
                        index = 7 - (current_day - day_of_week);
                    }
                    else
                    {
                        index = std::abs(current_day - day_of_week);
                    }

                    vec_day.replace(index - 1, total_price);
                }
            }
        }

        if(sql_builder.isExec())
        {
            QJsonArray weekly_payment_list;

            for(const double total_price : vec_day)
            {
                weekly_payment_list.append(total_price);
            }

            shipper_stats.insert("weekly_payment_list", weekly_payment_list);
        }
    }


    QString customer_account_id;


    // Get customer_account_id
    if(sql_builder.isExec())
    {
        const QVariant select = sql_builder.selectFromDB("customer_account_id", "user", "id_user", user_data.user_Id(), is_valid);

        if(sql_builder.isExec())
        {
            if(is_valid)
            {
                customer_account_id = select.toString();
            }
        }
    }


    if(sql_builder.isExec())
    {
        auto getDriverBalance = [this, customer_account_id = std::move(customer_account_id), shipper_stats = std::move(shipper_stats)]() mutable
        {
            qDebug() << "--FUNC = getDriverBalance()--";

            is_foreign_api_using = true;

            auto response_func = [this, shipper_stats = std::move(shipper_stats)]
            (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
            {
                qDebug() << "--RESPONSE FUNC = getDriverBalance()--";

                error_response = std::move(api_error_response);

                if(!error_response.isHaveLogError())
                {
                    int amount_cents{};

                    const QJsonArray available_array = response_body.value("available").toArray();

                    for(const auto &value : available_array)
                    {
                        amount_cents += value.toObject().value("amount").toInt();
                    }

                    const double balance = Global::roundDoubleToTwoPointsDecimal((amount_cents * 1.0) / 100);

                    shipper_stats.insert("balance", balance);

                    this->response_body = shipper_stats;
                }
                else
                {
                    response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                    const QJsonObject log_error =
                    {
                        {"description", "Stripe API error: can't get driver balance"}
                    };

                    error_response.insertLogError(log_error);

                }

                finishHandle();
            };

            ForeignApiHandler::getStripeApi()->getBalanceFromAccount(std::move(customer_account_id), std::move(response_func));
        };

        getDriverBalance();
    }
}


void Order::GET_Shipper_CheckExistNewOrder()
{
    QJsonObject new_order;

    // Get order
    {
        const QVector<QString> vec_key =
        {
            "id_order",

            "id_client",

            "state AS order_state",

            "pick_up_address",
            "pick_up_longitude",
            "pick_up_latitude",

            "drop_off_address",
            "drop_off_longitude",
            "drop_off_latitude",

            "order_price_without_tax",
            "distance_in_miles",
            "arrive_by"
        };

        const QString select = sql_builder.getSelectString(vec_key, "orders", "shipper_rated IS NULL AND state != ? AND id_shipper");

        new_order = json.getObjectFromQuery(select, {Order_State::Canceled, user_data.user_Id()});
    }


    if(is_all_valid())
    {
        // Select shipper coordinates
        if(is_all_valid())
        {
            const QVariantMap map_select =
            sql_builder.selectFromDB<QVariantMap>({"shipper_longitude", "shipper_latitude"}, "user", "id_user", user_data.user_Id(), is_valid);

            if(sql_builder.isExec())
            {
                new_order.insert("shipper_longitude", map_select.value("shipper_longitude").toDouble());
                new_order.insert("shipper_latitude", map_select.value("shipper_latitude").toDouble());
            }
        }


        // Select client name
        if(is_all_valid())
        {
            const QVariantMap map_select =
            sql_builder.selectFromDB<QVariantMap>({"full_name", "profile_picture_path", "phone_number"}, "user", "id_user", new_order.take("id_client").toInt(), is_valid);

            if(sql_builder.isExec())
            {
                new_order.insert("client_full_name", map_select.value("full_name").toString());
                new_order.insert("client_profile_picture_path", getUrlPicturePath(map_select.value("profile_picture_path").toString()));
                new_order.insert("client_phone_number", map_select.value("phone_number").toString());
            }
        }

        // Get client rating and past orders
        if(is_all_valid())
        {
            const QString select = "SELECT ROUND(AVG(shipper_rated), 1) AS client_rating, "
                                   "COUNT(id_order) AS client_past_orders "
                                   "FROM orders "
                                   "WHERE state = ?;";

            sql_builder.execQuery(select, Order_State::Completed);

            if(sql_builder.isExec())
            {
                sql_builder.isNext();

                new_order.insert("client_rating", sql_builder.getValue(0).toDouble());
                new_order.insert("client_past_orders", sql_builder.getValue(1).toInt());
            }
        }


        // Get Item List
        if(is_all_valid())
        {
            const QString select = "SELECT i.item_name "
                                   "FROM item AS i "
                                   "INNER JOIN item_for_order AS o "
                                   "ON i.id_item = o.id_item "
                                   "WHERE o.id_order = ?;";

            sql_builder.execQuery(select, new_order.value("id_order").toInt());

            if(sql_builder.isExec())
            {
                QJsonArray item_list;

                while(sql_builder.isNext())
                {
                    item_list.append(sql_builder.getValue(0).toString());
                }

                new_order.insert("item_list", item_list);
            }
        }

        if(is_all_valid())
        {
            response_body.insert("orders", new_order);
        }

    }

}



void Order::POST_Shipper_AcceptOrder()
{
    const int id_order = checker.checkReceivedId("id_order", Checker::Received_Id_Type::Id);

    if(is_valid)
    {
        const QVariantMap map_select =
        sql_builder.selectFromDB<QVariantMap>({"id_shipper", "state"}, "orders", "id_order", id_order, is_valid);

        if(sql_builder.isExec())
        {
            checkDoesOrderExist(id_order);

            // Check, that system offered specified order to this shipper
            if(is_valid)
            {
                is_valid = (map_select.value("id_shipper").toInt() == user_data.user_Id());

                if(!is_valid)
                {
                    error_response.insertError("order unavailable", "id_order", id_order);
                }
            }


            // Check order state
            if(is_valid)
            {
                const int current_order_state = map_select.value("state").toInt();

                is_valid = (current_order_state == Order_State::Finding_Shipper);

                if(!is_valid)
                {
                    if(current_order_state == Order_State::Canceled)
                    {
                        error_response.insertError("order was canceled", "id_order", id_order);
                    }
                    else
                    {
                        error_response.insertError("you have already accepted this order", "id_order", id_order);
                    }
                }
            }
        }


        // Update order and shipper
        if(is_all_valid())
        {
            QSqlDatabase db = sql_builder.beginTransaction();

            // Update Order
            if(sql_builder.isExec())
            {
                sql_builder.updateDB({"state", Order_State::Accepted}, "orders", {"id_order", id_order});
            }

            // Set that shipper accepted order (shipper_order_state)
            if(sql_builder.isExec())
            {
                sql_builder.updateDB({"shipper_order_state", Shipper_Order_State::Has_Accepted_Order}, "user", {"id_user", user_data.user_Id()});
            }

            sql_builder.endTransaction(db);
        }


        if(is_all_valid())
        {
            deleteTimerAndRemoveOrderFromMap(id_order);
        }
    }
}



void Order::POST_Shipper_DeclineOrder()
{
    const int id_order = checker.checkReceivedId("id_order", Checker::Received_Id_Type::Id);


    if(is_valid)
    {
        const QVariantMap map_select =
        sql_builder.selectFromDB<QVariantMap>({"id_shipper", "state"}, "orders", "id_order", id_order, is_valid);

        if(sql_builder.isExec())
        {
            checkDoesOrderExist(id_order);


            // Check, that system offered specified order to this shipper
            if(is_valid)
            {
                is_valid = (map_select.value("id_shipper").toInt() == user_data.user_Id());

                if(!is_valid)
                {
                    error_response.insertError("order unavailable", "id_order", id_order);
                }
            }

            // Check order state
            if(is_valid)
            {
                const int current_order_state = map_select.value("state").toInt();

                is_valid = (current_order_state == Order_State::Finding_Shipper);

                if(!is_valid)
                {
                    if(current_order_state == Order_State::Canceled)
                    {
                        error_response.insertError("order was canceled", "id_order", id_order);
                    }
                    else
                    {
                        error_response.insertError("you have already accepted this order", "id_order", id_order);
                    }
                }
            }
        }


        if(is_all_valid())
        {
            QSqlDatabase db = sql_builder.beginTransaction();

            // Set id_shipper = NULL in this order
            if(sql_builder.isExec())
            {
                sql_builder.updateDB({"id_shipper", {}}, "orders", {"id_order", id_order});
            }

            // Update that shipper doesn't have order
            if(sql_builder.isExec())
            {
                sql_builder.updateDB({"shipper_order_state", Shipper_Order_State::Does_Not_Has_Order}, "user", {"id_user", map_select.value("id_shipper").toInt()});
            }

            sql_builder.endTransaction(db);
        }


        if(is_all_valid())
        {
            findShipperForOrder(id_order, sql_builder.getConnectionName());
        }
    }
}



void Order::POST_Shipper_OnPickUpPoint()
{
    const int id_order = checker.checkReceivedId("id_order", Checker::Received_Id_Type::Id);

    // Check that user has opened order
    if(is_valid)
    {
        const QVariantMap map_select =
        sql_builder.selectFromDB<QVariantMap>({"id_client", "state"}, "orders", {"id_order", "id_shipper"}, {id_order, user_data.user_Id()}, is_valid);


        if(sql_builder.isExec())
        {
            checkDoesOrderExist(id_order);

            if(is_valid)
            {
                const int current_order_state = map_select.value("state").toInt();

                is_valid = (current_order_state == Order_State::Accepted);

                if(!is_valid)
                {
                    error_response.insertError("the order is not in the \"Accepted\" status, so you can't start delivery", "id_order", id_order);
                }
            }
        }


        // Update order state
        if(is_all_valid())
        {
            sql_builder.updateDB({"state", Order_State::In_Shipping}, "orders", {"id_order", id_order});

            if(sql_builder.isExec())
            {
                // Send Push To Client
                {
                    QString select = "SELECT id_user, push_token "
                                     "FROM push_token "
                                     "WHERE id_user = ?;";


                    PusherHandler::getHandler().sendPush(&PusherHandler::pushToClientShipperOnPickUpPoint, std::move(select), {map_select.value("id_client")});
                }
            }
        }
    }
}


void Order::POST_Shipper_OnDropOffPoint()
{
    const int id_order = checker.checkReceivedId("id_order", Checker::Received_Id_Type::Id);

    // Check that user has opened order
    if(is_valid)
    {
        QVariantMap map_select;

        {
            const QVector<QString> vec_key =
            {
                "id_client",
                "state",
                "order_price_without_tax",
                "stripe_charge_id"
            };

            map_select = sql_builder.selectFromDB<QVariantMap>(vec_key, "orders", {"id_order", "id_shipper"}, {id_order, user_data.user_Id()}, is_valid);
        }


        // Check order state
        if(sql_builder.isExec())
        {
            checkDoesOrderExist(id_order);

            if(is_valid)
            {
                const int current_order_state = map_select.value("state").toInt();

                is_valid = (current_order_state == Order_State::In_Shipping);

                if(!is_valid)
                {
                    error_response.insertError("the order is not in the \"In_Shipping\" status, so you can't complete order", "id_order", id_order);
                }
            }
        }

        QString driver_account_id;

        // Get driver_account_id
        if(is_all_valid())
        {
            const QVariant select =
            sql_builder.selectFromDB("customer_account_id", "user", "id_user", user_data.user_Id(), is_valid);

            if(is_all_valid())
            {
                driver_account_id = select.toString();
            }
        }


        if(is_all_valid())
        {
            if(!captureRequest({__FUNCTION__, id_order}))
            {
                error_response.insertError("already doing this request");

                return;
            }

            auto update_db_and_send_push = [this, id_order, id_client = map_select.value("id_client").toInt()](QString &&transfer_id)
            {
                qDebug() << "--FUNC = update_db_and_send_push()--";

                QSqlDatabase db = sql_builder.beginTransaction();

                // Update Order
                if(sql_builder.isExec())
                {
                    const QVariantMap map_col_value =
                    {
                        {"state", Order_State::Completed},
                        {"is_open", false},
                        {"time_of_delivery", QDateTime::currentDateTime()},
                        {"stripe_transfer_to_shipper_account_id", transfer_id}
                    };

                    sql_builder.updateDB(map_col_value, "orders", {"id_order", id_order});
                }

                // Set that shipper completed but not rated order (shipper_order_state)
                if(sql_builder.isExec())
                {
                    sql_builder.updateDB({"shipper_order_state", Shipper_Order_State::Has_Completed_Order_But_Not_Rated}, "user", {"id_user", user_data.user_Id()});
                }

                sql_builder.endTransaction(db);

                // Send Push To Client
                if(sql_builder.isExec())
                {
                    QString select = "SELECT id_user, push_token "
                                     "FROM push_token "
                                     "WHERE id_user = ?;";


                    PusherHandler::getHandler().sendPush(&PusherHandler::pushToClientShipperCompleteOrder, std::move(select), {id_client});
                }
            };

            auto transfer_money_to_shipper_account =
            [this, id_order, driver_account_id = std::move(driver_account_id),
            amount = map_select.value("order_price_without_tax").toDouble(), update_db_and_send_push = std::move(update_db_and_send_push)]
            (QString &&stripe_charge_id)
            {
                qDebug() << "--FUNC = transfer_money_to_shipper_account()--";

                is_foreign_api_using = true;

                auto response_func = [this, update_db_and_send_push = std::move(update_db_and_send_push), stripe_charge_id]
                (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
                {
                    qDebug() << "--RESPONSE FUNC = transfer_money_to_shipper_account()--";

                    error_response = std::move(api_error_response);

                    if(!error_response.isHaveLogError())
                    {
                        update_db_and_send_push(response_body.value("id").toString());
                    }
                    else
                    {
                        response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                        const QJsonObject log_error =
                        {
                            {"description", "Stripe API error: can't transfer money to shipper account using charge id"},
                            {"charge_id", stripe_charge_id}
                        };

                        error_response.insertLogError(log_error);
                    }


                    finishHandle();
                };


                StripeApi::Transfer transfer =
                {
                    amount,
                    driver_account_id,
                    "id_order = " + QString::number(id_order),
                    stripe_charge_id
                };

                ForeignApiHandler::getStripeApi()->transferMoneyToAnotherAccount(std::move(transfer), std::move(response_func));
            };


            auto capture_money = [this, id_order]
            (QString &&stripe_charge_id, decltype (transfer_money_to_shipper_account) &&transfer_money_to_shipper_account)
            {
                qDebug() << "--FUNC = capture_money()--";

                is_foreign_api_using = true;

                auto response_func = [this, transfer_money_to_shipper_account = std::move(transfer_money_to_shipper_account), stripe_charge_id, id_order]
                (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
                {
                    qDebug() << "--RESPONSE FUNC = capture_money()--";

                    error_response = std::move(api_error_response);

                    if(!error_response.isHaveLogError())
                    {
                        sql_builder.updateDB({"stripe_charge_status", Order_Charge_Status::Captured}, "orders", {"id_order", id_order});

                        if(sql_builder.isExec())
                        {
                            transfer_money_to_shipper_account(std::move(stripe_charge_id));
                        }
                        else
                        {
                            finishHandle();
                        }
                    }
                    else
                    {
                        response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                        const QJsonObject log_error =
                        {
                            {"description", "Stripe API error: can't capture charge"},
                            {"charge_id", stripe_charge_id}
                        };

                        error_response.insertLogError(log_error);

                        finishHandle();
                    }
                };


                ForeignApiHandler::getStripeApi()->captureCharge(std::move(stripe_charge_id), std::move(response_func));
            };

            // retreive_charge
            is_foreign_api_using = true;

            qDebug() << "--FUNC = retreive_charge()--";

            auto response_func = [this, stripe_charge_id = map_select.value("stripe_charge_id").toString(),
            transfer_money_to_shipper_account = std::move(transfer_money_to_shipper_account),
            capture_money = std::move(capture_money)]
            (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
            {
                qDebug() << "--RESPONSE FUNC = retreive_charge()--";

                error_response = std::move(api_error_response);

                if(!error_response.isHaveLogError())
                {
                    const bool is_captured = response_body.value("captured").toBool();

                    if(is_captured)
                    {
                        transfer_money_to_shipper_account(std::move(stripe_charge_id));
                    }
                    else
                    {
                        capture_money(std::move(stripe_charge_id), std::move(transfer_money_to_shipper_account));
                    }
                }
                else
                {
                    response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                    const QJsonObject log_error =
                    {
                        {"description", "Stripe API error: can't retreive charge"},
                        {"charge_id", stripe_charge_id}
                    };

                    error_response.insertLogError(log_error);

                    finishHandle();
                }
            };

            ForeignApiHandler::getStripeApi()->retreiveCharge(map_select.value("stripe_charge_id").toString(), std::move(response_func));
        }
    }
}


void Order::POST_Shipper_UpdateCoordinates()
{
    QVariantMap map_result;

    // Check received json
    {
        Map_required_json_check map_key_required =
        {
            {
                "new_longitude", Json::Validation_Type::Longitude
            },
            {
                "new_latitude", Json::Validation_Type::Latitude
            }
        };

        is_valid = json.validateJson(map_key_required, map_result, request_body.received_json, Json::Is_Required_Validation::Required);
    }


    // Update shipper coodrinates
    if(is_valid)
    {
        const QVariantMap map_col_value =
        {
            {"shipper_longitude", map_result.value("new_longitude")},
            {"shipper_latitude", map_result.value("new_latitude")}
        };

        sql_builder.updateDB(map_col_value, "user", {"id_user", user_data.user_Id()});
    }
}


void Order::POST_Shipper_ReadyToShip()
{
    const int ready_to_ship = checker.checkReceivedId("ready_to_ship", Checker::Received_Id_Type::Bool);

    if(is_valid)
    {
        sql_builder.updateDB({"ready_to_ship", ready_to_ship}, "user", {"id_user", user_data.user_Id()});
    }
}


void Order::GET_Shipper_GetPaid()
{
    QString card_token;

    // Check does driver have saved card
    {
        const QVariant select =
        sql_builder.selectFromDB("card_token", "payment_method", "id_user", user_data.user_Id(), is_valid);

        if(sql_builder.isExec())
        {
            if(is_valid)
            {
                card_token = select.toString();
            }
            else
            {
                error_response.insertError("to get paid, you need to save the card");
            }
        }
    }

    QString customer_account_id;


    // Get customer_account_id
    if(is_all_valid())
    {
        const QVariant select = sql_builder.selectFromDB("customer_account_id", "user", "id_user", user_data.user_Id(), is_valid);

        if(sql_builder.isExec())
        {
            if(is_valid)
            {
                customer_account_id = select.toString();
            }
            else
            {
                response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                const QJsonObject log_error =
                {
                    {"description", "Can't get customer_account_id"},
                    {"id_user", user_data.user_Id()}
                };

                error_response.insertLogError(log_error);
            }
        }
    }


    if(is_all_valid())
    {
        auto sendFundsToDriverCard = [this, customer_account_id, card_token = std::move(card_token)](const int amount_cents)
        {
            qDebug() << "--FUNC = sendFundsToDriverCard()--";

            is_foreign_api_using = true;

            auto response_func = [this]
            (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
            {
                qDebug() << "--RESPONSE FUNC = sendFundsToDriverCard()--";

                error_response = std::move(api_error_response);

                if(error_response.isHaveLogError())
                {
                    response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                    const QJsonObject log_error =
                    {
                        {"description", "Stripe API error: can't send funds to driver card"}
                    };

                    error_response.insertLogError(log_error);
                }

                finishHandle();
            };

            StripeApi::Payout payout =
            {
                amount_cents,
                card_token,
                customer_account_id
            };

            ForeignApiHandler::getStripeApi()->sendFundsToDebitCard(std::move(payout), std::move(response_func));

        };


        auto getDriverBalance = [this, customer_account_id = std::move(customer_account_id)]
        (decltype (sendFundsToDriverCard) &&sendFundsToDriverCard) mutable
        {
            qDebug() << "--FUNC = getDriverBalance()--";

            is_foreign_api_using = true;

            auto response_func = [this, sendFundsToDriverCard = std::move(sendFundsToDriverCard)]
            (QJsonObject &&response_body, ErrorResponse &&api_error_response) mutable
            {
                qDebug() << "--RESPONSE FUNC = getDriverBalance()--";

                error_response = std::move(api_error_response);

                if(!error_response.isHaveLogError())
                {
                    int amount_cents{};

                    const QJsonArray available_array = response_body.value("available").toArray();

                    for(const auto &value : available_array)
                    {
                        amount_cents += value.toObject().value("amount").toInt();
                    }

                    qDebug() << "Driver account balance = " << amount_cents;

                    if(amount_cents > 0)
                    {
                        sendFundsToDriverCard(amount_cents);
                    }
                    else
                    {
                        finishHandle();
                    }
                }
                else
                {
                    response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                    const QJsonObject log_error =
                    {
                        {"description", "Stripe API error: can't get driver balance"}
                    };

                    error_response.insertLogError(log_error);

                    finishHandle();
                }
            };

            ForeignApiHandler::getStripeApi()->getBalanceFromAccount(std::move(customer_account_id), std::move(response_func));
        };

        getDriverBalance(std::move(sendFundsToDriverCard));
    }


}


Order::Order_Price Order::checkItemsAndCalcOrderPrice(const QJsonArray &array_item_list)
{
    Order_Price order_price;

    // Check item_list

    const QString item_in = checker.getListId_Parse_(array_item_list, "id_item", "item_list", is_valid, false);

    if(is_valid)
    {
        Checker::Check_Array<> check_array = {array_item_list, "item_list", "id_item", Checker::Check_Is_Deleted::No};

        const QString select = "SELECT id_item "
                               "FROM item "
                               "WHERE " + item_in;

        check_array.select = select;

        checker.checkArrayOfId(check_array, std::nullopt, "item");
    }


    if(is_all_valid())
    {
        // id_item - count
        QMap<int, int> map_item_count;

        for(const auto &item : array_item_list)
        {
            const int id_item = item.toInt();

            auto it = map_item_count.find(id_item);

            if(it == map_item_count.end())
            {
                map_item_count.insert(id_item, 1);
            }
            else
            {
                it.value() += 1;
            }
        }


        {
            const QString select = "SELECT id_item, size "
                                   "FROM item "
                                   "WHERE " + item_in;

            sql_builder.execQuery(select);
        }

        if(sql_builder.isExec())
        {
            int small_item_count = 0, large_item_count = 0;

            while(sql_builder.isNext())
            {
                const int id_item = sql_builder.getValue("id_item").toInt();

                const bool size = sql_builder.getValue("size").toBool();

                auto it = map_item_count.constFind(id_item);

                if(it != map_item_count.constEnd())
                {
                    if(size)
                    {
                        large_item_count = large_item_count + it.value();
                    }
                    else
                    {
                        small_item_count = small_item_count + it.value();
                    }
                }
            }

            if(small_item_count > 0)
                order_price.order_price_without_tax += 20.0;

            ///////////////////////////////////////
            // 40 - 1 // 40 = 40.0 -> // 50 = 50.0
            // 80 - 2 // 50 = 40.0 -> // 70 = 50.0
            // 120 - 3 //90 = 40.0 -> // 90 = 50.0
            ///////////////////////////////////////
            if(large_item_count >= 1) {
                if (large_item_count > 1) {
                    order_price.order_price_without_tax +=
                        ((large_item_count * Global::getOrderSettings().large_item_price) - Global::getOrderSettings().items_discount);
                }
                else
                    order_price.order_price_without_tax = Global::getOrderSettings().large_item_price;
            }

            order_price.order_price_without_tax = Global::roundDoubleToTwoPointsDecimal(order_price.order_price_without_tax);

            order_price.tax = getTaxFromOrderPrice(order_price.order_price_without_tax);
        }
    }

    return order_price;
}


double Order::getTaxFromOrderPrice(const double order_price_without_tax)
{
    qDebug() << "order_price_without_tax = " << order_price_without_tax;

    const double app_tax = (order_price_without_tax * Global::getOrderSettings().application_tax);

    qDebug() << "App, tax = " << app_tax;

    return Global::roundDoubleToTwoPointsDecimal(app_tax);
}


void Order::calcOrderPriceDistance(Order::Order_Price &order_price, const double distance_in_miles)
{
    qDebug() << "distance_in_miles = " << distance_in_miles
             << "order_price.order_price_without_tax = " << order_price.order_price_without_tax
             << "order_price.tax = " << order_price.tax;

    if(distance_in_miles > Global::getOrderSettings().max_miles_from_pick_up_to_drop_off_without_tax)
    {
        const int number_of_miles = static_cast<int>(std::round(distance_in_miles - Global::getOrderSettings().max_miles_from_pick_up_to_drop_off_without_tax));

        if(number_of_miles > 0)
        {
            order_price.order_price_without_tax = order_price.order_price_without_tax + (Global::getOrderSettings().dollars_for_one_mile * number_of_miles);

            order_price.order_price_without_tax = Global::roundDoubleToTwoPointsDecimal(order_price.order_price_without_tax);

            order_price.tax = getTaxFromOrderPrice(order_price.order_price_without_tax);

            qDebug() << "number_of_miles = " << number_of_miles
                     << "order_price.order_price_without_tax = " << order_price.order_price_without_tax
                     << "order_price.tax = " << order_price.tax;
        }
    }
}



std::tuple<bool, double, int>
Order::checkDistanceMatrixResponseAndGetDistanceAndTime(const QVariantMap &map_result, const QJsonObject &api_response_body)
{
    bool finish_handle = false;
    double distance_miles{};
    int time_to_deliver_in_minutes{};

    // Check top status code
    {
        const QString top_status_code = api_response_body.value("status").toString();

        is_valid = (top_status_code == QLatin1String("OK"));

        if(!is_valid)
        {
            response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

            const QJsonObject log_error =
            {
                {"description", "Distance Matrix Google API error: can't get distance"},
                {"api_response_body", api_response_body}
            };

            error_response.insertLogError(log_error);

            finish_handle = true;
        }
    }


    // Get distance and time_to_deliver_in_minutes
    if(is_valid)
    {
        const QJsonObject object = api_response_body.value("rows").toArray()
                                   .first().toObject().value("elements").toArray()
                                   .first().toObject();

        const QString element_status_code = object.value("status").toString();

        is_valid = (element_status_code == QLatin1String("OK"));

        if(!is_valid)
        {
            const QJsonObject log_error =
            {
                {"description", "Distance Matrix Google API error: can't get distance - " + element_status_code},
                {"distance_matrix_google_response_element_status_code", element_status_code},
                {"pick_up_latitude", map_result.value("pick_up_latitude").toDouble()},
                {"pick_up_longitude", map_result.value("pick_up_longitude").toDouble()},
                {"drop_off_latitude", map_result.value("drop_off_latitude").toDouble()},
                {"drop_off_longitude", map_result.value("drop_off_longitude").toDouble()}
            };

            error_response.insertError("invalid coordinates", log_error);

            finish_handle = true;
        }
        else
        {
            const double distance_meters = object.value("distance").toObject().value("value").toDouble();

            if(!qFuzzyIsNull(distance_meters))
            {
                distance_miles = Global::roundDoubleToTwoPointsDecimal(distance_meters / 1609.344);
            }

            auto it = object.constFind("duration_in_traffic");

            if(it != object.constEnd())
            {
                time_to_deliver_in_minutes = it.value().toObject().value("value").toInt();
            }
            else
            {
                time_to_deliver_in_minutes = object.value("duration").toObject().value("value").toInt();
            }

            if(time_to_deliver_in_minutes > 0)
            {
                time_to_deliver_in_minutes = (time_to_deliver_in_minutes / 60);
            }
        }
    }

    return {finish_handle, distance_miles, time_to_deliver_in_minutes};
}


QTimer* Order::createTimerForOrderAndInsertOrderToMap(const int id_order, const QString connection_db_name, Order_For_Searching &&order_for_searching)
{
    qDebug() << "\r\n---------------" << __PRETTY_FUNCTION__ << "---------------\r\n";

    QTimer *timer = new QTimer;

    QObject::connect(timer, &QTimer::timeout, [id_order, connection_db_name]()
    {
        findShipperForOrder(id_order, connection_db_name);
    });

    Order_For_Searching temp_order_for_searching = std::move(order_for_searching);

    temp_order_for_searching.timer = timer;

    static_map_order.insert(id_order, temp_order_for_searching);

    return temp_order_for_searching.timer;
}


void Order::deleteTimerAndRemoveOrderFromMap(const int id_order)
{
    qDebug() << "\r\n---------------" << __PRETTY_FUNCTION__ << "---------------\r\n";

    auto it = static_map_order.find(id_order);

    if(it != static_map_order.end())
    {
        qDebug() << "Exist";

        Order_For_Searching order_for_searching = std::move(it.value());

        order_for_searching.timer->stop();
        order_for_searching.timer->deleteLater();

        static_map_order.remove(id_order);
    }
}


void Order::findShipperForOrder(const int id_order, const QString connection_db_name)
{
    qDebug() << "\r\n---------------" << __PRETTY_FUNCTION__ << "---------------\r\n";

    qDebug() << "---\\ Id Order = " << id_order << "//---\r\n";

    auto it = static_map_order.find(id_order);

    if(it != static_map_order.end())
    {
        qDebug() << "Exist";

        Order_For_Searching &order_for_searching = it.value();

        order_for_searching.timer->stop();


        SqlBuilder sql_builder(QSqlDatabase::database(connection_db_name), connection_db_name);

        // Clear state of last shipper
        if(order_for_searching.id_shipper_old > 0)
        {
            sql_builder.updateDB({"shipper_order_state", Shipper_Order_State::Does_Not_Has_Order}, "user", {"id_user", order_for_searching.id_shipper_old});

            if(sql_builder.isExec())
            {
                order_for_searching.id_shipper_old = 0;
            }
        }


        if(sql_builder.isExec())
        {
            QSqlDatabase db = sql_builder.beginTransaction();


            // Select the nearest shipper within specified miles
            if(sql_builder.isExec())
            {
                // 3959 - Miles, 6371 - Kilometers
                const QString distanse_string =
                        "( 3959 * acos(cos (radians(?) ) * cos(radians(shipper_latitude)) * cos( radians(shipper_longitude) - radians(?) ) + sin( radians(?) ) * sin( radians(shipper_latitude) ) ) )";

                const QString select = "SELECT id_user, "
                                       + distanse_string + " AS distance "
                                       "FROM user "
                                       "WHERE is_driver = 1 AND ready_to_ship = 1 "
                                       "AND shipper_order_state = ? "
                                       "AND id_user NOT IN(SELECT id_shipper FROM shipper_getted_order WHERE id_order = ?) "
                                       "ORDER BY distance ASC "
                                       "LIMIT 1;";

                const QVector<QVariant> vec_value =
                {
                    order_for_searching.pick_up_latitude, order_for_searching.pick_up_longitude, order_for_searching.pick_up_latitude,
                    Shipper_Order_State::Does_Not_Has_Order, id_order
                };

                sql_builder.execQuery(select, vec_value);
            }

            QVariant id_shipper;

            if(sql_builder.isExec())
            {
                if(sql_builder.isNext())
                {
                    id_shipper = sql_builder.getValue(0).toInt();
                }
            }

            qDebug() << "\r\nAdd shipper to order: " << "id_shipper = " << id_shipper;

            // Add shipper to order
            if(sql_builder.isExec())
            {
                sql_builder.updateDB({"id_shipper", id_shipper}, "orders", {"id_order", id_order});
            }


            if(!id_shipper.isNull())
            {
                order_for_searching.id_shipper_old = id_shipper.toInt();

                qDebug() << "\r\nSet that shipper get order: " << "id_shipper = " << id_shipper;

                // Set that shipper get order
                if(sql_builder.isExec())
                {
                    sql_builder.updateDB({"shipper_order_state", Shipper_Order_State::Getted_Order}, "user", {"id_user", id_shipper});
                }

                qDebug() << "\r\nAdd shipper to shipper_getted_order table: " << "id_shipper = " << id_shipper;

                // Add shipper to shipper_getted_order table
                if(sql_builder.isExec())
                {
                    const QVariantMap map_col_value =
                    {
                        {"id_order", id_order},
                        {"id_shipper", id_shipper}
                    };

                    sql_builder.insertIntoDB(map_col_value, "shipper_getted_order");
                }

                // Send Push To Shipper
                if(sql_builder.isExec())
                {
                    QString select = "SELECT id_user, push_token "
                                     "FROM push_token "
                                     "WHERE id_user = ?;";


                    PusherHandler::getHandler().sendPush(&PusherHandler::pushToShipperNewOrder, std::move(select), {id_shipper});
                }
            }


            sql_builder.endTransaction(db);
        }

        if(!sql_builder.isExec())
        {
            const QJsonObject error =
            {
                {"id_order", id_order},
                {"description", "Can't give order to shippers"},
                {"prepared_query", sql_builder.lastQuery()},
                {"query_error", sql_builder.lastError()},
                {"bound_values", QJsonObject::fromVariantMap(sql_builder.boundValues())}
            };

            Logger::writeLogInFile(error, __PRETTY_FUNCTION__);
        }


        order_for_searching.timer->start(Global::getOrderSettings().start_searching_for_other_shippers_every_specified_ms);
    }
}

QString Order::formatAddress(const QString &address)
{
    // MD-704, Springdale, MD 20774, USA
    // 1600 Amphitheatre Parkway, Mountain View, CA 94043, USA
    // 1502 Amphitheatre Pkwy, Mountain View, CA 94043, USA

    // Amphitheatre Pkwy, 1502, Mountain View
    // route + street_number + locality

    const QString street_number_and_route = address.section(',', 0, 0);

    QList<QString> list = street_number_and_route.split(" ");

    bool have_street_number = false;

    list.first().toInt(&have_street_number);

    QString street_number;

    if(have_street_number)
    {
        street_number = list.takeFirst();
    }


    QString route;

    for(const auto &value : list)
    {
        route.append(value).append(" ");
    }

    route.chop(1);

    const QString locality = address.section(',', 1, 1).simplified();

    if(have_street_number)
    {
        return route + ", " + street_number + ", " + locality;
    }
    else
    {
        return route + ", " + locality;
    }
}



template<class Function>
void Order::getDistanceAnd(DistanceMatrixGoogle::Coordinates &&coordinates, Function &&function)
{
    qDebug() << "----FUNC getDistanceAndInsertValues----";

    is_foreign_api_using = true;


    auto response_func = [this, coordinates, function = std::move(function)]
    (QJsonObject &&api_response_body, ErrorResponse &&api_error_response) mutable
    {
        qDebug() << "----FUNC Response getDistanceAndInsertValues----";

        error_response = std::move(api_error_response);

        Is_Finish_Handle finish_handle = Is_Finish_Handle::No;

        if(error_response.isHaveLogError())
        {
            error_response.insertError("invalid coordinates");

            finish_handle = Is_Finish_Handle::Yes;
        }
        else
        {
            double distance_miles{};
            int time_to_deliver_in_minutes{};

            // Check top status code
            {
                const QString top_status_code = api_response_body.value("status").toString();

                is_valid = (top_status_code == QLatin1String("OK"));

                if(!is_valid)
                {
                    response.setStatusCode(Response::HTTP_Status_Code::Internal_Server_Error);

                    const QJsonObject log_error =
                    {
                        {"description", "Distance Matrix Google API error: can't get distance"},
                        {"api_response_body", api_response_body}
                    };

                    error_response.insertLogError(log_error);

                    finish_handle = Is_Finish_Handle::Yes;
                }
            }


            // Get distance and time_to_deliver_in_minutes
            if(is_valid)
            {
                const QJsonObject object = api_response_body.value("rows").toArray()
                                           .first().toObject().value("elements").toArray()
                                           .first().toObject();

                const QString element_status_code = object.value("status").toString();

                is_valid = (element_status_code == QLatin1String("OK"));

                if(!is_valid)
                {
                    const QJsonObject log_error =
                    {
                        {"description", "Distance Matrix Google API error: can't get distance - " + element_status_code},
                        {"distance_matrix_google_response_element_status_code", element_status_code},
                        {"from_latitude", coordinates.from_latitude},
                        {"from_longitude", coordinates.from_longitude},
                        {"to_latitude", coordinates.to_latitude},
                        {"to_longitude", coordinates.to_longitude}
                    };

                    error_response.insertError("invalid coordinates", log_error);

                    finish_handle = Is_Finish_Handle::Yes;
                }
                else
                {
                    const double distance_meters = object.value("distance").toObject().value("value").toDouble();

                    if(!qFuzzyIsNull(distance_meters))
                    {
                        distance_miles = Global::roundDoubleToTwoPointsDecimal(distance_meters / 1609.344);
                    }

                    auto it = object.constFind("duration_in_traffic");

                    if(it != object.constEnd())
                    {
                        time_to_deliver_in_minutes = it.value().toObject().value("value").toInt();
                    }
                    else
                    {
                        time_to_deliver_in_minutes = object.value("duration").toObject().value("value").toInt();
                    }

                    if(time_to_deliver_in_minutes > 0)
                    {
                        time_to_deliver_in_minutes = (time_to_deliver_in_minutes / 60);
                    }
                }
            }


            if(is_valid)
            {
                Distance distance =
                {
                    api_response_body.value("origin_addresses").toArray().first().toString(),
                    api_response_body.value("destination_addresses").toArray().first().toString(),
                    distance_miles,
                    time_to_deliver_in_minutes
                };

                finish_handle = function(std::move(distance));
            }
        }

        if(finish_handle == Is_Finish_Handle::Yes)
        {
            finishHandle();
        }
    };

    ForeignApiHandler::getDistanceMatrixGoogle()->getDistanceAndDuration(std::move(coordinates), std::move(response_func));
}






