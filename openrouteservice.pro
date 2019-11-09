TARGET = qtgeoservices_openrouteservice

QT += location positioning network

HEADERS += \
    qgeocodereplyopenrouteservice.h \
    qgeocodingmanagerengineopenrouteservice.h \
    qgeoroutereplyopenrouteservice.h \
    qgeoroutingmanagerengineopenrouteservice.h \
    qgeoserviceproviderpluginopenrouteservice.h

SOURCES += \
    qgeocodereplyopenrouteservice.cpp \
    qgeocodingmanagerengineopenrouteservice.cpp \
    qgeoroutereplyopenrouteservice.cpp \
    qgeoroutingmanagerengineopenrouteservice.cpp \
    qgeoserviceproviderpluginopenrouteservice.cpp

OTHER_FILES += \
    openrouteservice_plugin.json

PLUGIN_TYPE = geoservices
PLUGIN_CLASS_NAME = QGeoServiceProviderFactoryOpenrouteservice
load(qt_plugin)
