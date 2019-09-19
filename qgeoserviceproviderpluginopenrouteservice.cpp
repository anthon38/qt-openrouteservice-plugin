/***************************************************************************************
 *  Copyright (c) 2019 Anthony Vital <anthony.vital@gmail.com>                         *
 *                                                                                     *
 *  This file is part of qt-openrouteservice-plugin.                                   *
 *                                                                                     *
 *  qt-openrouteservice-plugin is free software: you can redistribute it and/or modify *
 *  it under the terms of the GNU General Public License as published by               *
 *  the Free Software Foundation, either version 3 of the License, or                  *
 *  (at your option) any later version.                                                *
 *                                                                                     *
 *  qt-openrouteservice-plugin is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                     *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                      *
 *  GNU General Public License for more details.                                       *
 *                                                                                     *
 *  You should have received a copy of the GNU General Public License                  *
 *  along with qt-openrouteservice-plugin. If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************************/

#include "qgeoserviceproviderpluginopenrouteservice.h"
#include "qgeocodingmanagerengineopenrouteservice.h"
#include "qgeoroutingmanagerengineopenrouteservice.h"

QT_BEGIN_NAMESPACE

static inline QString msgAccessTokenParameter()
{
    return QGeoServiceProviderFactoryOpenrouteservice::tr("OpenRouteService plugin requires an 'ors.api_key' parameter.\n"
                                                          "Please visit https://openrouteservice.org");
}

QGeoCodingManagerEngine *QGeoServiceProviderFactoryOpenrouteservice::createGeocodingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    const QString accessToken = parameters.value(QStringLiteral("ors.api_key")).toString();

    if (!accessToken.isEmpty()) {
        return new QGeoCodingManagerEngineOpenrouteservice(parameters, error, errorString);
    } else {
        *error = QGeoServiceProvider::MissingRequiredParameterError;
        *errorString = msgAccessTokenParameter();
        return nullptr;
    }
}

QGeoMappingManagerEngine *QGeoServiceProviderFactoryOpenrouteservice::createMappingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    Q_UNUSED(parameters)
    Q_UNUSED(error)
    Q_UNUSED(errorString)

    return nullptr;
}

QGeoRoutingManagerEngine *QGeoServiceProviderFactoryOpenrouteservice::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
     const QString accessToken = parameters.value(QStringLiteral("ors.api_key")).toString();

     if (!accessToken.isEmpty()) {
         return new QGeoRoutingManagerEngineOpenrouteservice(parameters, error, errorString);
     } else {
         *error = QGeoServiceProvider::MissingRequiredParameterError;
         *errorString = msgAccessTokenParameter();
         return nullptr;
     }
}

QPlaceManagerEngine *QGeoServiceProviderFactoryOpenrouteservice::createPlaceManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    Q_UNUSED(parameters)
    Q_UNUSED(error)
    Q_UNUSED(errorString)

    return nullptr;
}

QT_END_NAMESPACE
