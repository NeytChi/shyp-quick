#ifndef ORDER_H
#define ORDER_H


#include "RequestHandler/abstractrequesthandler.h"

#include "PushNotifications/pusherhandler.h"

#include "ForeignApi/Stripe/stripeapi.h"

#include "ForeignApi/DistanceMatrixGoogle/distancematrixgoogle.h"

#include <QTimer>



class Order : public AbstractRequestHandler
{
public:
    Order(SqlBuilder &_sql_builder_, Response &_response_, const Send_Response_Function _send_response_function_);

    ~Order();


    static const QHash<unsigned char, std::function<void(Order*)>> static_handle_request_map;


    virtual void handle();



    void GET_OrderList();

    void GET_OrderById();

    void POST_RateUser();

    void GET_PastOrders();



    // Client
    void POST_Client_CreateOrder();

    void POST_Client_CalcOrder();

    void POST_Client_CancelOrder();

    void GET_Client_OrderState();

    void GET_Client_LastCompletedOrders();

    // Shipper
    void GET_Shipper_Stats();

    void GET_Shipper_CheckExistNewOrder();

    void POST_Shipper_AcceptOrder();

    void POST_Shipper_DeclineOrder();

    void POST_Shipper_OnPickUpPoint();

    void POST_Shipper_OnDropOffPoint();

    void POST_Shipper_UpdateCoordinates();

    void POST_Shipper_ReadyToShip();

    void GET_Shipper_GetPaid();



    //---Other---//


    void checkDoesOrderExist(const int id_order)
    {
        if(!is_valid)
        {
            error_response.insertError("id_order doesn't exist", "id_order", id_order);
        }
    }


    void checkIsOrderOpen(const int id_order, const bool is_open)
    {
        is_valid = is_open;

        if(!is_valid)
        {
            error_response.insertError("order already closed", "id_order", id_order);
        }
    }


    struct Order_Price
    {
        double order_price_without_tax = 0.0;
        double tax = 0.0;
    };


    [[nodiscard]]
    Order_Price checkItemsAndCalcOrderPrice(const QJsonArray &array_item_list);

    [[nodiscard]]
    double getTaxFromOrderPrice(const double order_price_without_tax);

    void calcOrderPriceDistance(Order_Price &order_price, const double distance_in_miles);

    [[nodiscard]]
    // finish_handle, distance_miles, time_to_deliver_in_minutes
    std::tuple<bool, double, int>
    checkDistanceMatrixResponseAndGetDistanceAndTime(const QVariantMap &map_result, const QJsonObject &api_response_body);


    enum class Is_Finish_Handle : bool
    {
        No = false,
        Yes = true,
    };

    struct Distance
    {
        QString origin_address;
        QString destination_address;
        double distance_miles;
        int time_to_deliver_in_minutes;
    };

    template<class Function>
    void getDistanceAnd(DistanceMatrixGoogle::Coordinates &&coordinates, Function &&function);



    struct Order_For_Searching
    {
        int id_shipper_old = 0;
        double pick_up_longitude;
        double pick_up_latitude;
        QTimer *timer = nullptr;
    };


    // id_order
    static QMap<int, Order_For_Searching> static_map_order;

    static QTimer* createTimerForOrderAndInsertOrderToMap(const int id_order, const QString connection_db_name, Order_For_Searching &&order_for_searching);

    static void deleteTimerAndRemoveOrderFromMap(const int id_order);

    static void findShipperForOrder(const int id_order, const QString connection_db_name);

    QString formatAddress(const QString &address);

};

#endif // ORDER_H
