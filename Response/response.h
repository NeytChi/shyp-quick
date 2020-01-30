#ifndef RESPONSE_H
#define RESPONSE_H

#include <QJsonObject>

#include "Response/ErrorResponse/errorresponse.h"

#include <QTcpSocket>


class Response
{
public:

    Response();

    enum HTTP_Status_Code
    {
        Unknown_Error = 520,
        OK = 200,
        No_Content = 204,
        See_Other = 303,
        Bad_Request = 400,
        Unauthorized = 401,
        Forbidden = 403,
        Not_Found = 404,
        Method_Not_Allowed = 405,
        Length_Required = 411,
        Payload_Too_Large = 413,
        URI_Too_Long = 414,
        Unsupported_Media_Type = 415,
        Internal_Server_Error = 500,
        Not_Implemented = 501,
        HTTP_Version_Not_Supported = 505
    };

    enum class Response_Type : unsigned char
    {
        Unknow,
        Json,
        File
    };

    enum class Header : unsigned char
    {
        Content_Length,
        Content_Type,
        Content_Disposition,
        Access_Control_Allow_Origin,
        Access_Control_Allow_Headers,
        Access_Control_Allow_Methods,
        Keep_Alive,
        Allow,
        WWW_Authenticate,
        Connection
    };

    // Supported image extensions
    enum class Supported_Image_Ext : unsigned char
    {
        Unknow,
        JPG,
        PNG,
        SVG
    };

    static const QHash<QString, Supported_Image_Ext> static_map_available_image_ext;

private:

    QByteArray headers;

    QByteArray data;

    ErrorResponse error_response;


    HTTP_Status_Code response_status_code = HTTP_Status_Code::Unknown_Error;

    static const QHash<int, QString> status_code;

    Response_Type response_type = Response_Type::Unknow;

    static const QMap<Header, QByteArray> static_map_headers;

public:

    void setStatusCode(const HTTP_Status_Code status_code)
    {
        response_status_code = status_code;
    }

    HTTP_Status_Code statusCode() const
    {
        return response_status_code;
    }

    QString getStatusCodeValue() const
    {
        return status_code.value(response_status_code);
    }

    Response_Type getResponseType() const
    {
        return response_type;
    }

    const ErrorResponse& getErrorResponse() const
    {
        return error_response;
    }

    QByteArray getHeaders()
    {
        return std::move(headers);
    }

    void addHeader(const Header header, const QByteArray &value)
    {
        headers.append(static_map_headers.value(header)).append(": ").append(value).append("\r\n");
    }


    QByteArray getData()
    {
        return std::move(data);
    }


    void setData(ErrorResponse &&error_response);

    enum class Content_Disposition : unsigned char
    {
        Inline,
        Attachment
    };

    void setData(QByteArray &&file_data, const Supported_Image_Ext file_type, const Content_Disposition content_disposition);

    void setData(QJsonObject &&response_json);


    void sendHeaders(QTcpSocket *const socket);

    void sendData(QTcpSocket *const socket);


    void clear()
    {
        headers.clear();

        data.clear();

        error_response.clear();

        response_status_code = HTTP_Status_Code::Unknown_Error;

        response_type = Response_Type::Unknow;
    }

};

#endif // RESPONSE_H
