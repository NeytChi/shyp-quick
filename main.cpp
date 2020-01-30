#include <QCoreApplication>
#include "myserver.h"
#include "Global/global.h"

#include <signal.h>

class Setter
{
public:

    static void setPrimaryPath() { Global::primary_path =  QCoreApplication::applicationDirPath(); }

    static void setServerUrl(const QString &server_url) { Global::server_url = server_url; }

    static void setPictureDirPath(const QString &picture_dir_path) { Global::picture_dir_path = picture_dir_path; }

    static void setOrderSettings(Global::Order_Settings &&order_settings) { Global::order_settings = std::move(order_settings); }

    static void setEmailContactUs(const QString &email_contact_us) { Global::email_contact_us = email_contact_us; }

    static void setAPNS(Global::APNS &&apns_struct) { Global::apns_struct = std::move(apns_struct); }

    static void setJsonKeysToFields(QJsonObject &&_jsobject_keys_fields_)
    { Validator::setKeysFields(std::move(_jsobject_keys_fields_)); }
};


QSet<QString> getNeededParameters()
{
    const QSet<QString> set_need_config =
    {
      // For listen socket
      QStringLiteral("server_port"),

      // For database connection
      QStringLiteral("database_name"),
      QStringLiteral("database_username"),
      QStringLiteral("database_password"),

      // For email sender
      QStringLiteral("email_login"),
      QStringLiteral("email_password"),
      QStringLiteral("email_from"),
      QStringLiteral("smtp_server"),
      QStringLiteral("smtp_port"),

      QStringLiteral("server_url"),

      QStringLiteral("picture_dir_path"),

      // For Order_Settings
      QStringLiteral("application_tax_in_percent"),
      QStringLiteral("max_miles_from_pick_up_to_drop_off_without_tax"),
      QStringLiteral("dollars_for_one_mile"),
      QStringLiteral("start_searching_for_other_shippers_every_specified_seconds"),
      QStringLiteral("large_item_price"),
      QStringLiteral("small_item_price"),
      QStringLiteral("items_discount"),

      QStringLiteral("email_contact_us"),

      // For APNS
      QStringLiteral("apns_push_url"),
      QStringLiteral("apns_topic"),
      QStringLiteral("apns_certificate_password")
    };

    return set_need_config;
}


QMap<QString, QString> readConfigFile(const QSet<QString> &set_need_config);

void checkThatWeGetAllParameters(const QSet<QString> &set_need_config, const QMap<QString, QString> &map_received_config);

void setEmailSenderConfigs(const QMap<QString, QString> &map_received_config);

void setOrderSettings(const QMap<QString, QString> &map_received_config);

void setJsonKeysToFields();

void startServer(const quint16 server_port);

enum class Check_Number : unsigned char
{
    No,
    Unsigned,
    Positive_Number
};

template<class Type>
Type getParameterWhichIsNumber(const QString &key, const QMap<QString, QString> &map_received_config, const Check_Number check_number)
{
    bool constexpr is_valid_type = (std::is_same<Type, quint16>::value) ||
                                   (std::is_same<Type, int>::value) ||
                                   (std::is_same<Type, double>::value);

    static_assert (is_valid_type, "Invalid parameter type");

    Type number_parameter{};

    QString type_name;

    bool is_valid = false;

    const QString str_parameter = map_received_config.value(key);

    if constexpr(std::is_same<Type, quint16>::value)
    {
        number_parameter = str_parameter.toUShort(&is_valid);
        type_name = "quint16";
    }
    else if constexpr(std::is_same<Type, int>::value)
    {
        number_parameter = str_parameter.toInt(&is_valid);
        type_name = "int";
    }
    else if constexpr(std::is_same<Type, double>::value)
    {
        number_parameter = str_parameter.toDouble(&is_valid);
        type_name = "double";
    }


    if(is_valid)
    {
        if(check_number == Check_Number::Unsigned)
        {
            is_valid = (number_parameter >= 0);

            if(!is_valid)
            {
                Logger::writeLogInFile(key + " must be >= 0", __PRETTY_FUNCTION__);

                exit(1);
            }
        }
        else if(check_number == Check_Number::Positive_Number)
        {
            is_valid = (number_parameter > 0);

            if(!is_valid)
            {
                Logger::writeLogInFile(key + " must be > 0", __PRETTY_FUNCTION__);

                exit(1);
            }
        }
    }
    else
    {
        Logger::writeLogInFile("can't convert " + key + " to " + type_name, __PRETTY_FUNCTION__);

        exit(1);
    }

    return number_parameter;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // To ignore SIGPIPE, when we write to closed socket
    signal(SIGPIPE, SIG_IGN);

    //
    Setter::setPrimaryPath();

    qDebug() << Global::getPrimaryPath();


    const QSet<QString> set_need_config = getNeededParameters();

    const QMap<QString, QString> map_received_config = readConfigFile(set_need_config);

    checkThatWeGetAllParameters(set_need_config, map_received_config);


    //
    Setter::setServerUrl(map_received_config.value("server_url"));

    Setter::setPictureDirPath(map_received_config.value("picture_dir_path"));

    //
    qDebug() << "Set DataBaseConnector configs...\r\n";

    DataBaseConnector::setConfigs(
    map_received_config.value("database_name"),
    map_received_config.value("database_username"),
    map_received_config.value("database_password"));

    Setter::setAPNS(
    {map_received_config.value("apns_push_url"),
     map_received_config.value("apns_topic"),
     map_received_config.value("apns_certificate_password")});

    setEmailSenderConfigs(map_received_config);

    const quint16 server_port = getParameterWhichIsNumber<quint16>("server_port", map_received_config, Check_Number::Positive_Number);

    setOrderSettings(map_received_config);

    Setter::setEmailContactUs(map_received_config.value("email_contact_us"));

    setJsonKeysToFields();

    startServer(server_port);

    return a.exec();
}


QMap<QString, QString> readConfigFile(const QSet<QString> &set_need_config)
{
    QMap<QString, QString> map_received_config;

    qDebug() << "Try to open config file...\r\n";

    QFile file_config(Global::getPrimaryPath() + QStringLiteral("/Configs/ServerConfigs.conf"));

    if(file_config.open(QIODevice::ReadOnly | QIODevice::Text))
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

        file_config.close();
    }
    else
    {
        Logger::writeLogInFile("can't open config file: " + file_config.fileName(), __PRETTY_FUNCTION__);

        exit(1);
    }

    return map_received_config;
}


void checkThatWeGetAllParameters(const QSet<QString> &set_need_config, const QMap<QString, QString> &map_received_config)
{
    qDebug() << "Check did we get all needed parameters...\r\n";

    if(map_received_config.size() != set_need_config.size())
    {
        QString missing_parameters = "Missing parameters: ";

        for(const auto &config_parameter : set_need_config)
        {
            if(!map_received_config.contains(config_parameter))
            {
                missing_parameters.append(config_parameter).append(", ");
            }
        }

        // Cut ', ' at the end
        missing_parameters.chop(2);

        qDebug() << missing_parameters;

        Logger::writeLogInFile(missing_parameters, __PRETTY_FUNCTION__);

        exit(1);
    }
}


void setEmailSenderConfigs(const QMap<QString, QString> &map_received_config)
{
    qDebug() << "Set EmailSender configs...\r\n";

    bool is_valid = false;

    const quint16 smtp_port = map_received_config.value("smtp_port").toUShort(&is_valid);

    if(is_valid)
    {
        const EmailSender::Email_Property email_property =
        {
            map_received_config.value("email_login").toUtf8(),
            map_received_config.value("email_password").toUtf8(),

            map_received_config.value("email_from").toUtf8(),
            map_received_config.value("smtp_server"),
            smtp_port
        };

        EmailSender::setConfigs(email_property);

    }
    else
    {
        Logger::writeLogInFile("can't convert smtp_port to int for email sender", __PRETTY_FUNCTION__);

        exit(1);
    }
}


void setOrderSettings(const QMap<QString, QString> &map_received_config)
{
    double application_tax = getParameterWhichIsNumber<double>("application_tax_in_percent", map_received_config, Check_Number::Unsigned);

    if(application_tax > 0.0)
    {
        application_tax = application_tax / 100.0;
    }


    const double max_miles_from_pick_up_to_drop_off_without_tax =
    getParameterWhichIsNumber<double>("max_miles_from_pick_up_to_drop_off_without_tax", map_received_config, Check_Number::Unsigned);

    const double dollars_for_one_mile =
    getParameterWhichIsNumber<double>("dollars_for_one_mile", map_received_config, Check_Number::Unsigned);

    const int start_searching_for_other_shippers_every_specified_ms =
    getParameterWhichIsNumber<int>("start_searching_for_other_shippers_every_specified_seconds", map_received_config, Check_Number::Positive_Number) * 1000;

    const double small_item_price =
    getParameterWhichIsNumber<double>("small_item_price", map_received_config, Check_Number::Unsigned);

    const double large_item_price =
    getParameterWhichIsNumber<double>("large_item_price", map_received_config, Check_Number::Unsigned);

    const double items_discount =
    getParameterWhichIsNumber<double>("items_discount", map_received_config, Check_Number::Unsigned);


    Setter::setOrderSettings({
     application_tax,
     max_miles_from_pick_up_to_drop_off_without_tax,
     dollars_for_one_mile,
     start_searching_for_other_shippers_every_specified_ms,
     small_item_price,
     large_item_price,
     items_discount});

}

void setJsonKeysToFields()
{
    QFile file_config(Global::getPrimaryPath() + QStringLiteral("/Configs/keys_to_fields.json"));

    if(file_config.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Setter::setJsonKeysToFields(QJsonDocument::fromJson(file_config.readAll()).object());

        file_config.close();
    }
    else
    {
        Logger::writeLogInFile("can't open config file: " + file_config.fileName(), __PRETTY_FUNCTION__);

        exit(1);
    }
}

void startServer(const quint16 server_port)
{
    MyServer *my_server = new MyServer();

    QObject::connect(my_server, &MyServer::destroyed, []()
    {
        qDebug() << "\r\n---MyServer::destroyed---\r\nExit...";

        QCoreApplication::quit();
    });

    my_server->start(server_port);
}






