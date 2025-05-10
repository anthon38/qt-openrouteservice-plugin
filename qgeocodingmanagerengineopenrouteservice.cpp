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

#include "qgeocodingmanagerengineopenrouteservice.h"
#include "qgeocodereplyopenrouteservice.h"

#include <QGeoAddress>
#include <QGeoCoordinate>
#include <QUrlQuery>
#include <QGeoShape>
#include <QGeoCircle>

QT_BEGIN_NAMESPACE

static QString addressToQuery(const QGeoAddress &address)
{
    return address.street() + QStringLiteral(", ") +
           address.district() + QStringLiteral(", ") +
           address.city() + QStringLiteral(", ") +
           address.state() + QStringLiteral(", ") +
           address.country();
}

QGeoCodingManagerEngineOpenrouteservice::QGeoCodingManagerEngineOpenrouteservice(const QVariantMap &parameters,
                                                       QGeoServiceProvider::Error *error,
                                                       QString *errorString)
:   QGeoCodingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("ors.api_key"))) {
        m_accessToken = parameters.value(QStringLiteral("ors.api_key")).toString();
    }
    if (parameters.contains(QStringLiteral("ors.search.sources"))) {
        QStringList sources = parameters.value(QStringLiteral("ors.search.sources")).toStringList();
        for (auto source : sources) {
            m_sources += source + ",";
        }
        if (m_sources.endsWith(','))
            m_sources.chop(1);
    }
    if (parameters.contains(QStringLiteral("ors.search.focus.point"))) {
        QVariant fp = parameters.value(QStringLiteral("ors.search.focus.point"));
        if (fp.canConvert<QGeoCoordinate>())
            m_focusPoint = fp.value<QGeoCoordinate>();
    }

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoCodingManagerEngineOpenrouteservice::~QGeoCodingManagerEngineOpenrouteservice()
{
}

QGeoCodeReply *QGeoCodingManagerEngineOpenrouteservice::geocode(const QGeoAddress &address, const QGeoShape &bounds)
{
    return geocode(addressToQuery(address), -1, -1, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineOpenrouteservice::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
{
    Q_UNUSED(offset)

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("api_key"), m_accessToken);
    query.addQueryItem(QStringLiteral("text"), address);
    if (limit != -1) {
        query.addQueryItem(QStringLiteral("size"), QString::number(limit));
    }
    if (!m_sources.isEmpty()) {
        query.addQueryItem(QStringLiteral("sources"), m_sources);
    }
    if (m_focusPoint.isValid()) {
        query.addQueryItem(QStringLiteral("focus.point.lat"), QString::number(m_focusPoint.latitude()));
        query.addQueryItem(QStringLiteral("focus.point.lon"), QString::number(m_focusPoint.longitude()));
    }
    if (bounds.isValid()) {
        if (bounds.type() == QGeoShape::CircleType) {
            QGeoCircle circle(bounds);
            query.addQueryItem(QStringLiteral("boundary.circle.lat"), QString::number(circle.center().latitude()));
            query.addQueryItem(QStringLiteral("boundary.circle.lon"), QString::number(circle.center().longitude()));
            query.addQueryItem(QStringLiteral("boundary.circle.radius"), QString::number(circle.radius()));
        } else {
            query.addQueryItem(QStringLiteral("boundary.rect.min_lat"), QString::number(bounds.boundingGeoRectangle().bottomLeft().latitude()));
            query.addQueryItem(QStringLiteral("boundary.rect.max_lat"), QString::number(bounds.boundingGeoRectangle().topLeft().latitude()));
            query.addQueryItem(QStringLiteral("boundary.rect.min_lon"), QString::number(bounds.boundingGeoRectangle().topLeft().longitude()));
            query.addQueryItem(QStringLiteral("boundary.rect.max_lon"), QString::number(bounds.boundingGeoRectangle().topRight().longitude()));
        }
    }

    QUrl url(QStringLiteral("https://api.openrouteservice.org/geocode/search?"));
    url.setQuery(query);

    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    networkRequest.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json, application/geo+json, application/gpx+xml, img/png; charset=utf-8"));

    QNetworkReply *reply = m_networkManager->get(networkRequest);

    QGeoCodeReplyOpenrouteservice *geocodeReply = new QGeoCodeReplyOpenrouteservice(reply, this);

    connect(geocodeReply, &QGeoCodeReplyOpenrouteservice::finished, this, &QGeoCodingManagerEngineOpenrouteservice::onReplyFinished);
    connect(geocodeReply, &QGeoCodeReplyOpenrouteservice::errorOccurred, this, &QGeoCodingManagerEngineOpenrouteservice::onReplyError);

    return geocodeReply;
}

QGeoCodeReply *QGeoCodingManagerEngineOpenrouteservice::reverseGeocode(const QGeoCoordinate &coordinate, const QGeoShape &bounds)
{
    Q_UNUSED(bounds)

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("api_key"), m_accessToken);
    query.addQueryItem(QStringLiteral("point.lon"), QString::number(coordinate.longitude()));
    query.addQueryItem(QStringLiteral("point.lat"), QString::number(coordinate.latitude()));
    query.addQueryItem(QStringLiteral("size"), QString::number(1));

    QUrl url(QStringLiteral("https://api.openrouteservice.org/geocode/reverse?"));
    url.setQuery(query);

    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    networkRequest.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json, application/geo+json, application/gpx+xml, img/png; charset=utf-8"));

    QNetworkReply *reply = m_networkManager->get(networkRequest);

    QGeoCodeReplyOpenrouteservice *geocodeReply = new QGeoCodeReplyOpenrouteservice(reply, this);

    connect(geocodeReply, &QGeoCodeReplyOpenrouteservice::finished, this, &QGeoCodingManagerEngineOpenrouteservice::onReplyFinished);
    connect(geocodeReply,  &QGeoCodeReplyOpenrouteservice::errorOccurred, this, &QGeoCodingManagerEngineOpenrouteservice::onReplyError);

    return geocodeReply;
}

void QGeoCodingManagerEngineOpenrouteservice::onReplyFinished()
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        Q_EMIT(finished(reply));
}

void QGeoCodingManagerEngineOpenrouteservice::onReplyError(QGeoCodeReply::Error errorCode, const QString &errorString)
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        Q_EMIT(errorOccurred(reply, errorCode, errorString));
}

QT_END_NAMESPACE
