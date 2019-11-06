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
#include <QGeoRectangle>
#include <QGeoAddress>

QT_BEGIN_NAMESPACE

static QGeoAddress parseAddressObject(const QJsonObject &object)
{
    QGeoAddress address;
    address.setText(object.value(QStringLiteral("label")).toString());
    address.setCountry(object.value(QStringLiteral("country")).toString());
    address.setCountryCode(object.value(QStringLiteral("country_a")).toString());
    address.setState(object.value(QStringLiteral("region")).toString());
    address.setCity(object.value(QStringLiteral("locality")).toString());
    address.setDistrict(object.value(QStringLiteral("localadmin")).toString());
    address.setStreet(object.value(QStringLiteral("street")).toString());
    address.setPostalCode(object.value(QStringLiteral("postalcode")).toString());
    return address;
}

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

    QJsonArray features = document.object().value(QStringLiteral("features")).toArray();
    for (int i = 0; i < features.count(); ++i) {
        QGeoLocation location;
        QJsonObject feature = features.at(i).toObject();
        if (feature.contains(QStringLiteral("bbox"))) {
            QGeoRectangle bbox;
            QJsonArray box = feature.value(QStringLiteral("bbox")).toArray();
            if (box.count() == 4) {
                bbox.setBottomLeft(QGeoCoordinate(box.at(1).toDouble(), box.at(0).toDouble()));
                bbox.setTopRight(QGeoCoordinate(box.at(3).toDouble(), box.at(2).toDouble()));
            }
            if (bbox.isValid()) {
                location.setBoundingBox(bbox);
            }
        }
        QJsonObject geometry = feature.value(QStringLiteral("geometry")).toObject();
        QJsonArray coordinates = geometry.value(QStringLiteral("coordinates")).toArray();
        location.setCoordinate(QGeoCoordinate(coordinates.at(1).toDouble(), coordinates.at(0).toDouble()));
        QJsonObject properties = feature.value(QStringLiteral("properties")).toObject();
        location.setAddress(parseAddressObject(properties));

        QVariantMap attributes;
        attributes.insert(QStringLiteral("name"), properties.value(QStringLiteral("name")).toString());
        location.setExtendedAttributes(attributes);

        addLocation(location);
    }

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
