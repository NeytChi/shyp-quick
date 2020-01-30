#include "userdata.h"

UserData::UserData()
{

}

UserData::~UserData()
{

}

inline UserData::UserData(UserData &&other) noexcept
    : user_id(other.user_id),
      auth_token(std::move(other.auth_token)),
      is_driver(other.is_driver)
{
    other.user_id = 0;
}


UserData &UserData::operator= (UserData &&other) noexcept
{
    if(this != &other)
    {
        user_id = other.user_id;

        auth_token = std::move(other.auth_token);

        is_driver = other.is_driver;

        other.user_id = 0;
    }

    return *this;
}



// is_auth, is_have_access, auth_error
std::tuple<bool, bool, QString>
UserData::checkAuth(const QString &auth_token, SqlBuilder &sql_builder, const unsigned short url_index)
{
    this->auth_token = auth_token;

    QString auth_error;

    bool is_auth = !auth_token.isEmpty();

    bool is_have_access = true;

    if(is_auth)
    {
        // Check does exist user with specified token
        {
            const QString select = sql_builder.getSelectString({"id_user", "is_driver"}, "auth_token", "auth_token");

            sql_builder.execQuery(select, auth_token);
        }

        if(sql_builder.isExec())
        {
            is_auth = sql_builder.isNext();

            if(is_auth)
            {
                user_id = sql_builder.getValue(0).toInt();

                is_driver = sql_builder.getValue(1).toBool();

                is_have_access = validateAccess(url_index);

            }
        }
    }


    return {is_auth, is_have_access, auth_error};
}


bool UserData::validateAccess(const unsigned short url_index)
{
    bool is_have_access = false;

    if(is_driver)
    {
        const QSet<unsigned short> set_url_forbidden_for_driver =
        {
            // User
            Url_Get::User_Client_Menu,

            // Order
            Url_Post::Order_Client_Create_Order,
            Url_Post::Order_Client_Cancel_Order,

            Url_Get::Order_Client_Get_Order_State,
            Url_Get::Order_Client_Last_Completed_Orders
        };

        is_have_access = !set_url_forbidden_for_driver.contains(url_index);
    }
    else
    {
        const QSet<unsigned short> set_url_forbidden_for_client =
        {
            // User
            Url_Get::User_Driver_Menu,

            // Order
            Url_Post::Order_Shipper_Accept_Order,
            Url_Post::Order_Shipper_Decline_Order,
            Url_Post::Order_Shipper_On_Pick_Up_Point,
            Url_Post::Order_Shipper_On_Drop_Off_Point,
            Url_Post::Order_Shipper_Update_Coordinates,
            Url_Post::Order_Shipper_Ready_To_Ship,

            Url_Get::Order_Shipper_Check_Exist_New_Order,
            Url_Get::Order_Shipper_Get_Stats,
            Url_Get::Order_Shipper_Get_Paid
        };

        is_have_access = !set_url_forbidden_for_client.contains(url_index);
    }

    return is_have_access;
}


