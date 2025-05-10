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

#include "qgeoroutereplyopenrouteservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QT_BEGIN_NAMESPACE

static QList<QGeoCoordinate> decodePolyline(const QString &line)
{
        QList<QGeoCoordinate> path;
        QByteArray data(line.toLocal8Bit());

        bool is3D = true;
        int index = 0;
        int len = data.length();
        int lat = 0, lng = 0, ele = 0;
        while (index < len) {
            // latitude
            int b, shift = 0, result = 0;
            do {
                b = data.at(index++) - 63;
                result |= (b & 0x1f) << shift;
                shift += 5;
            } while (b >= 0x20);
            int deltaLatitude = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
            lat += deltaLatitude;

            // longitute
            shift = 0;
            result = 0;
            do {
                b = data.at(index++) - 63;
                result |= (b & 0x1f) << shift;
                shift += 5;
            } while (b >= 0x20);
            int deltaLongitude = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
            lng += deltaLongitude;

            if (is3D) {
                // elevation
                shift = 0;
                result = 0;
                do {
                    b = data.at(index++) - 63;
                    result |= (b & 0x1f) << shift;
                    shift += 5;
                } while (b >= 0x20);
                int deltaElevation = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
                ele += deltaElevation;
                path.append(QGeoCoordinate(static_cast<double>(lat) / 1e5, static_cast<double>(lng) / 1e5, static_cast<double>(ele) / 100.0));
            } else
                path.append(QGeoCoordinate(static_cast<double>(lat) / 1e5, static_cast<double>(lng) / 1e5));
        }
        return path;
    }

static QList<QGeoCoordinate> parseGeometry(const QJsonValue &geometry)
{
    QList<QGeoCoordinate> path;
    if (geometry.isString()) path = decodePolyline(geometry.toString());
    return path;
}

QGeoRouteReplyOpenrouteservice::QGeoRouteReplyOpenrouteservice(QObject *parent)
    : QGeoRouteReply(QGeoRouteRequest(), parent), m_reply(nullptr)
{
}

QGeoRouteReplyOpenrouteservice::QGeoRouteReplyOpenrouteservice(QNetworkReply *reply, const QGeoRouteRequest &request,
                                     QObject *parent)
:   QGeoRouteReply(request, parent), m_reply(reply)
{
    connect(m_reply, &QNetworkReply::finished, this, &QGeoRouteReplyOpenrouteservice::networkReplyFinished);
    connect(m_reply, &QNetworkReply::errorOccurred, this, &QGeoRouteReplyOpenrouteservice::networkReplyError);
}

QGeoRouteReplyOpenrouteservice::~QGeoRouteReplyOpenrouteservice()
{
    if (m_reply)
        m_reply->deleteLater();
}

void QGeoRouteReplyOpenrouteservice::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();

    m_reply->deleteLater();
    m_reply = nullptr;
}

static QGeoRoute constructRoute(const QJsonObject &obj)
{
    QGeoRoute route;
    QJsonObject summary = obj.value(QStringLiteral("summary")).toObject();
    route.setDistance(summary.value(QStringLiteral("distance")).toDouble());
    route.setTravelTime(static_cast<int>(summary.value(QStringLiteral("duration")).toDouble()));

    QVariantMap attributes;
    attributes["ascent"] = summary.value(QStringLiteral("ascent"));
    attributes["descent"] = summary.value(QStringLiteral("descent"));
    route.setExtendedAttributes(attributes);

    QList<QGeoCoordinate> path = parseGeometry(obj.value(QStringLiteral("geometry")));
    route.setPath(path);

    return route;
}

void QGeoRouteReplyOpenrouteservice::networkReplyFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError) {
        setError(QGeoRouteReply::CommunicationError, m_reply->errorString());
        m_reply->deleteLater();
        m_reply = nullptr;
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(m_reply->readAll());
    if (document.isObject()) {
        QJsonObject object = document.object();
        QList<QGeoRoute> list;
        QJsonArray routes = object.value(QStringLiteral("routes")).toArray();
        for (int i = 0; i < routes.count(); i++) {
            QGeoRoute route = constructRoute(routes.at(i).toObject());
            list.append(route);
        }
        setRoutes(list);
        setFinished(true);
    } else {
        setError(QGeoRouteReply::ParseError, QStringLiteral("Couldn't parse json."));
    }

    m_reply->deleteLater();
    m_reply = nullptr;
}

void QGeoRouteReplyOpenrouteservice::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)

    if (!m_reply)
        return;

    setError(QGeoRouteReply::CommunicationError, m_reply->errorString());

    m_reply->deleteLater();
    m_reply = nullptr;
}

QT_END_NAMESPACE
