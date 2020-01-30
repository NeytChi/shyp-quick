#ifndef URL_LIST_H
#define URL_LIST_H

#include <QMap>
#include <QString>

enum class Method : unsigned char
{
    Unknown = 1,
    Get,
    Post
};


// NA - no auth
struct Url_Post
{
    enum : unsigned short
    {
        First_Post = 1,

        //User

        User_Login_NA,
        User_OAuth_Login_NA,

        User_Check_Sign_Up_NA,
        User_Sign_Up_Client_NA,
        User_Sign_Up_Driver_NA,
        User_Activation_Sign_Up_NA,

        User_Recovery_Pass_NA,
        User_Send_New_Pass_NA,

        User_Resend_Sms_Code_NA,

        User_Change_Profile_Picture,
        User_Change_Profile,
        User_Change_Password,

        User_Contact_Us,
        User_Add_Push_Token,
        User_Save_Payment_Method,
        //---------------------

        //Order

        Order_Client_Create_Order,
        Order_Client_Calc_Order,
        Order_Client_Cancel_Order,

        Order_Shipper_Accept_Order,
        Order_Shipper_Decline_Order,
        Order_Shipper_On_Pick_Up_Point,
        Order_Shipper_On_Drop_Off_Point,
        Order_Shipper_Update_Coordinates,
        Order_Shipper_Ready_To_Ship,

        Order_Rate_User,
        //---------------------


        Last_Post
    };
};


struct Url_Get
{
    enum : unsigned short
    {
        Firs_Get = Url_Post::Last_Post + 1,

        //User

        User_Client_Menu,
        User_Driver_Menu,
        User_Profile_Picture_NA,
        User_Profile,
        User_Logout,

        User_Get_Zip_Code_State_NA,
        User_Get_Car_Model_List_NA,
        User_Get_Payment_Method,
        User_Check_Auth_Token,
        //---------------------

        //Order

        Order_Client_Get_Order_State,
        Order_Client_Last_Completed_Orders,

        Order_Shipper_Check_Exist_New_Order,
        Order_Shipper_Get_Stats,
        Order_Shipper_Get_Paid,

        Order_Get_Order_List,
        Order_Get_Order_By_Id,
        //---------------------



        Last_Get
    };
};


enum class Url_Type : char
{
    Unknown,

    User,
    Order
};


#endif // URL_LIST_H
