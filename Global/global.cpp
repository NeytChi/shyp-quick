#include "global.h"

QReadWriteLock Global::lock_stop_server;
bool Global::is_need_to_stop_server = false;

QString Global::primary_path;

QString Global::server_url;

QString Global::picture_dir_path;

Global::Order_Settings Global::order_settings;

QString Global::email_contact_us;

Global::APNS Global::apns_struct;
