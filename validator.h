#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QRegExp>
#include <QJsonObject>
#include <QDebug>

class Validator
{

public:
    explicit Validator();

    struct Result
    {
        bool is_valid = false;
        QString error;
    };

    static Result checkIsStringNotEmpty(const QString &key_name, const QString &str);

    static std::tuple<bool, QString> checkCommonString(const QString &key_name, const QString &str, const int max_size);

    static std::tuple<bool, QString> checkPassword(const QString &key_name, const QString &str);

    static std::tuple<bool, QString> checkEmail(const QString &str);

    static std::tuple<bool, QString>  checkLongitude(const double longitude);
    static std::tuple<bool, QString>  checkLatitude(const double latitude);

    static Result checkFullName(const QString &key, const QString &full_name);

    static QString getFieldNameFromKey(const QString &key_name)
    {
        QString field_name = jsobject_keys_fields.value(key_name).toString();

        if(field_name.isEmpty())
        {
            field_name = key_name;
        }

        return field_name;
    }

private:

    static QJsonObject jsobject_keys_fields;

    static void setKeysFields(QJsonObject &&_jsobject_keys_fields_)
    {
        jsobject_keys_fields = std::move(_jsobject_keys_fields_);
    }

    friend class Setter;
};

#endif // VALIDATOR_H
