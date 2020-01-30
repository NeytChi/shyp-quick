#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include <QObject>
#include <QDebug>
#include <QSslSocket>
#include <QMimeType>

#include "Global/logger.h"



class EmailSender;

// EmailData only moveable, because it may contain files and etc.
class EmailData
{
public:

    EmailData(const QByteArray &_email_to_, const QByteArray &_subject_, const QByteArray &_caller_url_);


    void setEmailFrom(const QByteArray &_email_from_);

    void addText(QByteArray &&text);

    void addFile(const QMimeType &mime_type, const QByteArray &file_name, QByteArray &&file_data);

private:

    EmailData(const EmailData&) = delete;

    EmailData& operator= (const EmailData&) = delete;


    EmailData(EmailData &&other) = default;

    EmailData& operator= (EmailData &&other) = default;


    QByteArray email_from;
    const QByteArray email_to;
    const QByteArray subject;
    const QByteArray caller_url;

    QVector<QByteArray> vec_message_part; // Vector of attachments


    QByteArray getData();


    friend class EmailSender;
};





class EmailSender
{

public:

    explicit EmailSender();
    ~EmailSender();

    struct Email_Property
    {
        QByteArray email_login;
        QByteArray email_password;

        QByteArray email_from;
        QString smtp_server;
        quint16 smtp_port;
    };

private:

    enum class Response_Code : int
    {
        Service_Ready = 220,
        Command_Success = 250,
        Auth_Success = 235,
        Start_Message = 354
    };


    static Email_Property email_property;


    enum class Stage : unsigned char // Order is important
    {
        Ehlo,
        Auth_Plain,
        Mail_From,
        Rcpt_To,
        Data,
        Email_Body,
        Quit,
        Close_Connection
    };

    static bool checkResponseCode(QTcpSocket* socket, const Response_Code response_code, QByteArray &received_data)
    {
        received_data = socket->readLine();

        return (received_data.left(3).toInt() == static_cast<std::underlying_type<Response_Code>::type>(response_code));
    }


    struct Email
    {
        Stage current_stage;
        EmailData email_data;
    };

    // For saving stage and email body between the stages of server and client communication.
    // Because communication is asynchronous.
    static thread_local std::map<QTcpSocket*, Email> static_map_email; // Must not call remove key before close connection


    static void sendEmail(EmailData &&email_data);

public:


    static QByteArray getEmailFrom() { return email_property.email_from; }

    static void setConfigs(const Email_Property &_email_property_);


    enum class Type_Hash_Code : unsigned char
    {
        Account_Activation,
        Recovery_Password
    };

    static void sendHashCode(const QByteArray &email_to, const Type_Hash_Code type_hash_code, const QByteArray &hash_code, const QByteArray &caller_url);

    static void sendContactUs(EmailData &&email_data);


};

#endif // EMAILSENDER_H
