#include "emailsender.h"

EmailSender::Email_Property EmailSender::email_property;

thread_local std::map<QTcpSocket*, EmailSender::Email> EmailSender::static_map_email;



EmailSender::EmailSender()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}

EmailSender::~EmailSender()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}



void EmailSender::setConfigs(const Email_Property &_email_property_)
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    email_property = _email_property_;
}



/*
 * Client -> Connection
 * Server start line ->  220 sr33.hostlife.net ESMTP Exim 4.91 Mon, 29 Oct 2018 15:38:13 +0100\r\n
 *
 * Client -> EHLO
 * Server settings ->
 *
 * {
 *      "250-sr33.hostlife.net Hello mail.----.---- \r\n"
        "250-SIZE 52428800\r\n"
        "250-8BITMIME\r\n"
        "250-PIPELINING\r\n"
        "250-AUTH PLAIN LOGIN\r\n"
        "250-STARTTLS\r\n"
        "250 HELP\r\n"
 *
 * }
 *
 * Client -> AUTH PLAIN LOGIN ->  login and pass to base64
 * Server -> 235 Authentication succeeded\r\n"
 *
 * Client -> Mail From
 * Server -> 250 OK\r\n
 *
 * Client -> RCPT TO
 * Server -> 250 Accepted\r\n
 *
 * Client -> Data
 * Server -> 354 Enter message, ending with \".\" on a line by itself\r\n
 *
 * Client -> msg -> end = .\r\n.\r\n
 * Server -> 250 OK id=1gH94G-008ufL-DR\r\n"
 *
 * Client -> Quit
 * Server -> 221 sr33.hostlife.net closing connection\r\n
 *
 *
*/





void EmailSender::sendEmail(EmailData &&email_data)
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";


    if(email_data.email_from.isEmpty())
    {
        email_data.email_from = email_property.email_from;
    }


    QSslSocket *socket = new QSslSocket();

    static_map_email.emplace
            (std::piecewise_construct,
             std::forward_as_tuple(socket),
             std::forward_as_tuple(Email{Stage::Ehlo, std::move(email_data)})
             );


    QObject::connect(socket, &QTcpSocket::readyRead, [socket] ()
    {
        qDebug() << "\r\n------ " << "EmailSender::QTcpSocket::readyRead()" << " ------\r\n";

        auto it_email = static_map_email.find(socket);

        if(it_email == static_map_email.end())
        {
            socket->close();
            socket->deleteLater();

            return;
        }

        Email &email = it_email->second;


        QByteArray data_to_write;

        QByteArray received_data;

        bool is_valid = socket->canReadLine();


        if(is_valid)
        {
            // Client -> Connection
            // Server start line -> 220 sr33.hostlife.net ESMTP Exim 4.91 Mon, 29 Oct 2018 15:38:13 +0100\r\n

            // Client -> EHLO smtp_server\r\n
            if(email.current_stage == Stage::Ehlo)
            {
                qDebug() << "Stage::Ehlo";

                is_valid = checkResponseCode(socket, Response_Code::Service_Ready, received_data);

                if(is_valid)
                {
                    data_to_write = "EHLO " + email_property.smtp_server.toUtf8();
                }
            }

            // Server settings ->
            // 250-sr33.hostlife.net Hello mail.----.---- \r\n
            // 250-SIZE 52428800\r\n
            // 250-8BITMIME\r\n
            // 250-PIPELINING\r\n
            // 250-AUTH PLAIN LOGIN\r\n
            // 250-STARTTLS\r\n
            // 250 HELP\r\n

            // Client -> AUTH PLAIN base64\r\n // base64 -> login and pass to base64
            else if(email.current_stage == Stage::Auth_Plain)
            {
                qDebug() << "Stage::Auth_Plain";

                while(socket->canReadLine() && is_valid)
                {
                    is_valid = checkResponseCode(socket, Response_Code::Command_Success, received_data);
                }

                if(is_valid)
                {
                    const QByteArray login_pass = QByteArray().append((char) 0).append(email_property.email_login)
                                                  .append((char) 0).append(email_property.email_password).toBase64();

                    data_to_write = "AUTH PLAIN " + login_pass;
                }
            }

            // Server -> 235 Authentication succeeded\r\n"
            // Client -> MAIL FROM: <email@gmail.com>\r\n
            else if(email.current_stage == Stage::Mail_From)
            {
                qDebug() << "Stage::Mail_From";

                is_valid = checkResponseCode(socket, Response_Code::Auth_Success, received_data);

                if(is_valid)
                {
                    data_to_write = "MAIL FROM: <" + email_property.email_from + ">";
                }
            }

            // Server -> 250 OK\r\n
            // Client -> RCPT TO: <email@gmail.com>\r\n
            else if(email.current_stage == Stage::Rcpt_To)
            {
                qDebug() << "Stage::Rcpt_To";

                is_valid = checkResponseCode(socket, Response_Code::Command_Success, received_data);

                if(is_valid)
                {
                    data_to_write = "RCPT TO: <" + email.email_data.email_to + ">";
                }
            }

            // Server -> 250 Accepted\r\n
            // Client -> DATA\r\n
            else if(email.current_stage == Stage::Data)
            {
                qDebug() << "Stage::Data";

                is_valid = checkResponseCode(socket, Response_Code::Command_Success, received_data);

                if(is_valid)
                {
                    data_to_write = "DATA";
                }
            }

            // Server -> 354 Enter message, ending with \".\" on a line by itself\r\n
            // Client -> message -> end = \r\n.\r\n
            else if(email.current_stage == Stage::Email_Body)
            {
                qDebug() << "Stage::Email_Body";

                is_valid = checkResponseCode(socket, Response_Code::Start_Message, received_data);

                if(is_valid)
                {
                    data_to_write = email.email_data.getData();
                }
            }

            // Server -> 250 OK id=1gH94G-008ufL-DR\r\n"
            // Client -> QUIT\r\n
            else if(email.current_stage == Stage::Quit)
            {
                qDebug() << "Stage::Quit";

                is_valid = checkResponseCode(socket, Response_Code::Command_Success, received_data);

                if(is_valid)
                {
                    data_to_write = "QUIT";
                }
            }

            // Server -> 221 sr33.hostlife.net closing connection\r\n
            else if(email.current_stage == Stage::Close_Connection) // remove socket from static_map
            {
                qDebug() << "Stage::Close_Connection";

                qDebug() << socket->readLine();

                socket->close();
                socket->deleteLater();

                static_map_email.erase(socket);

                return;
            }//
        }


        if(is_valid)
        {
            data_to_write.append("\r\n");

            socket->write(data_to_write);
            socket->flush();

            email.current_stage = static_cast<Stage>(static_cast<std::underlying_type<Stage>::type>(email.current_stage) + 1);
        }
        else // remove socket from static_map
        {
            qDebug() << "Email Sender Error Occurred";

            const QJsonObject error_description =
            {
                {"caller_url", QString::fromUtf8(email.email_data.caller_url)},
                {"email_from", QString::fromUtf8(email.email_data.email_from)},
                {"email_to", QString::fromUtf8(email.email_data.email_to)},
                {"received_data", QString::fromUtf8(received_data + socket->readAll())}
            };

            Logger::writeLogInFile(error_description, __PRETTY_FUNCTION__);

            socket->close();
            socket->deleteLater();

            static_map_email.erase(socket);
        }
    });


    QObject::connect(socket, &QSslSocket::disconnected, [socket]()
    {
        qDebug() << "Disconnected email sender";

        socket->deleteLater();
    });

    QObject::connect(socket, &QSslSocket::stateChanged, [&](QAbstractSocket::SocketState state)
    {
        qDebug() << "Email Sender State = " << state;
    });

    QObject::connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), [&](QAbstractSocket::SocketError socketError)
    {
        qDebug() << "Email Sender Socket Error = " <<  socketError;
    });

    QObject::connect(socket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
          [=](const QList<QSslError> &errors)
    {
        qDebug() << "SSL error = " << errors;
    });

    socket->connectToHostEncrypted(email_property.smtp_server, email_property.smtp_port);
}


// https://github.com/bluetiger9/SmtpClient-for-Qt/blob/v1.1/src/smtpclient.cpp
// https://stackoverflow.com/questions/10873269/correct-email-format-for-email-with-plain-html-and-an-attachment-in-smtp
// https://morf.lv/adding-file-attachments-to-smtp-client-for-qt5





void EmailSender::sendHashCode(const QByteArray &email_to, const Type_Hash_Code type_hash_code, const QByteArray &hash_code, const QByteArray &caller_url)
{
    QByteArray subject;

    if(type_hash_code == Type_Hash_Code::Account_Activation)
    {
        subject = "Account Activation";
    }
    else if(type_hash_code == Type_Hash_Code::Recovery_Password)
    {
        subject = "Recovery Password";
    }

    EmailData email_data(email_to, subject, caller_url);

    {
        QByteArray text = QByteArrayLiteral("Your code:\n") + hash_code;

        email_data.addText(std::move(text));
    }

    sendEmail(std::move(email_data));
}

void EmailSender::sendContactUs(EmailData &&email_data)
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    sendEmail(std::move(email_data));
}


EmailData::EmailData(const QByteArray &_email_to_, const QByteArray &_subject_, const QByteArray &_caller_url_)
    : email_to(_email_to_),
      subject(_subject_),
      caller_url(_caller_url_)
{

}

void EmailData::setEmailFrom(const QByteArray &_email_from_)
{
    email_from = _email_from_;
}


/*
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: quoted-printable

here goes the text message
*/
void EmailData::addText(QByteArray &&text)
{
    const QByteArray line_break = "\r\n";

    QByteArray part = "Content-Type: text/plain; charset=UTF-8" + line_break;

    part.append("Content-Transfer-Encoding: quoted-printable" + line_break + line_break);


    const auto body_text = std::move(text);

    part.append(body_text).append(line_break);

    vec_message_part.append(std::move(part));
}



/*
Content-Type: text/plain;
Content-Transfer-Encoding: base64
Content-Disposition: attachment; filename="Test.txt"

KiAxMyBGRVRDSC // file converted to base64
*/
void EmailData::addFile(const QMimeType &mime_type, const QByteArray &file_name, QByteArray &&file_data)
{
    const QByteArray line_break = "\r\n";


    QByteArray part = "Content-Type: ";

    if(!mime_type.isValid())
    {
        part.append("application/octet-stream");
    }
    else
    {
        part.append(mime_type.name().toUtf8());
    }

    part.append(";" + line_break);

    part.append("Content-Transfer-Encoding: base64" + line_break);

    part.append("Content-Disposition: attachment; filename=\"" + file_name + "\"" + line_break + line_break);


    const auto body_file = std::move(file_data);

    part.append(body_file.toBase64()).append(line_break);

    vec_message_part.append(std::move(part));
}


/* Simple
From: Some User <someusername@somecompany.com>
To: User1 <user1@company.com>
Subject: subject
Content-Type: text/plain

Hi!
.
*/


/* Multipart
From: "Edited Out" <editedout@yahoo.com>
To: "Edited Out" <editedout@yahoo.com>
Subject: Testing 4
MIME-Version: 1.0
Content-Type: multipart/alternative; boundary=Asrf456sasxexbulkGe4h // -> multipart/mixed

--Asrf456sasxexbulkGe4h
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: quoted-printable

here goes the text message

--Asrf456sasxexbulkGe4h
Content-Transfer-Encoding: base64
Content-Type: text/plain; //name="Test.txt"
Content-Disposition: attachment; filename="Test.txt"

KiAxMyBGRVRDSC // file converted to base64
--Asrf456sasxexbulkGe4h--
CRLF.CRLF

*/

QByteArray EmailData::getData()
{
    const QByteArray line_break = "\r\n";

    QByteArray data;

    data = "From: " + email_from + line_break;

    data.append("To: ").append(email_to).append(line_break);

    data.append("Subject: ").append(subject).append(line_break);

    data.append("MIME-Version: 1.0" + line_break);

    if(vec_message_part.size() > 1)
    {
        const QByteArray boundary = "Asrf456sasxexbulkGe4h";

        data.append("Content-Type: multipart/mixed; boundary=" + boundary + line_break + line_break);

        for(auto it = vec_message_part.begin(); it != vec_message_part.end();)
        {
            data.append("--" + boundary + line_break);

            data.append(vec_message_part.takeFirst());

            if(it != vec_message_part.end())
            {
                data.append(line_break);
            }
            else
            {
                data.append("--" + boundary + "--" + line_break);
            }
        }
    }
    else if(!vec_message_part.isEmpty())
    {
        data.append(vec_message_part.takeFirst());
    }

    data.append("."); // we doesn't add line break here because it appends to the end in sendEmail()

    return data;
}









