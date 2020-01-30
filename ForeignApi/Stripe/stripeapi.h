#ifndef STRIPEAPI_H
#define STRIPEAPI_H

#include "ForeignApi/abstractforeignapi.h"


class StripeApi : AbstractForeignApi
{
public:
    StripeApi();

    ~StripeApi();


private:

    struct StripeApi_Settings
    {
        QByteArray secret_key;

        QString create_customer_url;

        QString create_account_url;

        QString create_charge_url;

    } stripeApi_settings;


    bool initApi();



public:

    void createCustomer(QString &&email, QString &&name, Response_Function &&response_function);

    struct Account
    {
        QString email;
        QString first_name;
        QString last_name;
        QString ip_address;
    };

    void createAccount(Account &&account, Response_Function &&response_function);

    void createCardToCustomer(QString &&customer_id, QString &&card_token, Response_Function &&response_function);

    void deleteCardFromCustomer(QString &&customer_id, QString &&card_token, Response_Function &&response_function);

    void createCardToAccount(QString &&account_id, QString &&card_token, Response_Function &&response_function);

    void deleteCardFromAccount(QString &&account_id, QString &&card_token, Response_Function &&response_function);

    struct Charge
    {
        QString customer_id;
        double amount_dollars;
        QString card_token;
    };

    void authorizeCharge(Charge &&charge, Response_Function &&response_function);

    void captureCharge(QString &&charge_id, Response_Function &&response_function);

    void refundUncapturedCharge(QString &&charge_id, Response_Function &&response_function);

    void retreiveCharge(QString &&charge_id, Response_Function &&response_function);

    struct Transfer
    {
        double amount_dollars;
        QString another_account_id;
        QString description;
        QString charge_id;
    };

    void transferMoneyToAnotherAccount(Transfer &&transfer, Response_Function &&response_function);


    void getBalanceFromAccount(QString &&account_id, Response_Function &&response_function);


    struct Payout
    {
        int amount_cents;
        QString debit_card_id;
        QString account_id;
    };

    void sendFundsToDebitCard(Payout &&payout, Response_Function &&response_function);


    friend class Worker;

    friend class ForeignApiHandler;

    friend class AbstractForeignApi;

    friend class MyServer;
};

#endif // STRIPEAPI_H
