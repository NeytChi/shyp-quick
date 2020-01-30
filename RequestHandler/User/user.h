#ifndef USER_H
#define USER_H


#include "RequestHandler/abstractrequesthandler.h"

#include "EmailSender/emailsender.h"


#include <QCryptographicHash>
#include <QRandomGenerator>
#include <qpassworddigestor.h>

#include <functional>


class User : public AbstractRequestHandler
{
public:
    User(SqlBuilder &_sql_builder_, Response &_response_, const Send_Response_Function _send_response_function_);

    ~User();


    static const QHash<unsigned char, std::function<void(User*)>> static_handle_request_map;


    virtual void handle();


    void GET_Menu();

    void GET_Profile();

    void GET_Logout();

    void GET_Profile_Picture();



    void POST_Login();

    void POST_OAuthLogin();

    void POST_CheckSignUp();

    void POST_SignUp();

    void POST_ActivationSignUp();

    void POST_RecoveryPass();

    void POST_SendNewPass();

    void POST_ResendSmsCode();

    void POST_ChangeProfilePicture();

    void POST_ChangeProfile();

    void POST_ChangePassword();


    void POST_Contact_Us();

    void POST_AddPushToken();

    void GET_ZipCodeState();

    void GET_CarModelList();

    void GET_PaymentMethod();

    void POST_SavePaymentMethod();

    void GET_CheckAuthToken();



    //---Other---//
    QByteArray getHashToken();

    QByteArray getHashPassword(const QByteArray &password, QByteArray &salt);

    bool checkHashPassword(const QByteArray &password, const QByteArray &salt, const QByteArray hash_password);

    void addAuthToken(QJsonObject &response_body, const int id_user, const bool is_driver);


    QString generateUniqueSmsCode();


    enum class Is_Finish_Handle : bool
    {
        No = false,
        Yes = true,
    };

    struct Address
    {
        QString address;
        double latitude;
        double longitude;
    };


    template<class Function>
    void getLatLongFromAddress(QString &&address, Function &&function);


    QJsonObject getZipCodeByStateJson();

    QJsonObject getCarModelListJson();

    void checkZipCodeAndState(const QString &zip_code, const QString &state);

};

#endif // USER_H
