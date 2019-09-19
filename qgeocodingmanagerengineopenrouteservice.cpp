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

#include <QGeoCoordinate>
#include <QUrlQuery>

QT_BEGIN_NAMESPACE

QGeoCodingManagerEngineOpenrouteservice::QGeoCodingManagerEngineOpenrouteservice(const QVariantMap &parameters,
                                                       QGeoServiceProvider::Error *error,
                                                       QString *errorString)
:   QGeoCodingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("ors.api_key"))) {
        m_accessToken = parameters.value(QStringLiteral("ors.api_key")).toString();
    }

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoCodingManagerEngineOpenrouteservice::~QGeoCodingManagerEngineOpenrouteservice()
{
}

//QGeoCodeReply *QGeoCodingManagerEngineGraphhopper::geocode(const QGeoAddress &address, const QGeoShape &bounds)
//{
//}

//QGeoCodeReply *QGeoCodingManagerEngineGraphhopper::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
//{
//}

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
    connect(geocodeReply, QOverload<QGeoCodeReply::Error, const QString &>::of(&QGeoCodeReply::error),
            this, &QGeoCodingManagerEngineOpenrouteservice::onReplyError);

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
        Q_EMIT(error(reply, errorCode, errorString));
}

QT_END_NAMESPACE
