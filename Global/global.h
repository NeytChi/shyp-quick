#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

#include <cmath>

#include <QDebug>
#include <QReadWriteLock>

class Global
{
private:
    Global() = delete;

    static QReadWriteLock lock_stop_server;
    static bool is_need_to_stop_server;

    static QString primary_path;
    static QString server_url;
    static QString picture_dir_path;

    struct Order_Settings
    {
        double application_tax;
        double max_miles_from_pick_up_to_drop_off_without_tax;
        double dollars_for_one_mile;
        int start_searching_for_other_shippers_every_specified_ms;
        double large_item_price;
        double items_discount;
    };

    static Order_Settings order_settings;

    static QString email_contact_us;

    struct APNS
    {
        QString apns_push_url;
        QString apns_topic;
        QString apns_certificate_password;
    };

    static APNS apns_struct;

public:

    [[nodiscard]]
    static bool isNeedToStopServer()
    {
        lock_stop_server.lockForRead();

        const bool value = is_need_to_stop_server;

        lock_stop_server.unlock();

        return value;
    }

    static void stopServer()
    {
        lock_stop_server.lockForWrite();

        is_need_to_stop_server = true;

        lock_stop_server.unlock();
    }

    [[nodiscard]]
    static const QString& getPrimaryPath() { return primary_path; }

    [[nodiscard]]
    static const QString& getServerUrl() { return server_url; }

    [[nodiscard]]
    static const QString& getPictureDirPath() { return picture_dir_path; }

    [[nodiscard]]
    static const APNS& getAPNS() { return apns_struct; }

    [[nodiscard]]
    static double roundDoubleToTwoPointsDecimal(const double value)
    {
        return std::round(value * 100.0) / 100.0; // round to 2 points decimal
    }

    [[nodiscard]]
    static const Order_Settings& getOrderSettings() { return order_settings; }

    [[nodiscard]]
    static const QString& getEmailContactUs() { return email_contact_us; }

    friend class Setter;
};

#endif // GLOBAL_H
