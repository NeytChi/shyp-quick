#include "requestbody.h"

RequestBody::RequestBody()
{

}

RequestBody::~RequestBody()
{

}

RequestBody::RequestBody(RequestBody &&other) noexcept
    : file_map(std::move(other.file_map)),
      received_json(std::move(other.received_json)),
      map_get_parameter(std::move(other.map_get_parameter)),
      request_url(std::move(other.request_url))
{

}




RequestBody &RequestBody::operator=(RequestBody &&other) noexcept
{
    if(this != &other)
    {
        file_map = std::move(other.file_map);

        received_json = std::move(other.received_json);

        map_get_parameter = std::move(other.map_get_parameter);

        request_url = std::move(other.request_url);
    }

    return *this;
}



ErrorResponse RequestBody::parseJson(const QByteArray &request_data)
{
    ErrorResponse error_response;

    QJsonParseError parser_error;

    const QJsonDocument json_doc = QJsonDocument::fromJson(request_data, &parser_error);

    const bool is_valid_data = !json_doc.isNull() && (parser_error.error == QJsonParseError::NoError) && json_doc.isObject();

    const bool is_not_empty_data = !json_doc.isEmpty();

    if(is_valid_data && is_not_empty_data)
    {
        received_json = json_doc.object();
    }
    else if(!is_valid_data)
    {
        if(json_doc.isNull() || (parser_error.error != QJsonParseError::NoError))
        {
            error_response.insertErrorDescription(parser_error.errorString());
        }
        else
        {
            error_response.insertErrorDescription("this is not a object");
        }
    }

    if(!is_valid_data)
    {
        error_response.insertError(error_invalid_data);
    }
    else if(!is_not_empty_data)
    {
        error_response.insertError(error_empty_data);
    }

    return error_response;
}



ErrorResponse RequestBody::parseMultiData(QByteArray &request_data, const QString &boundary)
{
    /*
     * -----------------------------1950805726999813559643879861\r\n
     * Content-Disposition: form-data; name=\"f\"; filename=\"dead.letter\"\r\n
     * Content-Type: application/octet-stream\r\n
     * \r\n
     * From pancake@Oven Fri Sep 14 11:39:02 2018\n
     * Subject: ad\nTo: m\n\n\n\nda\nadw\n.\n.\n\n\r\n
     * -----------------------------1950805726999813559643879861\r\n
     * Content-Disposition: form-data; name=\"lll\"\r\n
     * \r\n
     * Somebody once told me the world is gonna roll me\\r\\n I ain't the sharpest tool in the shed\r\n
     * -----------------------------1950805726999813559643879861--\r\n
     *
     */


    QString error_description;


    // --boundary\r\n
    //1. Skip boundary -> -current_index -> boundary.size() + (2 -> \r\n)
    int current_index = boundary.size() + 2;


    while(true)
    {
        QString name, filename;

        // Content-Disposition: form-data; name=\"f\"; filename=\"dead.letter\"\r\n
        {
            //2. -start_disposition -> -current_index
            const int start_disposition = current_index;

            //3. Get end of Content-Disposition -end_disposition -> \r\n, from -start_disposition
            const int end_disposition = request_data.indexOf("\r\n", start_disposition);

            if(end_disposition == -1)
            {
                error_description = "can't find end of Content-Disposition line";
                break;
            }

            //4. Get name(required), filename(not required) between -start_disposition and -end_disposition

            // Get name
            {
                const QString key_name = "name=\"";

                int name_start = request_data.indexOf(key_name, start_disposition);

                if( (name_start == -1) || (name_start >= end_disposition) )
                {
                    error_description = "can't find 'name' of Content-Disposition line";
                    break;
                }

                name_start += key_name.size();

                const int name_end = request_data.indexOf("\"", name_start);

                if( (name_end == -1) || (name_end >= end_disposition) )
                {
                    error_description = "can't find end of 'name' of Content-Disposition line";
                    break;
                }

                name = QString::fromUtf8(request_data.mid(name_start, name_end - name_start));
            }


            // Get filename
            {
                const QString key_name = "filename=\"";

                int filename_start = request_data.indexOf(key_name, start_disposition);

                if( (filename_start != -1) && (filename_start < end_disposition) )
                {
                    filename_start += key_name.size();

                    const int filename_end = request_data.indexOf("\"", filename_start);

                    if( (filename_end == -1) || (filename_end >= end_disposition) )
                    {
                        error_description = "can't find end of 'filename' of Content-Disposition line";
                        break;
                    }

                    filename = QString::fromUtf8(request_data.mid(filename_start, filename_end - filename_start));
                }
            }

            //5. -current_index -> -end_disposition + (2 -> \r\n)
            current_index = end_disposition + 2;
        }


        QString content_type;

        // Content-Type: application/octet-stream\r\n

        //6. If (-current_index != '\r'), it means that we on Content-Type line
        if(request_data.at(current_index) != '\r')
        {
            //6.1. -start_type -> -current_index
            int start_type = current_index;

            //6.2. -end_type -> \r\n, from -start_type
            const int end_type = request_data.indexOf("\r\n", start_type);

            if(end_type == -1)
            {
                error_description = "can't find end of Content-Type line";
                break;
            }

            //6.3. Get content type between -start_type and -end_type // application/octet-stream
            start_type = request_data.indexOf(":", start_type);

            if( (start_type == -1) || (start_type >= end_type))
            {
                error_description = "can't find start of type of Content-Type line";
                break;
            }

            start_type += 2;

            content_type = QString::fromUtf8(request_data.mid(start_type, end_type - start_type));

            //6.4. -current_index -> -end_type + (2 -> \r\n)
            current_index = end_type + 2;
        }


        // \r\n
        // body\r\n
        // --boundary\r\n or --boundary--\r\n

        //7. If (-current_index == '\r'), it means that we on start of body
        if(request_data.at(current_index) == '\r')
        {
            //8. Skip empty line -current_index -> -current_index + (2 -> \r\n)
            current_index += 2;


            //9. -start_body = -current_index
            const int start_body = current_index;


            //10. -end_body -> boundary - (2 -> \r\n)
            int end_body = request_data.indexOf(boundary, start_body);

            if(end_body == -1)
            {
                error_description = "can't find end of body of " + name;
                break;
            }

            end_body -= 2;

            //11. Get body
            if(!filename.isEmpty())
            {
                QByteArray file_data = request_data.mid(start_body, end_body - start_body);

                QMimeDatabase mime_database;

                QMimeType mime_type = mime_database.mimeTypeForData(file_data);

                file_map.insert(name, {filename, mime_type, std::move(file_data)});
            }
            else
            {
                const QString value_str = QString::fromUtf8(request_data.mid(start_body, end_body - start_body));

                bool is_value_number = false;

                const int number = value_str.toInt(&is_value_number);


                if(is_value_number)
                {
                    received_json.insert(name, number);
                }
                else
                {
                    received_json.insert(name, value_str);
                }
            }

            request_data.remove(0, end_body);


            //12. -current_index -> -end_body + (2 -> \r\n), -current_index -> \r\n
            current_index = 2; // on boundary

            current_index = request_data.indexOf("\r\n", current_index);


            if(current_index == -1)
            {
                error_description = "can't find end of boundary after " + name;
                break;
            }

            //13. -current_index -> -current_index + (2 -> \r\n)
            current_index += 2;


            //14. If (-current_index == request_data.length()), it means that we reached the end
            if(current_index == request_data.length())
            {
                break;
            }
        }
        else
        {
            error_description = "can't find start of body after " + name;
            break;
        }

    }


    ErrorResponse error_response;

    if(!error_description.isEmpty())
    {
        error_response.insertError(error_invalid_data);

        error_response.insertErrorDescription(error_description);
    }
    else if(received_json.isEmpty() && file_map.isEmpty())
    {
        error_response.insertError(error_empty_data);
    }

    return error_response;
}



ErrorResponse RequestBody::parseAppUrlEncoded(const QByteArray &request_data)
{
    ErrorResponse error_response;


    const QString url_encoded = QString::fromUtf8(request_data);


    // say=Hi&to=Mom

    // 1. Split with separator -> '&'
    const QVector<QStringRef> vec_parameter = url_encoded.splitRef('&');

    bool is_valid_data = false;

    // vec_parameter = { say=Hi, to=Mom }

    // 2. Split with separator -> '=' // vec = { say, Hi }
    for(const auto &parameter : vec_parameter)
    {
        const QVector<QStringRef> vec_key_value = parameter.split('=');

        is_valid_data = (vec_key_value.size() == 1) || (vec_key_value.size() == 2);

        if(is_valid_data)
        {
            const QString key = vec_key_value.first().toString();

            // != -> /page?=
            is_valid_data = !key.isEmpty();

            if(is_valid_data)
            {
                // key = value
                if(vec_key_value.size() == 2)
                {
                    received_json.insert(key, vec_key_value.last().toString());
                }
                // key
                else
                {
                    received_json.insert(key, QString());
                }
            }
            else
            {
                error_response.insertErrorDescription("empty key");
            }
        }
        else
        {
            error_response.insertErrorDescription("invalid size of parameters with '='");
        }


        if(!is_valid_data)
        {
            break;
        }
    }


    if(!is_valid_data)
    {
        error_response.insertError(error_invalid_data);
    }
    else if(received_json.isEmpty())
    {
        error_response.insertError(error_empty_data);
    }


    return error_response;
}



ErrorResponse RequestBody::parseGetParameters(const QString &url)
{
    request_url = url;

    ErrorResponse error_response;



    // 1. Check does exist '?'
    int begin_query = url.indexOf('?');

    if(begin_query == -1)
    {
        return error_response;
    }


    // /page?name=ferret&color=purple

    // 2. Seperate all from '?'
    const QStringRef line_parameter = url.rightRef(url.size() - begin_query - 1);


    // name=ferret&color=purple

    // 3. Split with separator -> '&'
    const QVector<QStringRef> vec_parameter = line_parameter.split('&');


    bool is_valid_data = false;

    // vec_parameter = { name=ferret, color=purple }

    // 4. Split with separator -> '=' // vec = { name, ferret }
    for(const auto &parameter : vec_parameter)
    {
        const QVector<QStringRef> vec_key_value = parameter.split('=');

        is_valid_data = (vec_key_value.size() == 1) || (vec_key_value.size() == 2);

        if(is_valid_data)
        {
            const QString key = vec_key_value.first().toString();

            // != -> /page?=
            is_valid_data = !key.isEmpty();

            if(is_valid_data)
            {
                // key = value
                if(vec_key_value.size() == 2)
                {
                    map_get_parameter.insert(key, vec_key_value.last().toString());
                }
                // key
                else
                {
                    map_get_parameter.insert(key, QString());
                }
            }
            else
            {
                error_response.insertErrorDescription("empty key");
            }
        }
        else
        {
            error_response.insertErrorDescription("invalid size of parameters with '='");
        }


        if(!is_valid_data)
        {
            break;
        }
    }


    if(!is_valid_data)
    {
        error_response.insertError(error_invalid_data);
    }
    else if(map_get_parameter.isEmpty())
    {
        error_response.insertError(error_empty_data);
    }

    return error_response;
}
