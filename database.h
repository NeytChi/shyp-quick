#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QDebug>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QCoreApplication>

#include "Global/databaseconnector.h"
#include "Global/global.h"

class DataBase : public QObject
{
    Q_OBJECT



public:

    explicit DataBase(QObject *parent = nullptr);

    ~DataBase();

    QString query;
    bool init(QString &check_db_error);

    bool createTable(const QString &table, const QVector<QJsonObject> *const &vec, QString &check_db_error);


private:
    QSqlQuery db_query;



    //Create = false, Alter = true
    bool state = false;
    bool isInit = false;

    QString databaseName;
    QString primaryKey;
    QString tableName;


    QString alter;


    QString end;

    const int default_size = 1;

    struct Column_Inf
    {
        QString type;
        QString key;
        QString def;
        int index;
        int length;
        bool isNullable = false;
    };


    struct Required_Col
    {
        QString name;
        QString type;
        QString def;
        int size;
        bool isUnique;
        bool isNullable;
    };

    struct Full_Inf
    {
        QString first;
        QHash<QString, Column_Inf> db_column_inf;
        QVector<QString> vec_foreign;
    };

    struct Add_Foreign_key
    {
        const QString &column_name;
        const QString &onTable;
        const QString &onColumnName;
        const bool isUnique;
    };


    bool createSchema(const QString &schema);

    void add_id(const QString &name);
    void add_fk(const Add_Foreign_key &fk);
    void add_string(const QString &name, const int size, const QString &Default = "NULL");
    void add_unique(const QString &name);

    void autoCreate(const QVector<QJsonObject> *const &vector);

    void getInf(Full_Inf &full_inf);

    void alterColumn(const Column_Inf &col_inf, Required_Col &req_col);
    void addColumn(Required_Col &req_col, const QString &after);
    void createColumn(Required_Col &req_col);
    void addDefault(const Required_Col &req_col, QString &receiver)
    {
        if( (req_col.def != QLatin1String("NULL")) &&
            (req_col.def != QLatin1String("NOT NULL")) )
        {
            if(req_col.isNullable)
            {
                receiver.append(QStringLiteral("NULL"));
            }
            else
            {
                receiver.append(QStringLiteral("NOT NULL"));
            }

            if(!req_col.isUnique)
            {
                receiver.append(QStringLiteral(" DEFAULT "));
                if(req_col.type.contains(QLatin1String("int"), Qt::CaseInsensitive))
                {
                    receiver.append(req_col.def);
                }
                else if(req_col.type.contains("timestamp"))
                {
                    receiver.append(req_col.def);
                }
                else
                {
                    receiver.append(QStringLiteral("'%1'").arg(req_col.def));
                }
            }
        }
        else
        {
            receiver.append(req_col.def);
        }
    }



    QString change_uni_index(const QString &name, const bool isAdd = false)
    {
        if(isAdd)
        {
            return QStringLiteral("ADD UNIQUE INDEX `%1_%2` (%1 ASC);\n ")
                   .arg(name)
                   .arg(tableName);
        }
        else
        {
            return QStringLiteral("DROP INDEX `%1_%2`;\n ")
                   .arg(name)
                   .arg(tableName);
        }
    }
};


// Below line for FromMysqlToJson mysql generator. Don't touch!
// Start Tables Descriptions asdqwe53579dsf
class Tables
{
private:

    //user
    QVector<QJsonObject> user =
    {
        QJsonObject
        {
            {"name", "id_user"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "full_name"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "email"},
            {"type", "varchar"},
            {"size", 255},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "salt"},
            {"type", "varchar"},
            {"size", 255},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "hash_password"},
            {"type", "varchar"},
            {"size", 255},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "profile_picture_path"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "google_access_token"},
            {"type", "varchar"},
            {"size", 500},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "google_user_id"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "facebook_access_token"},
            {"type", "varchar"},
            {"size", 500},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "facebook_user_id"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "customer_account_id"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "state"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "zip_code"},
            {"type", "varchar"},
            {"size", 45}
        },
        QJsonObject
        {
            {"name", "phone_number"},
            {"type", "varchar"},
            {"size", 45},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "client_home_address"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "client_home_latitude"},
            {"type", "double"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "client_home_longitude"},
            {"type", "double"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "is_driver"},
            {"type", "tinyint(1) unsigned"}
        },
        QJsonObject
        {
            {"name", "driver_license_number"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "car_model"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "plate_number"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "shipper_order_state"},
            {"type", "unsigned"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "ready_to_ship"},
            {"type", "tinyint(1)"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "shipper_longitude"},
            {"type", "double"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "shipper_latitude"},
            {"type", "double"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "registration_date"},
            {"type", "timestamp"},
            {"default", "CURRENT_TIMESTAMP"}
        }
    };

    //auth_token
    QVector<QJsonObject> auth_token =
    {
        QJsonObject
        {
            {"name", "id_auth_token"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "id_user"},
            {"type", "unsigned"},
            {"haveForeign", true},
            {"onTable", "user"},
            {"onColumnName", "id_user"}
        },
        QJsonObject
        {
            {"name", "auth_token"},
            {"type", "varchar"},
            {"size", 255},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "is_driver"},
            {"type", "tinyint(1) unsigned"}
        }
    };

    //log
    QVector<QJsonObject> log =
    {
        QJsonObject
        {
            {"name", "id_log"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "headers"},
            {"type", "text"}
        },
        QJsonObject
        {
            {"name", "url"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "request_data"},
            {"type", "text"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "description"},
            {"type", "text"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "type"},
            {"type", "varchar"},
            {"size", 45}
        },
        QJsonObject
        {
            {"name", "success"},
            {"type", "tinyint(1)"},
            {"isNullable", true},
            {"default", "'0'"}
        },
        QJsonObject
        {
            {"name", "date"},
            {"type", "timestamp"},
            {"default", "CURRENT_TIMESTAMP"}
        },
        QJsonObject
        {
            {"name", "prepared_query"},
            {"type", "text"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "prepared_values"},
            {"type", "text"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "not_valid_headers"},
            {"type", "text"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "not_valid_values"},
            {"type", "text"},
            {"isNullable", true}
        }
    };

    //log_push
    QVector<QJsonObject> log_push =
    {
        QJsonObject
        {
            {"name", "id_log_push"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "type"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "id_user"},
            {"type", "unsigned"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "push_token"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "push_name"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "description"},
            {"type", "text"}
        },
        QJsonObject
        {
            {"name", "platform"},
            {"type", "varchar"},
            {"size", 45}
        },
        QJsonObject
        {
            {"name", "push_body"},
            {"type", "text"}
        },
        QJsonObject
        {
            {"name", "prepared_query"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "prepared_values"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "date"},
            {"type", "timestamp"},
            {"default", "CURRENT_TIMESTAMP"}
        }
    };

    //push_token
    QVector<QJsonObject> push_token =
    {
        QJsonObject
        {
            {"name", "id_push_token"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "id_user"},
            {"type", "unsigned"},
            {"haveForeign", true},
            {"onTable", "user"},
            {"onColumnName", "id_user"}
        },
        QJsonObject
        {
            {"name", "push_token"},
            {"type", "varchar"},
            {"size", 255},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "platform"},
            {"type", "varchar"},
            {"size", 50}
        }
    };

    //restore_pass
    QVector<QJsonObject> restore_pass =
    {
        QJsonObject
        {
            {"name", "id_restore_pass"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "id_user"},
            {"type", "unsigned"},
            {"haveForeign", true},
            {"onTable", "user"},
            {"onColumnName", "id_user"}
        },
        QJsonObject
        {
            {"name", "phone_number"},
            {"type", "varchar"},
            {"size", 45},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "sms_code"},
            {"type", "varchar"},
            {"size", 45},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "expires_in"},
            {"type", "timestamp"},
            {"default", "CURRENT_TIMESTAMP"}
        },
        QJsonObject
        {
            {"name", "attempts"},
            {"type", "unsigned"}
        },
        QJsonObject
        {
            {"name", "block_time"},
            {"type", "timestamp"},
            {"isNullable", true},
            {"default", "NULL"}
        }
    };

    //not_registered
    QVector<QJsonObject> not_registered =
    {
        QJsonObject
        {
            {"name", "id_not_registered"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "full_name"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "email"},
            {"type", "varchar"},
            {"size", 255},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "salt"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "hash_password"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "sms_code"},
            {"type", "varchar"},
            {"size", 45},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "expires_in"},
            {"type", "timestamp"},
            {"default", "CURRENT_TIMESTAMP"}
        },
        QJsonObject
        {
            {"name", "google_access_token"},
            {"type", "varchar"},
            {"size", 500},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "google_user_id"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "facebook_access_token"},
            {"type", "varchar"},
            {"size", 500},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "facebook_user_id"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "state"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "zip_code"},
            {"type", "varchar"},
            {"size", 45}
        },
        QJsonObject
        {
            {"name", "phone_number"},
            {"type", "varchar"},
            {"size", 45},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "client_home_address"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "client_home_latitude"},
            {"type", "double"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "client_home_longitude"},
            {"type", "double"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "is_driver"},
            {"type", "tinyint(1) unsigned"}
        },
        QJsonObject
        {
            {"name", "driver_license_number"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "car_model"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "plate_number"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        }
    };

    //payment_method
    QVector<QJsonObject> payment_method =
    {
        QJsonObject
        {
            {"name", "id_payment_method"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "id_user"},
            {"type", "unsigned"},
            {"isUnique", true},
            {"haveForeign", true},
            {"onTable", "user"},
            {"onColumnName", "id_user"}
        },
        QJsonObject
        {
            {"name", "card_token"},
            {"type", "varchar"},
            {"size", 255},
            {"isUnique", true}
        },
        QJsonObject
        {
            {"name", "last_four_numbers"},
            {"type", "varchar"},
            {"size", 4}
        },
        QJsonObject
        {
            {"name", "card_type"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "exp_month"},
            {"type", "unsigned"}
        },
        QJsonObject
        {
            {"name", "exp_year"},
            {"type", "unsigned"}
        }
    };

    //orders
    QVector<QJsonObject> orders =
    {
        QJsonObject
        {
            {"name", "id_order"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "id_client"},
            {"type", "unsigned"},
            {"haveForeign", true},
            {"onTable", "user"},
            {"onColumnName", "id_user"}
        },
        QJsonObject
        {
            {"name", "id_shipper"},
            {"type", "unsigned"},
            {"isNullable", true},
            {"haveForeign", true},
            {"onTable", "user"},
            {"onColumnName", "id_user"}
        },
        QJsonObject
        {
            {"name", "stripe_charge_id"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "stripe_charge_status"},
            {"type", "tinyint(1) unsigned"}
        },
        QJsonObject
        {
            {"name", "last_four_numbers"},
            {"type", "varchar"},
            {"size", 4}
        },
        QJsonObject
        {
            {"name", "card_type"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "state"},
            {"type", "unsigned"}
        },
        QJsonObject
        {
            {"name", "is_open"},
            {"type", "tinyint(1) unsigned"}
        },
        QJsonObject
        {
            {"name", "pick_up_address"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "pick_up_longitude"},
            {"type", "double"}
        },
        QJsonObject
        {
            {"name", "pick_up_latitude"},
            {"type", "double"}
        },
        QJsonObject
        {
            {"name", "drop_off_address"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "drop_off_longitude"},
            {"type", "double"}
        },
        QJsonObject
        {
            {"name", "drop_off_latitude"},
            {"type", "double"}
        },
        QJsonObject
        {
            {"name", "order_price_without_tax"},
            {"type", "double unsigned"}
        },
        QJsonObject
        {
            {"name", "tax"},
            {"type", "double unsigned"}
        },
        QJsonObject
        {
            {"name", "distance_in_miles"},
            {"type", "double unsigned"}
        },
        QJsonObject
        {
            {"name", "time_to_deliver"},
            {"type", "int"}
        },
        QJsonObject
        {
            {"name", "arrive_by"},
            {"type", "timestamp"},
            {"default", "CURRENT_TIMESTAMP"}
        },
        QJsonObject
        {
            {"name", "stripe_transfer_to_shipper_account_id"},
            {"type", "varchar"},
            {"size", 255},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "time_of_delivery"},
            {"type", "timestamp"},
            {"isNullable", true},
            {"default", "NULL"}
        },
        QJsonObject
        {
            {"name", "client_rated"},
            {"type", "unsigned"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "shipper_rated"},
            {"type", "unsigned"},
            {"isNullable", true}
        },
        QJsonObject
        {
            {"name", "creation_date"},
            {"type", "timestamp"},
            {"default", "CURRENT_TIMESTAMP"}
        }
    };

    //item
    QVector<QJsonObject> item =
    {
        QJsonObject
        {
            {"name", "id_item"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "item_name"},
            {"type", "varchar"},
            {"size", 255}
        },
        QJsonObject
        {
            {"name", "size"},
            {"type", "tinyint(1) unsigned"}
        }
    };

    //item_for_order
    QVector<QJsonObject> item_for_order =
    {
        QJsonObject
        {
            {"name", "id_item_for_order"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "id_item"},
            {"type", "unsigned"},
            {"haveForeign", true},
            {"onTable", "item"},
            {"onColumnName", "id_item"}
        },
        QJsonObject
        {
            {"name", "id_order"},
            {"type", "unsigned"},
            {"haveForeign", true},
            {"onTable", "orders"},
            {"onColumnName", "id_order"}
        }
    };

    //shipper_getted_order
    QVector<QJsonObject> shipper_getted_order =
    {
        QJsonObject
        {
            {"name", "id_shipper_getted_order"},
            {"type", "primary"}
        },
        QJsonObject
        {
            {"name", "id_order"},
            {"type", "unsigned"},
            {"haveForeign", true},
            {"onTable", "orders"},
            {"onColumnName", "id_order"}
        },
        QJsonObject
        {
            {"name", "id_shipper"},
            {"type", "unsigned"},
            {"haveForeign", true},
            {"onTable", "user"},
            {"onColumnName", "id_user"}
        }
    };


public:

    QVector<const QVector<QJsonObject>*> getVector(QVector<QString> &name_table)
    {

        QVector<const QVector<QJsonObject>*> tables;




        name_table.append("user");
        name_table.append("auth_token");
        name_table.append("log");
        name_table.append("log_push");
        name_table.append("push_token");
        name_table.append("restore_pass");
        name_table.append("not_registered");
        name_table.append("payment_method");
        name_table.append("orders");
        name_table.append("item");
        name_table.append("item_for_order");
        name_table.append("shipper_getted_order");



        tables.append(&user);
        tables.append(&auth_token);
        tables.append(&log);
        tables.append(&log_push);
        tables.append(&push_token);
        tables.append(&restore_pass);
        tables.append(&not_registered);
        tables.append(&payment_method);
        tables.append(&orders);
        tables.append(&item);
        tables.append(&item_for_order);
        tables.append(&shipper_getted_order);

        return tables;
    }
};

#endif // DATABASE_H
