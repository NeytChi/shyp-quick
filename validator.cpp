#include "validator.h"

QJsonObject Validator::jsobject_keys_fields;

Validator::Validator()
{

}

Validator::Result Validator::checkIsStringNotEmpty(const QString &key_name, const QString &str)
{
    Result result;

    result.is_valid = !str.isEmpty();

    if(!result.is_valid)
    {
        result.error = "Please add a " + getFieldNameFromKey(key_name);
    }

    return result;
}


std::tuple<bool, QString> Validator::checkCommonString(const QString &key_name, const QString &str, const int max_size)
{
    auto [is_valid, error] = checkIsStringNotEmpty(key_name, str);

    if(is_valid)
    {
        is_valid = (str.size() <= max_size);

        if(!is_valid)
        {
            error = "Too many characters in " + getFieldNameFromKey(key_name) + " field";
        }
    }

    return {is_valid, error};
}

std::tuple<bool, QString> Validator::checkPassword(const QString &key_name, const QString &str)
{
    auto [is_valid, error] = checkIsStringNotEmpty(key_name, str);

    if(is_valid)
    {
        is_valid = (str.size() >= 6 && str.size() <= 15);

        if(!is_valid)
        {
            error = getFieldNameFromKey(key_name) + " should be 6-15 characters";
        }
    }

    return {is_valid, error};
}


std::tuple<bool, QString> Validator::checkEmail(const QString &str)
{
    auto [is_valid, error] = checkIsStringNotEmpty("email", str);

    if(is_valid)
    {
        is_valid = (str.size() < 254);

        if(!is_valid)
        {
            error = "Too many characters in " + getFieldNameFromKey("email") + " field";
        }
    }

    if(is_valid)
    {
        QRegExp mailREX("[A-Z0-9.!#$%&'*+-/?^_`{|}~]{1,64}@[A-Z0-9.-]+\\.[A-Z]+");
        mailREX.setCaseSensitivity(Qt::CaseInsensitive);

        is_valid =  mailREX.exactMatch(str.simplified());

        if(!is_valid)
        {
            error = "Email contains invalid characters";
        }
    }


    return {is_valid, error};
}


std::tuple<bool, QString>  Validator::checkLongitude(const double longitude)
{
    const bool is_valid = ( (longitude >= -180.0) && (longitude <= 180.0) );

    QString error;

    if(!is_valid)
    {
        error = ": longitude must >= -180.0 and <= +180.0";
    }

    return {is_valid, error};
}

std::tuple<bool, QString>  Validator::checkLatitude(const double latitude)
{
    const bool is_valid = ( (latitude >= -90.0) && (latitude <= 90.0) );

    QString error;

    if(!is_valid)
    {
        error = ": latitude must be >= -90.0 and <= +90.0";
    }

    return {is_valid, error};
}

Validator::Result Validator::checkFullName(const QString &key, const QString &name)
{
    auto [is_valid, error] = Validator::checkCommonString(key, name, 255);

    if(is_valid)
    {
        const auto list_name = name.split(" ");

        is_valid = (list_name.size() >= 2) && (list_name.size() <= 4);

        if(is_valid)
        {
            for(const auto &name : list_name)
            {
                is_valid = (name.length() > 0);

                if(!is_valid)
                {
                    break;
                }
            }
        }

        if(!is_valid)
        {
            error = "Wrong name format. " + getFieldNameFromKey(key) + " should consist of 2-4 words separated by space.";
        }
    }

    return {is_valid, std::move(error)};
}

