#include "TleProcessor.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDebug>
#include <QUrl>

CTleProcessor::CTleProcessor(std::unique_ptr<INoradScheduleSaver> saver, double lat, double lon, double altm)
{
    if (saver == nullptr)
        throw std::invalid_argument("[CTleProcessor::CTleProcessor]: Saver is nullptr");
    m_noradSaver = std::move(saver);
    m_noradPrc.reset(new CNoradProcessor(this, lat, lon, altm));
}


bool CTleProcessor::downloadTleFromUrl(const uint32_t satelliteNumber, const std::string& savePath){
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QString urlString = QString("https://celestrak.org/NORAD/elements/gp.php?CATNR=%1&FORMAT=TLE").arg(QString::number(satelliteNumber));
    QUrl url(urlString);
    QNetworkRequest request(url);
    request.setTransferTimeout(10000); // Таймаут 10 секунд

    qDebug() << "Downloading TLE data for " << satelliteNumber;

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        bool success = false;
        if (reply->error() == QNetworkReply::NoError) {
            QString outputFile = QString::fromStdString(savePath);
            QFile file(outputFile);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(reply->readAll());
                file.close();
                qDebug() << "Data successfully saved to" << outputFile;
                success = true;
            } else {
                qDebug() << "Error: Unable to open file" << outputFile << "for writing";
            }
        } else {
            qDebug() << "Error downloading data:" << reply->errorString();
        }
        reply->deleteLater();
        manager->deleteLater();

        emit tleDownloaded(success);
    });
    return true;
}

bool CTleProcessor::downloadTleFromUrl(const uint32_t satelliteNumber, const std::string& savePath, const std::string& username, const std::string& password) {
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QUrl loginUrl("https://www.space-track.org/ajaxauth/login");
    QNetworkRequest loginRequest(loginUrl);
    loginRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    loginRequest.setTransferTimeout(10000);

    QString loginData = QString("identity=%1&password=%2").arg(QString::fromStdString(username)).arg(QString::fromStdString(password));
    QByteArray postData = loginData.toUtf8();

    qInfo() << "[CTleProcessor::downloadTleFromUrl]: Authenticating with Space-Track for satellite:" << satelliteNumber;

    QNetworkReply *loginReply = manager->post(loginRequest, postData);

    // Log in to download tle
    connect(loginReply, &QNetworkReply::finished, this, [=]() {
        bool success = false;
        int httpStatus = loginReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "[CTleProcessor::downloadTleFromUrl]: Login HTTP status:" << httpStatus << " | Error code:" << loginReply->error();
        if (loginReply->error() == QNetworkReply::NoError && httpStatus == 200)
        {
            qInfo() << "[CTleProcessor::downloadTleFromUrl]: Login successful. Now downloading TLE data for " << satelliteNumber;
            QString urlString = QString("https://www.space-track.org/basicspacedata/query/class/tle_latest/NORAD_CAT_ID/%1/ORDINAL/1/format/3le").arg(satelliteNumber);
            QUrl queryUrl(urlString);
            QNetworkRequest queryRequest(queryUrl);
            queryRequest.setTransferTimeout(10000);
            QNetworkReply *queryReply = manager->get(queryRequest);

            // When we logged in, Send query to take tle data
            connect(queryReply, &QNetworkReply::finished, this, [=]() mutable{
                    int queryHttpStatus = queryReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    qDebug() << "[CTleProcessor::downloadTleFromUrl]: Query HTTP status:" << queryHttpStatus << " | Error code:" << queryReply->error();

                    if (queryReply->error() == QNetworkReply::NoError && queryHttpStatus == 200) {
                        QString outputFile = QString::fromStdString(savePath);
                        QFile file(outputFile);

                        // If tle data received successfuly, then save downloaded data in file
                        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            QByteArray data = queryReply->readAll();
                            file.write(data);
                            file.close();
                            qInfo() << "[CTleProcessor::downloadTleFromUrl]: Data successfully saved to" << outputFile << " | Data size:" << data.size();  // Added: Confirm data was written
                            success = true;
                        } else {
                            qCritical() << "[CTleProcessor::downloadTleFromUrl]: Error: Unable to open file" << outputFile << "for writing";
                        }
                    } else{
                        qCritical() << "[CTleProcessor::downloadTleFromUrl]: Error downloading TLE data:" << queryReply->errorString();
                    }
                    queryReply->deleteLater();
                    QUrl logoutUrl("https://www.space-track.org/ajaxauth/logout");
                    QNetworkRequest logoutRequest(logoutUrl);
                    QNetworkReply *logoutReply = manager->get(logoutRequest);
                    connect(logoutReply, &QNetworkReply::finished, this, [=]() {
                        qDebug() << "Logout completed. HTTP status:" << logoutReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                                 << " | Error:" << logoutReply->error();
                        logoutReply->deleteLater();
                    });
                    manager->deleteLater();
                    emit tleDownloaded(success);
                });
        } else {
            qCritical() << "[CTleProcessor::downloadTleFromUrl]: Login failed:" << loginReply->errorString();
            loginReply->deleteLater();
            manager->deleteLater();
            emit tleDownloaded(success);
        }
    });

    return true;
}

bool CTleProcessor::loadTleFile(const std::string& file)
{
    if(!m_noradPrc) return false;

    m_noradPrc->loadToBufferTLE(file);

    return true;
}

bool CTleProcessor::processTleData(const uint32_t satelliteNumber, const uint32_t dt_mks)
{
    if(!m_noradPrc) return false;

    CNoradProcessor::NORAD_ERROR e = m_noradPrc->genSchedule(satelliteNumber, m_vecNoradSchedule, m_noradSaver, dt_mks);
    qDebug() << "[CTleProcessor::processTleData]: status:" <<e;

    return true;
}
