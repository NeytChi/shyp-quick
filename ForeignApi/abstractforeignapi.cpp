#include "abstractforeignapi.h"

AbstractForeignApi::AbstractForeignApi()
{

}


AbstractForeignApi::~AbstractForeignApi()
{
    if(manager != nullptr)
    {
        manager->deleteLater();
    }
}


void AbstractForeignApi::addRequest(ApiRequestProperty &&api_request_property)
{
    if(state != State::Not_Init)
    {
        list_queue.push(std::move(api_request_property));

        if(state == State::Ready)
        {
            sendRequest();
        }
    }
}


void AbstractForeignApi::sendRequest()
{
    if(!list_queue.empty())
    {
        if(state == State::Ready)
        {
            state = State::Handling;

            const auto &api_property = list_queue.front();

            api_property.calling_api_function();
        }
    }
}


void AbstractForeignApi::handleResponse()
{
    const ApiRequestProperty api_request_property = std::move(list_queue.front());

    list_queue.pop();

    bool is_valid = false;


    int status_code = 0;

    // Get response status code
    {
        const auto attribute_code = current_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

        is_valid = attribute_code.isValid();

        if(is_valid)
        {
            status_code = attribute_code.toInt(&is_valid);
        }
    }


    QJsonObject response_body = QJsonDocument::fromJson(received_data).object();

    QJsonObject log_error;

    if(is_valid)
    {
        is_valid = (status_code == 200) || (status_code == 201);
    }
    else
    {
        log_error.insert("status_code", "can't get response status code");
    }


    ErrorResponse error_response;

    if(!is_valid)
    {
        log_error.insert("reply_error", current_reply->errorString());

        log_error.insert("location", api_request_property.function_name);

        response_body.insert("log_error", log_error);

        error_response.insertLogError("api_response", response_body);
    }


    current_reply->deleteLater();

    current_reply = nullptr;

    received_data.clear();

    header_request = default_request;


    api_request_property.response_function(std::move(response_body), std::move(error_response));

    state = State::Ready;

    if(!list_queue.empty())
    {
        sendRequest();
    }
}


void AbstractForeignApi::connectReplyToSlots()
{
    QObject::connect(current_reply, &QNetworkReply::readyRead, [this]()
    {
        if(current_reply->error() == QNetworkReply::NoError)
        {
            received_data.append(current_reply->readAll());
        }
    });


    QObject::connect(current_reply, &QNetworkReply::finished, [this]()
    {
        handleResponse();
    });
}


std::pair<bool, QMap<QString, QString> > AbstractForeignApi::readConfigFile(const QString &file_name, const QSet<QString> &set_need_config)
{
    QFile file_config(Global::getPrimaryPath() + QStringLiteral("/Configs/ForeignApi/") + file_name + ".conf");


    bool is_init = file_config.open(QIODevice::ReadOnly | QIODevice::Text);

    QMap<QString, QString> map_received_config;

    if(is_init)
    {
        QTextStream stream(&file_config);

        while(!stream.atEnd())
        {
            QStringList list_parameter = stream.readLine().simplified().split(QStringLiteral("="));

            if( (list_parameter.size() == 2) && set_need_config.contains(list_parameter.first()))
            {
                map_received_config.insert(list_parameter.first(), list_parameter.last());
            }
        }

        is_init = (map_received_config.size() == set_need_config.size());

        if(!is_init)
        {
            QString missing_parameters = "Error to read \"" + file_name + ".conf\" - " + "Missing parameters: ";

            for(const auto &config_parameter : set_need_config)
            {
                if(!map_received_config.contains(config_parameter))
                {
                    missing_parameters.append(config_parameter).append(", ");

                    is_init = false;
                }
            }

            // Cut ', ' at the end
            missing_parameters.chop(2);

            Logger::writeLogInFile(missing_parameters, __PRETTY_FUNCTION__);
        }

        file_config.close();
    }
    else
    {
        Logger::writeLogInFile("can't open config file: " + file_config.fileName(), __PRETTY_FUNCTION__);
    }

    return {is_init, map_received_config};
}
