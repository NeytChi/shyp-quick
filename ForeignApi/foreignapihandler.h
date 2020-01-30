#ifndef FOREIGNAPIHANDLER_H
#define FOREIGNAPIHANDLER_H

#include <memory>

#include "ForeignApi/DistanceMatrixGoogle/distancematrixgoogle.h"
#include "ForeignApi/FacebookOAuth/facebookoauth.h"
#include "ForeignApi/GoogleOAuth/googleoauth.h"
#include "ForeignApi/ReverseGeocoding/reversegeocoding.h"
#include "ForeignApi/Stripe/stripeapi.h"
#include "ForeignApi/Twilio/twilio.h"


class ForeignApiHandler
{
public:
    ForeignApiHandler();


private:

    static std::unique_ptr<DistanceMatrixGoogle> distance_matrix_google;
    static std::unique_ptr<FacebookOAuth> facebook_oauth;
    static std::unique_ptr<GoogleOAuth> google_oauth;
    static std::unique_ptr<ReverseGeocoding> reverse_geocoding;
    static std::unique_ptr<StripeApi> stripe;
    static std::unique_ptr<Twilio> twilio;



public:

    static DistanceMatrixGoogle* getDistanceMatrixGoogle() { return distance_matrix_google.get(); }
    static FacebookOAuth* getFacebookOAuth() { return facebook_oauth.get(); }
    static GoogleOAuth* getGoogleOAuth() { return google_oauth.get(); }
    static ReverseGeocoding* getReverseGeocoding() { return reverse_geocoding.get(); }
    static StripeApi* getStripeApi() { return stripe.get(); }
    static Twilio* getTwilio() { return twilio.get(); }
};

#endif // FOREIGNAPIHANDLER_H
