#include "database.h"

DataBase::DataBase(QObject *parent) : QObject(parent)
{

}

DataBase::~DataBase()
{

    if(db_query.isValid())
    {
        db_query.clear();
    }

    {
        QSqlDatabase data_base = QSqlDatabase::database(databaseName);
        data_base.close();
    }
    QSqlDatabase::removeDatabase(databaseName);
}

bool DataBase::init(QString &check_db_error)
{
    // 1. Connect to database
    // 2. Check does already exist current schema
    // 2.1 If no then create new schema
    // 3. Connect to current schema

    QSqlDatabase data_base;

    const QString db_connection_name = DataBaseConnector::getSchemaName();

    bool is_opend_db = DataBaseConnector::connectToDB(data_base, db_connection_name, DataBaseConnector::Open_With_Schema_Name::No);


    if(!is_opend_db)
    {
        check_db_error = QStringLiteral("can't open database ") + data_base.lastError().text();

        DataBaseConnector::disconnectFromDB(db_connection_name);

        return false;
    }


    databaseName = DataBaseConnector::getSchemaName();


    // Select current schema
    {
        const QString select = QStringLiteral("SELECT 1 FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '%1'").arg(databaseName);

        db_query = data_base.exec(select);
    }


    if(!db_query.next())
    {
        bool is_create_schema = createSchema(databaseName);

        if(!is_create_schema)
        {
            check_db_error = QStringLiteral("can't create database ") + data_base.lastError().text();

            DataBaseConnector::disconnectFromDB(db_connection_name);

            return false;
        }
    }

    DataBaseConnector::disconnectFromDB(db_connection_name);


    is_opend_db = DataBaseConnector::connectToDB(data_base, db_connection_name, DataBaseConnector::Open_With_Schema_Name::Yes);

    if(!is_opend_db)
    {
        check_db_error = QStringLiteral("can't open current database ") + data_base.lastError().text();

        DataBaseConnector::disconnectFromDB(db_connection_name);

        return false;
    }



    db_query = QSqlQuery(data_base);

    isInit = true;


    return true;

}


bool DataBase::createSchema(const QString &schema)
{
    QSqlDatabase data_base = QSqlDatabase::database(schema);
    data_base.exec(QStringLiteral("CREATE DATABASE IF NOT EXISTS %1 DEFAULT CHARACTER SET utf8;").arg(schema));

    if(data_base.lastError().type() != QSqlError::NoError)
    {
        return false;
    }

    return true;
}

bool DataBase::createTable(const QString &table, const QVector<QJsonObject> *const &vec, QString &check_db_error)
{
    query.clear();
    end.clear();
    alter.clear();
    primaryKey.clear();
    tableName = table;

    QSqlDatabase data_base = QSqlDatabase::database(databaseName);

    if(!isInit || !data_base.isValid())
    {
        check_db_error = QStringLiteral("need initialization before create table");
        return false;
    }


    if(data_base.tables().contains(table))
    {
        alter = (QStringLiteral("ALTER TABLE %1.%2\n ").arg(databaseName).arg(table));
        state = true;
    }
    else
    {
        query.append(QStringLiteral("CREATE TABLE IF NOT EXISTS %1.%2 (\n ").arg(databaseName).arg(table));
        state = false;
    }

    autoCreate(vec);

    if(!state)
    {
        if(primaryKey.isEmpty())
        {
            add_id(QStringLiteral("id"));

        }


        query.append(end);

        if(query.endsWith(QStringLiteral(",\n ")))
        {
            query.chop(3);
            query.append(QStringLiteral("\n) "));
        }



        query.append(QStringLiteral("ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=utf8;"));

        QFile logs(Global::getPrimaryPath() + QStringLiteral("/Configs/logs.conf"));
        if(!logs.open(QIODevice::Append | QIODevice::Text))
        {
            qDebug() << "Failed";
        }
        else
        {
            logs.write(query.toUtf8());
            logs.write(QByteArrayLiteral("\r\n---------\r\n\r\n"));
            logs.flush();
            logs.close();
        }

        db_query.exec(query);
    }
    else
    {
        if(!query.isEmpty())
        {

           db_query.clear();

           bool isTR = data_base.transaction();

           if(isTR)
           {
               QVector<QStringRef> vec_ref = query.splitRef(QStringLiteral(";\n "));

               qDebug() << vec_ref;
               for(const auto &it : vec_ref)
               {
                   if(!it.isEmpty())
                   {
                       db_query.exec(it.toString().trimmed());
                   }

                   if(db_query.lastError().type() != QSqlError::NoError)
                   {
                       isTR = false;
                       qDebug() << "Rollback ( Q_Q" << db_query.lastError()
                                << "rollback" << data_base.rollback();
                       break;
                   }
               }


               if(isTR)
               {
                   bool isCC = data_base.commit();
                   if(!isCC)
                   {
                       qDebug() << "Can not commit" << db_query.lastError() << data_base.lastError();

                       check_db_error = QStringLiteral("server error when commit ") + data_base.lastError().text();
                   }

                   qDebug() << "isCC: " << isCC;
               }

           }
           else
           {
               qDebug() << "Can not transaction" << db_query.lastError() << data_base.lastError();

               check_db_error = QStringLiteral("server error when begin transaction ") + db_query.lastError().text();
           }

           qDebug() << "isTR: " << isTR;

        }

    }


    if(db_query.lastError().type() != QSqlError::NoError)
    {
        check_db_error = QStringLiteral("server error when create or alter table ") + db_query.lastError().text();

        return false;
    }



    return true;


}


void DataBase::add_unique(const QString &name)
{


    end.append(QStringLiteral("UNIQUE INDEX %1_%2 (%1 ASC),\n ")
               .arg(name).arg(tableName));


}


void DataBase::add_id(const QString &name)
{
    if(primaryKey.isEmpty())
    {
        query.insert(query.indexOf('(') + 2, QStringLiteral(" %1 INT UNSIGNED NOT NULL AUTO_INCREMENT,\n").arg(name));

        primaryKey = name;
        end.append(QStringLiteral("PRIMARY KEY (%1),\n ").arg(primaryKey));

        add_unique(name);
    }

}


void DataBase::add_string(const QString &name, const int size, const QString &Default)
{

    query.append(QStringLiteral("%1 varchar(%2) ").arg(name).arg(size));

    if(Default != QLatin1String("NOT NULL"))
    {
        query.append(QStringLiteral("DEFAULT "));
        if(Default == QLatin1String("NULL"))
        {
            query.append(Default);
        }
        else
        {
            query.append(QStringLiteral("'%1'").arg(Default));
        }
    }
    else
    {
        query.append(Default);
    }

    query.append(QStringLiteral(",\n "));
}


void DataBase::add_fk(const Add_Foreign_key &fk)
{

    if(!fk.column_name.isEmpty() && !fk.onTable.isEmpty() && !fk.onColumnName.isEmpty())
    {
        QString temp;
        if(state)
        {
            temp.append(alter);
            if(!fk.isUnique)
            {
               temp.append(QStringLiteral("ADD INDEX fk_%1_INDEX_%2 (%1 ASC);\n ")
                           .arg(fk.column_name).arg(tableName));
               temp.append(alter);
            }
            temp.append(QStringLiteral("ADD "));

        }
        else if(!fk.isUnique)
        {
           temp.append(QStringLiteral("INDEX fk_%1_INDEX_%2 (%1 ASC),\n ")
                       .arg(fk.column_name).arg(tableName));
        }


        temp.append(QStringLiteral("CONSTRAINT fk_%1_%2 FOREIGN KEY (%1) REFERENCES %3 (%4) ON DELETE CASCADE ON UPDATE NO ACTION")
                     .arg(fk.column_name)
                     .arg(tableName)
                     .arg(databaseName + "." + fk.onTable)
                     .arg(fk.onColumnName));
        if(state)
        {
            temp.append(QStringLiteral(";\n "));
            query.append(temp);
        }
        else
        {
            temp.append(QStringLiteral(",\n "));
            end.append(temp);
        }
    }

}


void DataBase::getInf(Full_Inf &full_inf)
{
    db_query.exec(QStringLiteral("SELECT COLUMN_NAME, COLUMN_TYPE, "
                                 "COLUMN_DEFAULT, COLUMN_KEY, IS_NULLABLE, CHARACTER_MAXIMUM_LENGTH "
                                 "FROM INFORMATION_SCHEMA.COLUMNS "
                                 "WHERE table_name = '%1' AND table_schema = '%2'").arg(tableName).arg(databaseName));


    int index = 0;
    while(db_query.next())
    {
        Column_Inf inf;
        inf.index = index;
        inf.type = db_query.value(1).toString();
        inf.length = db_query.value(5).toInt();
        inf.key = db_query.value(3).toString();

        if(index == 0)
        {
            full_inf.first = db_query.value(0).toString();
        }

        inf.def = db_query.value(2).toString();


        if(db_query.value(4).toString() == QLatin1String("YES"))
        {
            if(inf.def.isEmpty())
            {
                inf.def = QStringLiteral("NULL");
            }
            inf.isNullable = true;
        }
        else if(inf.def.isEmpty())
        {
            inf.def = QStringLiteral("NOT NULL");
        }

       full_inf.db_column_inf.insert(db_query.value(0).toString(), inf);

       ++index;
    }

    db_query.exec(QStringLiteral("SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE "
                                 "WHERE table_name = '%1' AND table_schema = '%2' "
                                 "AND CONSTRAINT_NAME LIKE 'fk%';").arg(tableName).arg(databaseName));


    while(db_query.next())
    {
        full_inf.vec_foreign.append(db_query.value(0).toString());
    }
}


void DataBase::autoCreate(const QVector<QJsonObject> *const &vector)
{

    QVector<QString> vec_NameValues;
    vec_NameValues.resize(vector->size());


    Full_Inf full_inf;

    getInf(full_inf);


    for(const auto &jobj : *vector)
    {

        Required_Col req_col;
        req_col.name = jobj.value(QStringLiteral("name")).toString();
        req_col.type = jobj.value(QStringLiteral("type")).toString();
        req_col.size = jobj.value(QStringLiteral("size")).toInt();


        if(!req_col.name.isEmpty() && !req_col.type.isEmpty())
        {
            req_col.isUnique = jobj.value(QStringLiteral("isUnique")).toBool();
            req_col.def = jobj.value(QStringLiteral("default")).toString();
            req_col.isNullable = jobj.value(QStringLiteral("isNullable")).toBool();


            // Check default value

            {
                if(req_col.def.isEmpty())
                {
                    QJsonValue value = jobj.value(QStringLiteral("default"));

                    if(!value.isDouble())
                    {
                        if(req_col.isNullable)
                        {
                            req_col.def = QStringLiteral("NULL");
                        }
                        else
                        {
                            req_col.def = QStringLiteral("NOT NULL");
                        }

                    }
                    else
                    {
                        req_col.def = QString::number(value.toInt());
                    }
                }
            }

            //if Alter
            if(state)
            {
                vec_NameValues.append(req_col.name);

                //if this column exists in origin list

                if(full_inf.db_column_inf.contains(req_col.name))
                {

                    Column_Inf col_inf = full_inf.db_column_inf.value(req_col.name);


                    if(col_inf.key == QLatin1String("PRI"))
                    {
                        qDebug() << "continue";
                        continue;
                    }

                    alterColumn(col_inf, req_col);
                }

                // Else adding column

                else if(req_col.type != QLatin1String("primary"))
                {

                    QString after;
                    query.append(alter);

                    if(vec_NameValues.size() > 1)
                    {
                        after = vec_NameValues.at(vec_NameValues.size() - 2);
                    }
                    else
                    {
                        if(vec_NameValues.size() == 1)
                        {
                            after = full_inf.first;
                        }

                    }

                    addColumn(req_col, after);
                }

            }
            else
            {
                createColumn(req_col);
            }

            // Adding foreign key if exists

            if(jobj.value(QStringLiteral("haveForeign")).toBool() &&
               !full_inf.vec_foreign.contains(req_col.name))
            {
                QString onTable = jobj.value(QStringLiteral("onTable")).toString();
                QString onColumnName = jobj.value(QStringLiteral("onColumnName")).toString();
                add_fk({req_col.name, onTable, onColumnName, req_col.isUnique});
            }
        }

    }


    /*
     * db_table         required_table
     *
     * id int           id int
     * first_name -X    last_name
     * last_name
     *
     * if there is no column in requierd_table, but there is in db_table, delete it
     */

    // if alter, delete colums
    if(state)
    {

        for(auto it = full_inf.db_column_inf.begin(); it != full_inf.db_column_inf.end(); it++)
        {
            if(!vec_NameValues.contains(it.key()))
            {
                QString key = it.value().key;

                if(key != QLatin1String("PRI"))
                {
                    if(full_inf.vec_foreign.contains(it.key()))
                    {
                        query.append(alter).append(QStringLiteral("DROP FOREIGN KEY fk_%1_%2;\n ")
                                                   .arg(it.key()).arg(tableName));
                    }

                    if(key == QLatin1String("UNI"))
                    {
                        query.append(alter).append(change_uni_index(it.key()));

                    }
                    else if(key == QLatin1String("MUL"))
                    {
                        query.append(alter).append(QStringLiteral("DROP INDEX `fk_%1_INDEX_%2`;\n ")
                                                   .arg(it.key()).arg(tableName));

                    }


                    query.append(alter).append(QStringLiteral("DROP COLUMN %1;\n ").arg(it.key()));
                }
            }
        }
    }

}

void DataBase::alterColumn(const Column_Inf &col_inf, Required_Col &req_col)
{

    //UNI -> common(DROP UNI);

    if( (col_inf.key == QLatin1String("UNI")) && !req_col.isUnique)
    {
        query.append(alter + change_uni_index(req_col.name));
    }


    // Check if type changed

    QString send;

    if(!col_inf.type.contains(req_col.type, Qt::CaseInsensitive))
    {

        bool withSize = false;

        qDebug() << "ORIGIN:" << req_col.name << req_col.type << req_col.size
                 << "default:" << req_col.def << "isUnique:" << req_col.isUnique;

        qDebug() << "COLUMN:" << req_col.name << col_inf.type
                 << "keyCol " << col_inf.key << "defCol " << col_inf.def;


        if(req_col.type == QLatin1String("varchar"))
        {
            if(req_col.size < default_size)
            {
                req_col.size = default_size;
            }

            withSize = true;
        }
        else if(req_col.type == QLatin1String("unsigned"))
        {
            req_col.type.prepend(QStringLiteral("int "));
        }


        if(withSize)
        {
            send.append(QStringLiteral("MODIFY COLUMN %1 %2(%3) ")
                        .arg(req_col.name)
                        .arg(req_col.type)
                        .arg(req_col.size));
        }
        else
        {
            send.append(QStringLiteral("MODIFY COLUMN %1 %2 ")
                        .arg(req_col.name)
                        .arg(req_col.type));
        }

        qDebug() << "DIFFERENT TYPE. It was = " << col_inf.type
                 << "Has become = " << req_col.type
                 << "with size" << withSize << req_col.size;


    }
    else
    {
        // Check if size changed

        if( (req_col.type == QLatin1String("varchar") ) &&
            (req_col.size != col_inf.length) )
        {
            if(req_col.size < default_size)
            {
                req_col.size = default_size;
            }

            send.append(QStringLiteral("MODIFY COLUMN %1 varchar(%2) ")
                        .arg(req_col.name).arg(req_col.size));

            qDebug() << "DIFFERENT SIZE It was = " << col_inf.type
                     << "has become = " << req_col.type << req_col.size;

        }

        // Check if default changed


        else
        {
            bool changed = false;

            if( (req_col.def != col_inf.def) )
            {
                changed = !req_col.isUnique && (req_col.type.toLower() != QLatin1String("timestamp") );
            }


            changed = changed + (req_col.isNullable != col_inf.isNullable);
            if(changed)
            {
                qDebug() << "DIFFERENT DEFAULT It was = " << col_inf.type << col_inf.def
                         << "isNullable" << col_inf.isNullable
                         << "has become = " << req_col.type << req_col.def
                         << "isNullable" << req_col.isNullable;

                send.append(QStringLiteral("MODIFY COLUMN %1 %2 ")
                            .arg(req_col.name).arg(col_inf.type));
            }
        }

    }


    if(!send.isEmpty())
    {

        addDefault(req_col, send);
        send.append(QStringLiteral(";\n "));

        query.append(alter).append(send);
        qDebug() << "SEND:" << send << endl;
    }

    //common -> UNI(Set default not null);

    if( (col_inf.key.isEmpty() || col_inf.key == "MUL") && req_col.isUnique)
    {
        query.append(alter).append(change_uni_index(req_col.name, true));
    }

}


void DataBase::addColumn(Required_Col &req_col, const QString &after)
{
    if(req_col.type == QLatin1String("varchar"))
    {
        if(req_col.type == QLatin1String("varchar"))
        {
            if(req_col.size < default_size)
            {
                req_col.size = default_size;
            }
        }

        query.append(QStringLiteral("ADD COLUMN %1 %2(%3) ")
                     .arg(req_col.name)
                     .arg(req_col.type)
                     .arg(req_col.size));
    }
    else
    {
        if(req_col.type == QLatin1String("unsigned"))
        {
            req_col.type.prepend(QStringLiteral("int "));
        }
        query.append(QStringLiteral("ADD COLUMN %1 %2 ").arg(req_col.name).arg(req_col.type));
    }

    qDebug() << "ADD COLUMN =  " << req_col.name << req_col.type << req_col.size;

    addDefault(req_col, query);

    query.append(QStringLiteral(" AFTER ")).append(after);

    if(!req_col.isUnique)
    {
        query.append(QStringLiteral(";\n "));
    }
    else
    {
        query.append(QStringLiteral(",\n "));
        query.append(change_uni_index(req_col.name, true));
    }
}


void DataBase::createColumn(Required_Col &req_col)
{
    if(req_col.type == QLatin1String("primary"))
    {
        add_id(req_col.name);

    }
    else if(req_col.type == QLatin1String("varchar"))
    {

        if(req_col.size < default_size)
        {
            req_col.size = default_size;
        }
        if(req_col.isUnique)
        {
            add_unique(req_col.name);
        }

        add_string(req_col.name, req_col.size, req_col.def);

    }
    else
    {
        if(req_col.isUnique)
        {
            add_unique(req_col.name);
        }

        if(req_col.type == QLatin1String("unsigned"))
        {
            req_col.type.prepend(QStringLiteral("int "));
        }


        query.append(req_col.name + QLatin1Char(' ')).append(req_col.type + QLatin1Char(' '));

        addDefault(req_col, query);


        query.append(QStringLiteral(",\n "));
    }

}
