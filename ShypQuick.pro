QT -= gui
QT += core network sql
CONFIG += c++17 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_DEBUG_OUTPUT
#QMAKE_CXXFLAGS += -O3

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Checker/checker.cpp \
    ForeignApi/DistanceMatrixGoogle/distancematrixgoogle.cpp \
    ForeignApi/FacebookOAuth/facebookoauth.cpp \
    ForeignApi/GoogleOAuth/googleoauth.cpp \
    ForeignApi/ReverseGeocoding/reversegeocoding.cpp \
    ForeignApi/Stripe/stripeapi.cpp \
    ForeignApi/Twilio/twilio.cpp \
    ForeignApi/abstractforeignapi.cpp \
    ForeignApi/foreignapihandler.cpp \
    Global/databaseconnector.cpp \
    Global/global.cpp \
    Global/logger.cpp \
    Json/json.cpp \
    EmailSender/emailsender.cpp \
    PushNotifications/Pusher/abstractpusher.cpp \
    PushNotifications/Pusher/iospusher.cpp \
    PushNotifications/pusherhandler.cpp \
    RequestBody/requestbody.cpp \
    RequestHandler/Order/order.cpp \
    RequestHandler/User/user.cpp \
    RequestHandler/abstractrequesthandler.cpp \
    Response/response.cpp \
    SqlBuilder/sqlbuilder.cpp \
    Response/ErrorResponse/errorresponse.cpp \
    UserData/userdata.cpp \
        main.cpp \
    clientsocket.cpp \
    database.cpp \
    myserver.cpp \
    validator.cpp \
    worker.cpp \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Checker/checker.h \
    Checker/checker.tpp \
    ForeignApi/DistanceMatrixGoogle/distancematrixgoogle.h \
    ForeignApi/FacebookOAuth/facebookoauth.h \
    ForeignApi/GoogleOAuth/googleoauth.h \
    ForeignApi/ReverseGeocoding/reversegeocoding.h \
    ForeignApi/Stripe/stripeapi.h \
    ForeignApi/Twilio/twilio.h \
    ForeignApi/abstractforeignapi.h \
    ForeignApi/foreignapihandler.h \
    Global/databaseconnector.h \
    Global/global.h \
    Global/logger.h \
    Json/json.h \
    Json/json.tpp \
    EmailSender/emailsender.h \
    PushNotifications/Pusher/abstractpusher.h \
    PushNotifications/Pusher/iospusher.h \
    PushNotifications/pusherhandler.h \
    RequestBody/requestbody.h \
    RequestHandler/Order/order.h \
    RequestHandler/User/user.h \
    RequestHandler/abstractrequesthandler.h \
    Response/response.h \
    SqlBuilder/sqlbuilder.h \
    SqlBuilder/sqlbuilder.tpp \
    Response/ErrorResponse/errorresponse.h \
    UserData/userdata.h \
    clientsocket.h \
    database.h \
    myserver.h \
    validator.h \
    worker.h \
    url_list.h

DISTFILES += \
    ServerConfigs.conf
