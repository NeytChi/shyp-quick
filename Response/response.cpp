#include "response.h"

Response::Response()
{

}


const QHash<int, QString> Response::status_code =
{
    {
        HTTP_Status_Code::OK, QStringLiteral("OK")
    },
    {
        HTTP_Status_Code::Bad_Request, QStringLiteral("Bad Request")
    },
    {
        HTTP_Status_Code::Unauthorized, QStringLiteral("Unauthorized")
    },
    {
        HTTP_Status_Code::Forbidden, QStringLiteral("Forbidden")
    },
    {
        HTTP_Status_Code::No_Content, QStringLiteral("No Content")
    },
    {
        HTTP_Status_Code::See_Other, QStringLiteral("See Other")
    },
    {
        HTTP_Status_Code::Not_Found, QStringLiteral("Not Found")
    },
    {
        HTTP_Status_Code::Method_Not_Allowed, QStringLiteral("Method Not Allowed")
    },
    {
        HTTP_Status_Code::Length_Required, QStringLiteral("Length Required")
    },
    {
        HTTP_Status_Code::Payload_Too_Large, QStringLiteral("Payload Too Large")
    },
    {
        HTTP_Status_Code::URI_Too_Long, QStringLiteral("URI Too Long")
    },
    {
        HTTP_Status_Code::Unsupported_Media_Type, QStringLiteral("Unsupported Media Type")
    },
    {
        HTTP_Status_Code::Internal_Server_Error, QStringLiteral("Internal Server Error")
    },
    {
        HTTP_Status_Code::Not_Implemented, QStringLiteral("Not Implemented")
    },
    {
        HTTP_Status_Code::HTTP_Version_Not_Supported, QStringLiteral("HTTP Version Not Supported")
    },
    {
        HTTP_Status_Code::Unknown_Error, QStringLiteral("Unknown Error")
    }
};


const QMap<Response::Header, QByteArray> Response::static_map_headers =
{
    {Header::Content_Length, "Content-Length"},
    {Header::Content_Type, "Content-Type"},
    {Header::Content_Disposition, "Content-Disposition"},
    {Header::Access_Control_Allow_Origin, "Access-Control-Allow-Origin"},
    {Header::Access_Control_Allow_Headers, "Access-Control-Allow-Headers"},
    {Header::Access_Control_Allow_Methods, "Access-Control-Allow-Methods"},
    {Header::Keep_Alive, "Keep-Alive"},
    {Header::Allow, "Allow"},
    {Header::WWW_Authenticate, "WWW-Authenticate"},
    {Header::Connection, "Connection"}
};

const QHash<QString, Response::Supported_Image_Ext> Response::static_map_available_image_ext =
{
    {
        "jpg", Supported_Image_Ext::JPG
    },
    {
        "jpeg", Supported_Image_Ext::JPG
    },
    {
        "jpe", Supported_Image_Ext::JPG
    },
    {
        "jfif", Supported_Image_Ext::JPG
    },
    {
        "png", Supported_Image_Ext::PNG
    },
    {
        "svg", Supported_Image_Ext::SVG
    },
    {
        "svgz", Supported_Image_Ext::SVG
    }
};




void Response::setData(ErrorResponse &&error_response)
{
    this->error_response = std::move(error_response);

    QJsonObject data = this->error_response.response_error;

    data.insert("success", 0);

    this->data = QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Compact);

    addHeader(Header::Content_Type, "application/json");

    addHeader(Header::Content_Length, QByteArray::number(this->data.length()));

    response_type = Response_Type::Json;
}



void Response::setData(QByteArray &&file_data, const Supported_Image_Ext file_type, const Content_Disposition content_disposition)
{
    this->data = std::move(file_data);


    switch (file_type)
    {
    case Supported_Image_Ext::JPG:
        addHeader(Header::Content_Type, "image/jpeg");

        break;

    case Supported_Image_Ext::PNG:
        addHeader(Header::Content_Type, "image/png");

        break;

    case Supported_Image_Ext::SVG:
        addHeader(Header::Content_Type, "image/svg+xml");

        break;

    case Supported_Image_Ext::Unknow:
        addHeader(Header::Content_Type, "application/octet-stream");

        break;
    }

    switch(content_disposition)
    {
    case Content_Disposition::Inline:
        addHeader(Header::Content_Disposition, "inline");

        break;

    case Content_Disposition::Attachment:
        addHeader(Header::Content_Disposition, "attachment");

        break;
    }

    addHeader(Header::Content_Length, QByteArray::number(this->data.length()));

    response_type = Response_Type::File;
}



void Response::setData(QJsonObject &&response_json)
{
    const QJsonObject temp = std::move(response_json);

    const QJsonObject data =
    {
        {"data", temp},
        {"success", 1}
    };

    this->data = QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Compact);

    addHeader(Header::Content_Type, "application/json");

    addHeader(Header::Content_Length, QByteArray::number(this->data.length()));

    response_type = Response_Type::Json;
}

void Response::sendHeaders(QTcpSocket *const socket)
{
    // HTTP/1.1 200 OK\r\n

    {
        const QByteArray start_line =
        "HTTP/1.1 " + QByteArray::number(static_cast<int>(response_status_code)) + " " +
        getStatusCodeValue().toUtf8() + "\r\n";

        socket->write(start_line);
    }

    if(data.isEmpty())
    {
        addHeader(Header::Content_Length, "0");
    }


    socket->write(headers);

    socket->write("\r\n");
}

void Response::sendData(QTcpSocket *const socket)
{
    socket->write(data);
}





