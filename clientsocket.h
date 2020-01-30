#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H


#include <QDateTime>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QTimer>
#include <QHostAddress>

#include "SqlBuilder/sqlbuilder.h"
#include "Global/logger.h"

#include "RequestHandler/User/user.h"
#include "RequestHandler/Order/order.h"


#include "validator.h"
#include "url_list.h"

#include <Response/response.h>



class ClientSocket : public QObject
{
    Q_OBJECT
public:
    explicit ClientSocket(SqlBuilder *const _sql_builder_, QObject *parent);

    ~ClientSocket();

    Q_INVOKABLE
    void process(const qintptr socket_descriptor);


private:

    QTcpSocket *socket;

    SqlBuilder *const sql_builder;

    UserData user_data;


    // Socket connection duration -> 1 min, if recieve data then timer restarts
    QTimer connection_duration;


    static const int max_headers_size;


    // To check and validate once headers
    bool is_check_headers = false;


    enum class State : char
    {
        ReadStartLine,
        ReadHeaders,
        ReadBody
    } state = State::ReadStartLine;

    enum class Handle_State : unsigned char
    {
        Ready,
        Handling
    } handle_state = Handle_State::Ready;


    enum class Client_Socket_State : unsigned char
    {
        Working,
        Closed
    } client_socket_state = Client_Socket_State::Working;



    Response response;


    Method method = Method::Unknown;



    enum class Supported_media_types : char
    {
        Unknown,
        App_FormUrlEncoded,
        Multipart_FormData,
        JSON
    };


    static const QHash<QString, Supported_media_types> available_media_types;

    void handleRequest();

    [[nodiscard]]
    ErrorResponse parseRequest(RequestBody &request_body);

    [[nodiscard]]
    AbstractRequestHandler* getHandler();


    void sendResponse();


    void reset();


    void writeLogs(const ErrorResponse &error_response);



    struct Url_Settings
    {
        unsigned short url_index = 0;
        Url_Type url_type = Url_Type::Unknown;
        Method url_method = Method::Post;
    };

    static const QHash<QString, Url_Settings> available_auth_urls;

    static const QHash<QString, Url_Settings> available_no_auth_urls;



    enum class Required_headers : char
    {
        Unknown,
        Authorization,
        Content_Length,
        Content_Type
    };

    static const QHash<QString, Required_headers> required_headers;


    enum class Request_Errors : char
    {
        Validation_Header_Error,
        Not_Valid_Header
    };

    struct Request_headers
    {
        QMap<Required_headers, QString> map_required;
        QMap<Request_Errors, QString> map_errors;
        QVector<QString> vec_received;

        QString boundary;
        QString path;
        Url_Settings url_settings;
        int headers_size = 0;


        void reset()
        {
            map_required.clear();
            map_errors.clear();
            vec_received.clear();

            boundary.clear();
            path.clear();
            url_settings = {0, Url_Type::Unknown, Method::Unknown};
            headers_size = 0;
        }
    } request_headers;


    // Recieved data
    QByteArray request_data;



    bool checkStartLine(const QString &start_line);


    bool parseHeaders();


    bool validateHeaders();


    bool checkAuth();



private slots:

    void onReadyRead();

    void onDisconnect();
};


#endif // CLIENTSOCKET_H
