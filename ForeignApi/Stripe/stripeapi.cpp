#include "stripeapi.h"


StripeApi::StripeApi() : AbstractForeignApi ()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}


StripeApi::~StripeApi()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";
}

bool StripeApi::initApi()
{
    qDebug() << "\r\n------ " << __PRETTY_FUNCTION__ << " ------\r\n";

    const QSet<QString> set_need_config =
    {
        "secret_key",

        "create_customer_url",
        "create_account_url",
        "create_charge_url"
    };

    const auto [is_init, map_received_config] = readConfigFile("stripe", set_need_config);

    if(is_init)
    {
        stripeApi_settings.secret_key = map_received_config.value("secret_key").toUtf8();

        stripeApi_settings.create_customer_url = map_received_config.value("create_customer_url");
        stripeApi_settings.create_account_url = map_received_config.value("create_account_url");
        stripeApi_settings.create_charge_url = map_received_config.value("create_charge_url");



        manager = new QNetworkAccessManager;


        default_request.setRawHeader("Authorization", "Bearer " + stripeApi_settings.secret_key);
        default_request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

        header_request = default_request;


        state = State::Ready;
    }

    return is_init;
}

void StripeApi::createCustomer(QString &&email, QString &&name, Response_Function &&response_function)
{
    auto calling_function = [this, email = std::move(email), name = std::move(name)]()
    {
        header_request.setUrl(stripeApi_settings.create_customer_url);

        QUrlQuery url_query;
        url_query.addQueryItem(QStringLiteral("email"), email);
        url_query.addQueryItem(QStringLiteral("name"), name);

        current_reply = manager->post(header_request, url_query.toString(QUrl::FullyDecoded).toUtf8());

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
  "id": "cus_FwmUnBLK1pXIKm",
  "object": "customer",
  ...
}
*/

void StripeApi::createAccount(Account &&account, Response_Function &&response_function)
{
    auto calling_function = [this, account = std::move(account)]()
    {
        header_request.setUrl(stripeApi_settings.create_account_url);


        QUrlQuery url_query;
        url_query.addQueryItem(QStringLiteral("type"), "custom");
        url_query.addQueryItem(QStringLiteral("country"), "US");
        url_query.addQueryItem(QStringLiteral("email"), account.email);
        url_query.addQueryItem(QStringLiteral("requested_capabilities[]"), "transfers");
        url_query.addQueryItem("business_type", "individual");
        url_query.addQueryItem("individual[first_name]", account.first_name);
        url_query.addQueryItem("individual[last_name]", account.last_name);
        url_query.addQueryItem("individual[email]", account.email);
        url_query.addQueryItem("tos_acceptance[date]", QString::number(QDateTime::currentSecsSinceEpoch()));
        url_query.addQueryItem("tos_acceptance[ip]", account.ip_address);
        url_query.addQueryItem("settings[payouts[schedule[interval]]]", "manual");
        url_query.addQueryItem("business_profile[product_description]", "ShypQuick");

        current_reply = manager->post(header_request, url_query.toString(QUrl::FullyDecoded).toUtf8());

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
  "id": "acct_1032D82eZvKYlo2C",
  "object": "account",
  ...
}
*/

void StripeApi::createCardToCustomer(QString &&customer_id, QString &&card_token, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, customer_id = std::move(customer_id), card_token = std::move(card_token)]()
    {
        // https://api.stripe.com/v1/customers/<customer_id>/sources

        header_request.setUrl("https://api.stripe.com/v1/customers/" + customer_id + "/sources");

        QUrlQuery url_query;
        url_query.addQueryItem(QStringLiteral("source"), card_token);


        current_reply = manager->post(header_request, url_query.toString(QUrl::FullyDecoded).toUtf8());

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
  "id": "card_1FVwKT2eZvKYlo2ClWewS0dw",
  "object": "card",
  ...
  "brand": "Visa",
  "country": "US",
  "customer": "cus_G20KGnKmyy20CV",
  "cvc_check": null,
  "dynamic_last4": null,
  "exp_month": 8,
  "exp_year": 2020,
  "fingerprint": "Xt5EWLLDS7FJjR1c",
  "funding": "credit",
  "last4": "4242",
  "metadata": {},
  "name": null,
  "tokenization_method": null
}

*/

void StripeApi::deleteCardFromCustomer(QString &&customer_id, QString &&card_token, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, customer_id = std::move(customer_id), card_token = std::move(card_token)]()
    {
        // https://api.stripe.com/v1/customers/<customer_id>/sources/<card_token>

        header_request.setUrl("https://api.stripe.com/v1/customers/" + customer_id + "/sources/" + card_token);

        current_reply = manager->deleteResource(header_request);

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response

{
  "id": "card_1FZeh6D4UBVko0kOFL48FfrH",
  "object": "card",
  "deleted": true
}

*/

void StripeApi::createCardToAccount(QString &&account_id, QString &&card_token, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, account_id = std::move(account_id), card_token = std::move(card_token)]()
    {
        // https://api.stripe.com/v1/accounts/<account_id>/external_accounts

        header_request.setUrl("https://api.stripe.com/v1/accounts/" + account_id + "/external_accounts");

        QUrlQuery url_query;
        url_query.addQueryItem(QStringLiteral("external_account"), card_token);
        url_query.addQueryItem("default_for_currency", "true");


        current_reply = manager->post(header_request, url_query.toString(QUrl::FullyDecoded).toUtf8());

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
  "id": "card_1FVwKT2eZvKYlo2ClWewS0dw",
  "object": "card",
  ...
  "brand": "Visa",
  "country": "US",
  "cvc_check": null,
  "dynamic_last4": null,
  "exp_month": 8,
  "exp_year": 2020,
  "fingerprint": "Xt5EWLLDS7FJjR1c",
  "funding": "credit",
  "last4": "4242",
  "metadata": {},
  "name": null,
  "tokenization_method": null,
  "account": "acct_1032D82eZvKYlo2C"
}

*/

void StripeApi::deleteCardFromAccount(QString &&account_id, QString &&card_token, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, account_id = std::move(account_id), card_token = std::move(card_token)]()
    {
        // https://api.stripe.com/v1/accounts/<account_id>/external_accounts/<card_token>

        header_request.setUrl("https://api.stripe.com/v1/accounts/" + account_id + "/external_accounts/" + card_token);

        current_reply = manager->deleteResource(header_request);

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response

{
  "id": "card_1FZeh6D4UBVko0kOFL48FfrH",
  "object": "card",
  "deleted": true
}

*/

void StripeApi::authorizeCharge(Charge &&charge, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, charge = std::move(charge)]()
    {
        header_request.setUrl(stripeApi_settings.create_charge_url);

        QUrlQuery url_query;
        url_query.addQueryItem(QStringLiteral("amount"), QString::number(Global::roundDoubleToTwoPointsDecimal(charge.amount_dollars) * 100.0));
        url_query.addQueryItem(QStringLiteral("currency"), "usd");
        url_query.addQueryItem(QStringLiteral("customer"), charge.customer_id);
        url_query.addQueryItem(QStringLiteral("capture"), "false");
        url_query.addQueryItem("source", charge.card_token);


        current_reply = manager->post(header_request, url_query.toString(QUrl::FullyDecoded).toUtf8());

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
  "id": "ch_1DHXXVD4UBVko0kONL9b3oYg",
  "object": "charge",
  "amount": 2000,
  "amount_refunded": 0,
  "application": null,
  ...
}

*/

void StripeApi::captureCharge(QString &&charge_id, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, charge_id = std::move(charge_id)]()
    {
        // https://api.stripe.com/v1/charges/<charge_id>/capture
        header_request.setUrl(QUrl("https://api.stripe.com/v1/charges/" + charge_id + "/capture"));

        current_reply = manager->post(header_request, "");

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
  "id": "ch_1DHXXVD4UBVko0kONL9b3oYg",
  "object": "charge",
  "amount": 999,
  "amount_refunded": 0,
  "application": null,
  "application_fee": null,
  ...
}

*/

void StripeApi::refundUncapturedCharge(QString &&charge_id, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, charge_id = std::move(charge_id)]()
    {
        header_request.setUrl(QUrl("https://api.stripe.com/v1/refunds"));


        QUrlQuery url_query;
        url_query.addQueryItem(QStringLiteral("charge"), charge_id);

        current_reply = manager->post(header_request, url_query.toString(QUrl::FullyDecoded).toUtf8());

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

void StripeApi::retreiveCharge(QString &&charge_id, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, charge_id = std::move(charge_id)]()
    {
        // https://api.stripe.com/v1/charges/ch_1DHXXVD4UBVko0kONL9b3oYg
        header_request.setUrl(QUrl("https://api.stripe.com/v1/charges/" + charge_id));

        current_reply = manager->get(header_request);

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
  "id": "ch_1DHXXVD4UBVko0kONL9b3oYg",
  "object": "charge",
  "amount": 999,
  "amount_refunded": 0,
  "application": null,
  ...
  "captured": true,
  "created": 1538662253,
  "currency": "usd",
  "customer": null,
  ...
}

*/

void StripeApi::transferMoneyToAnotherAccount(Transfer &&transfer, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, transfer = std::move(transfer)]()
    {
        header_request.setUrl(QUrl("https://api.stripe.com/v1/transfers"));

        QUrlQuery url_query;
        url_query.addQueryItem(QStringLiteral("amount"), QString::number(Global::roundDoubleToTwoPointsDecimal(transfer.amount_dollars) * 100.0));
        url_query.addQueryItem(QStringLiteral("currency"), "usd");
        url_query.addQueryItem(QStringLiteral("destination"), transfer.another_account_id);

        if(!transfer.description.isEmpty())
        {
            url_query.addQueryItem(QStringLiteral("description"), transfer.description);
        }

        if(!transfer.charge_id.isEmpty())
        {
            url_query.addQueryItem(QStringLiteral("source_transaction"), transfer.charge_id);
        }

        current_reply = manager->post(header_request, url_query.toString(QUrl::FullyDecoded).toUtf8());

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
  "id": "tr_1FYuRVD4UBVko0kOjyQdkuVO",
  "object": "transfer",
  "amount": 1100,
  "amount_reversed": 0,
  "balance_transaction": "txn_1DHXXVD4UBVko0kOXpyiknsk",
  ...
  "source_type": "card",
  "transfer_group": null
}

*/

void StripeApi::getBalanceFromAccount(QString &&account_id, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, account_id = std::move(account_id)]()
    {
        header_request.setUrl(QUrl("https://api.stripe.com/v1/balance"));

        header_request.setRawHeader("Stripe-Account", account_id.toUtf8());

        current_reply = manager->get(header_request);

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}

/* Response:

{
  "object": "balance",
  "available": [
    {
      "amount": 1880,
      "currency": "usd",
      "source_types": {
        "card": 1880
      }
    }
  ],
  "connect_reserved": [
    {
      "amount": 0,
      "currency": "usd"
    }
  ],
  "livemode": false,
  "pending": [
    {
      "amount": 0,
      "currency": "usd",
      "source_types": {
        "card": 0
      }
    }
  ]
}

*/

void StripeApi::sendFundsToDebitCard(Payout &&payout, AbstractForeignApi::Response_Function &&response_function)
{
    auto calling_function = [this, payout = std::move(payout)]()
    {
        header_request.setUrl(QUrl("https://api.stripe.com/v1/payouts"));

        header_request.setRawHeader("Stripe-Account", payout.account_id.toUtf8());

        QUrlQuery url_query;
        url_query.addQueryItem(QStringLiteral("amount"), QString::number(payout.amount_cents));
        url_query.addQueryItem(QStringLiteral("currency"), "usd");
        url_query.addQueryItem(QStringLiteral("destination"), payout.debit_card_id);
        url_query.addQueryItem("source_type", "card");


        current_reply = manager->post(header_request, url_query.toString(QUrl::FullyDecoded).toUtf8());

        connectReplyToSlots();
    };

    addRequest({calling_function, std::move(response_function), __PRETTY_FUNCTION__});
}



