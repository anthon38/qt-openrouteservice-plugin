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

#include "qgeocodereplyopenrouteservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGeoCoordinate>

QT_BEGIN_NAMESPACE

QGeoCodeReplyOpenrouteservice::QGeoCodeReplyOpenrouteservice(QNetworkReply *reply, QObject *parent)
:   QGeoCodeReply(parent)
{
    Q_ASSERT(parent);
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }

    connect(reply, &QNetworkReply::finished, this, &QGeoCodeReplyOpenrouteservice::onNetworkReplyFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &QGeoCodeReplyOpenrouteservice::onNetworkReplyError);

    connect(this, &QGeoCodeReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QGeoCodeReplyOpenrouteservice::~QGeoCodeReplyOpenrouteservice()
{
}

void QGeoCodeReplyOpenrouteservice::onNetworkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isObject()) {
        setError(ParseError, tr("Response parse error"));
        return;
    }

    QGeoLocation location;

    QJsonArray features = document.object().value(QStringLiteral("features")).toArray();
    QJsonObject poi = features.at(0).toObject();
    QJsonObject geometry = poi.value(QStringLiteral("geometry")).toObject();
    QJsonArray coordinates = geometry.value(QStringLiteral("coordinates")).toArray();
    location.setCoordinate(QGeoCoordinate(coordinates.at(1).toDouble(), coordinates.at(0).toDouble()));

    QVariantMap attributes;
    QJsonObject properties = poi.value(QStringLiteral("properties")).toObject();
    attributes.insert(QStringLiteral("name"), properties.value(QStringLiteral("name")).toString());
    location.setExtendedAttributes(attributes);

    addLocation(location);

    setFinished(true);
}

void QGeoCodeReplyOpenrouteservice::onNetworkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QGeoCodeReply::CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
