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

#include "qgeoroutingmanagerengineopenrouteservice.h"
#include "qgeoroutereplyopenrouteservice.h"

#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

QT_BEGIN_NAMESPACE

QGeoRoutingManagerEngineOpenrouteservice::QGeoRoutingManagerEngineOpenrouteservice(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString)
    : QGeoRoutingManagerEngine(parameters),
      m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("ors.api_key"))) {
        m_accessToken = parameters.value(QStringLiteral("ors.api_key")).toString();
    }

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoRoutingManagerEngineOpenrouteservice::~QGeoRoutingManagerEngineOpenrouteservice()
{
}

QGeoRouteReply* QGeoRoutingManagerEngineOpenrouteservice::calculateRoute(const QGeoRouteRequest &request)
{
    QString url = QStringLiteral("https://api.openrouteservice.org/v2/directions/");
    QGeoRouteRequest::TravelModes travelModes = request.travelModes();
    if (travelModes.testFlag(QGeoRouteRequest::PedestrianTravel)) {
        url += QStringLiteral("foot-hiking");
    } else if (travelModes.testFlag(QGeoRouteRequest::BicycleTravel)) {
        url += QStringLiteral("cycling-regular");
    } else if (travelModes.testFlag(QGeoRouteRequest::CarTravel)) {
        url += QStringLiteral("driving-car");
    } else if (travelModes.testFlag(QGeoRouteRequest::TruckTravel)) {
        url += QStringLiteral("driving-hgv");
    } else {
        url += QStringLiteral("driving-car");
        qWarning()<<"Undefined or unimplemented QGeoRouteRequest::TravelMode. Defaulting to QGeoRouteRequest::CarTravel.";
    }
    url += QStringLiteral("/json");

    QNetworkRequest networkRequest;
    networkRequest.setUrl(QUrl(url));
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkRequest.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json, application/geo+json, application/gpx+xml, img/png; charset=utf-8"));
    networkRequest.setRawHeader(QByteArrayLiteral("Authorization"), m_accessToken.toUtf8());

    QJsonObject geojson;

    QJsonArray coordinates;
    for (const auto &c : request.waypoints()) {
        QJsonArray waypoint;
        waypoint.append(c.longitude());
        waypoint.append(c.latitude());
        coordinates.append(waypoint);
    }
    geojson.insert(QStringLiteral("coordinates"), coordinates);

    geojson.insert(QStringLiteral("elevation"), true);
    geojson.insert(QStringLiteral("instructions"), false);

    QGeoRouteRequest::RouteOptimizations routeOptimization = request.routeOptimization();
    if (routeOptimization.testFlag(QGeoRouteRequest::ShortestRoute)) {
        geojson.insert(QStringLiteral("preference"), QStringLiteral("shortest"));
    } else if (routeOptimization.testFlag(QGeoRouteRequest::FastestRoute)) {
        geojson.insert(QStringLiteral("preference"), QStringLiteral("fastest"));
    }

    QNetworkReply *reply = m_networkManager->post(networkRequest, QJsonDocument(geojson).toJson());

    QGeoRouteReplyOpenrouteservice *routeReply = new QGeoRouteReplyOpenrouteservice(reply, request, this);

    connect(routeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(routeReply, SIGNAL(error(QGeoRouteReply::Error,QString)),
            this, SLOT(replyError(QGeoRouteReply::Error,QString)));

    return routeReply;
}

void QGeoRoutingManagerEngineOpenrouteservice::replyFinished()
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        Q_EMIT(finished(reply));
}

void QGeoRoutingManagerEngineOpenrouteservice::replyError(QGeoRouteReply::Error errorCode,
                                             const QString &errorString)
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        Q_EMIT(error(reply, errorCode, errorString));
}

QT_END_NAMESPACE
