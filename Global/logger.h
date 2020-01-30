#ifndef LOGGER_H
#define LOGGER_H


#include <QDebug>

#include <QString>

#include <QFile>
#include <QDir>
#include <QCoreApplication>

#include <QJsonDocument>
#include <QJsonObject>

#include <QMutex>

#include <QDateTime>

#include "Global/global.h"

class Logger
{

private:

    Logger() = delete;


    static QMutex mutex_file;

public:

    template<class Error>
    static void writeLogInFile(const Error &error, const QString &location)
    {
        qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";


        QMutexLocker locker_mutex(&mutex_file);

        constexpr bool is_string = std::is_same<Error, QString>::value || std::is_convertible<Error, QString>::value;

        constexpr bool is_object = std::is_same<Error, QJsonObject>::value;

        static_assert (is_string || is_object, "Invalid type in writeLogInFile()");

        QFile log_file(Global::getPrimaryPath() + QStringLiteral("/Configs/logs.conf"));

        if(log_file.open(QIODevice::Append | QIODevice::Text))
        {
            QDateTime date_time = QDateTime::currentDateTime();

            if constexpr(is_string)
            {
                const QJsonObject object =
                {
                    {"location", location},

                    {QStringLiteral("error_description"), error},

                    {QStringLiteral("type"), QStringLiteral("Internal Server Error")},

                    {QStringLiteral("log_date"), date_time.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")) + toTimeWithUtcOffset(date_time.offsetFromUtc())},

                    {QStringLiteral("success"), false}
                };

                log_file.write(QJsonDocument(object).toJson());
            }
            else if constexpr(is_object)
            {
                QJsonObject object = error;

                object.insert("location", location);

                object.insert("log_date", date_time.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")) + toTimeWithUtcOffset(date_time.offsetFromUtc()));

                log_file.write(QJsonDocument(object).toJson());
            }


            log_file.write(QByteArrayLiteral("\r\n---------\r\n\r\n"));
            log_file.flush();

            log_file.close();
        }
        else
        {
            qDebug() << "Failed to open = " << log_file.fileName();
        }
    }

    static QString toTimeWithUtcOffset(const int &sec);

};

#endif // LOGGER_H
