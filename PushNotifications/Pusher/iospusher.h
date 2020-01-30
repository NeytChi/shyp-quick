#ifndef IOSPUSHER_H
#define IOSPUSHER_H



#include "abstractpusher.h"


class IosPusher : public AbstractPusher
{
    Q_OBJECT

public:
    IosPusher(const QSqlDatabase &data_base, const QString _connection_db_name_);

    virtual ~IosPusher() override;


    virtual bool initializePusher() override;

    void terminate();

private:

    virtual void sendPush() override;

    virtual void handleResponse() override;

signals:
    void finished();

};

#endif // IOSPUSHER_H
