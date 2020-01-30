#include "logger.h"

QMutex Logger::mutex_file;


QString Logger::toTimeWithUtcOffset(const int &sec)
{
    int hour = 0;
    int minute = 0;

    if(sec != 0)
    {
        hour = sec / 3600;

        minute = (sec - (hour * 3600)) / 60;
    }

    QString offset = QStringLiteral(" UTC ");

    if(sec >= 0)
    {
        offset.append(QLatin1Char('+'));

        if(hour < 10)
        {
            offset.append("0");
        }
        offset.append(QString::number(hour)).append(QLatin1Char(':'));

        if(minute < 10)
        {
            offset.append("0");
        }
    }
    else
    {
        offset.append(QLatin1Char('-'));

        if(hour > -10)
        {
            offset.append("0");
        }
        offset.append(QString::number(abs(hour))).append(QLatin1Char(':'));

        if(minute > -10)
        {
            offset.append("0");
        }
    }

    offset.append(QString::number(abs(minute)));

    return offset;
}
