#ifndef USERDATA_H
#define USERDATA_H


#include "SqlBuilder/sqlbuilder.h"

#include "url_list.h"


class UserData
{
public:
    UserData();

    ~UserData();

    inline UserData(UserData &&other) noexcept;

    UserData& operator= (UserData &&other) noexcept;


private:

    UserData(const UserData&) = delete;

    UserData& operator = (const UserData&) = delete;


    int user_id{};

    QString auth_token;

    bool is_driver = false;


public:


    int user_Id() const { return user_id; }

    const QString& user_Token() const { return auth_token; }

    bool isDriver() const { return is_driver; }


    /*!
     * \return is_auth, is_have_access, auth_error
     */
    std::tuple<bool, bool, QString>
    checkAuth(const QString &auth_token, SqlBuilder &sql_builder, const unsigned short url_index);

    bool validateAccess(const unsigned short url_index);
};

#endif // USERDATA_H
