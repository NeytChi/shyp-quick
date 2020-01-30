#include "foreignapihandler.h"

ForeignApiHandler::ForeignApiHandler()
{

}


std::unique_ptr<DistanceMatrixGoogle> ForeignApiHandler::distance_matrix_google = std::make_unique<DistanceMatrixGoogle>();
std::unique_ptr<FacebookOAuth> ForeignApiHandler::facebook_oauth = std::make_unique<FacebookOAuth>();
std::unique_ptr<GoogleOAuth> ForeignApiHandler::google_oauth = std::make_unique<GoogleOAuth>();
std::unique_ptr<ReverseGeocoding> ForeignApiHandler::reverse_geocoding = std::make_unique<ReverseGeocoding>();
std::unique_ptr<StripeApi> ForeignApiHandler::stripe = std::make_unique<StripeApi>();
std::unique_ptr<Twilio> ForeignApiHandler::twilio = std::make_unique<Twilio>();
