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

#ifndef QGEOCODINGMANAGERENGINEOPENROUTESERVICE_H
#define QGEOCODINGMANAGERENGINEOPENROUTESERVICE_H

#include <QGeoServiceProvider>
#include <QGeoCodingManagerEngine>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;


class QGeoCodingManagerEngineOpenrouteservice : public QGeoCodingManagerEngine
{
    Q_OBJECT

public:
    QGeoCodingManagerEngineOpenrouteservice(const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                               QString *errorString);
    ~QGeoCodingManagerEngineOpenrouteservice() override;

//    QGeoCodeReply *geocode(const QGeoAddress &address, const QGeoShape &bounds) override;
//    QGeoCodeReply *geocode(const QString &address, int limit, int offset,
//                           const QGeoShape &bounds) override;
    QGeoCodeReply *reverseGeocode(const QGeoCoordinate &coordinate,
                                  const QGeoShape &bounds) override;

private Q_SLOTS:
    void onReplyFinished();
    void onReplyError(QGeoCodeReply::Error errorCode, const QString &errorString);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_accessToken;
};

QT_END_NAMESPACE

#endif // QGEOCODINGMANAGERENGINEOPENROUTESERVICE_H
