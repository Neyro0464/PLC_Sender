#include "TleProcessor.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QUrl>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QTextStream>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

CTleProcessor::CTleProcessor(std::unique_ptr<INoradScheduleSaver> saver, double lat, double lon, double altm)
{
    if (saver == nullptr)
        throw std::invalid_argument("[CTleProcessor::CTleProcessor]: Saver is nullptr");
    m_noradSaver = std::move(saver);
    m_noradPrc.reset(new CNoradProcessor(this, lat, lon, altm));
}

bool CTleProcessor::downloadTleFromUrl(const uint32_t satelliteNumber, const QString& savePath, const QString& username, const QString& password, const uint16_t port, const QString url, const uint32_t tleCacheHours) {
    // Проверяем существующий файл
    if (isTleFileValid(savePath, satelliteNumber, tleCacheHours)) {
        qInfo() << "[CTleProcessor::downloadTleFromUrl]: Using cached TLE data for satellite:" << satelliteNumber;
        emit tleDownloaded(true);
        return true;
    }

    QEventLoop loop;
    bool success = false;

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    // Добавляем таймаут для всей операции
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);

    QUrl loginUrl("https://www.space-track.org/ajaxauth/login");
    QNetworkRequest loginRequest(loginUrl);
    loginRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    loginRequest.setTransferTimeout(10000);

    QString loginData = QString("identity=%1&password=%2").arg(username).arg(password);
    QByteArray postData = loginData.toUtf8();

    qInfo() << "[CTleProcessor::downloadTleFromUrl]: Authenticating with Space-Track for satellite:" << satelliteNumber;

    QNetworkReply *loginReply = manager->post(loginRequest, postData);

    connect(loginReply, &QNetworkReply::finished, [&]() {
        int httpStatus = loginReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "[CTleProcessor::downloadTleFromUrl]: Login HTTP status:" << httpStatus << " | Error code:" << loginReply->error();

        if (loginReply->error() == QNetworkReply::NoError && httpStatus == 200) {
            qInfo() << "[CTleProcessor::downloadTleFromUrl]: Login successful. Now downloading TLE data for " << satelliteNumber;
            QString urlString = QString(url).arg(satelliteNumber);
            QUrl queryUrl(urlString);
            QNetworkRequest queryRequest(queryUrl);
            queryRequest.setTransferTimeout(10000);
            QNetworkReply *queryReply = manager->get(queryRequest);

            connect(queryReply, &QNetworkReply::finished, [&, queryReply, manager]() {
                int queryHttpStatus = queryReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                qDebug() << "[CTleProcessor::downloadTleFromUrl]: Query HTTP status:" << queryHttpStatus
                         << " | Error code:" << queryReply->error();

                if (queryReply->error() == QNetworkReply::NoError && queryHttpStatus == 200) {
                    QString outputFile = QString::fromStdString(savePath.toStdString());
                    QFile file(outputFile);

                    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QByteArray data = queryReply->readAll();
                        file.write(data);
                        file.close();
                        qInfo() << "[CTleProcessor::downloadTleFromUrl]: Data successfully saved to"
                                << outputFile << " | Data size:" << data.size();
                        success = true;
                    } else {
                        qCritical() << "[CTleProcessor::downloadTleFromUrl]: Error: Unable to open file"
                                    << outputFile << "for writing";
                    }
                } else {
                    qCritical() << "[CTleProcessor::downloadTleFromUrl]: Error downloading TLE data:"
                                << queryReply->errorString();
                }

                // Выполняем logout
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
                loop.quit();  // Завершаем цикл событий только после полного завершения всех операций
                });

        } else {
            qCritical() << "[CTleProcessor::downloadTleFromUrl]: Login failed:" << loginReply->errorString();
            loginReply->deleteLater();
            manager->deleteLater();
            emit tleDownloaded(false);
            loop.quit();
        }
    });

    loginReply->deleteLater();
    loop.exec();  // Ждем завершения всех операций
    return success;
}

bool CTleProcessor::isTleFileValid(const QString& filePath, uint32_t satelliteNumber, const uint32_t tleCacheHours) {
    QFile file(filePath);
    if (!file.exists()) {
        return false;
    }

    // Получаем время последней модификации файла
    QFileInfo fileInfo(file);
    QDateTime lastModified = fileInfo.lastModified();
    QDateTime current = QDateTime::currentDateTime();
    uint32_t hoursElapsed = lastModified.secsTo(current) / 3600;

    if (hoursElapsed > tleCacheHours) {
        return false;
    }

    // Проверяем содержимое файла
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    QString line;
    bool foundSatellite = false;

    // Читаем файл построчно
    while (!in.atEnd()) {
        line = in.readLine();
        // Проверяем вторую строку TLE, которая содержит номер спутника
        if (line.startsWith("2 ")) {
            // Номер спутника находится в позициях 3-7
            QString satNumStr = line.mid(2, 5).trimmed();
            bool ok;
            uint32_t fileSatNum = satNumStr.toUInt(&ok);
            if (ok && fileSatNum == satelliteNumber) {
                foundSatellite = true;
                break;
            }
        }
    }

    file.close();
    return foundSatellite;
}

bool CTleProcessor::loadTleFile(const std::string& file)
{
    if(!m_noradPrc) return false;

    m_noradPrc->loadToBufferTLE(file);

    return true;
}

bool CTleProcessor::processTleData(const uint32_t satelliteNumber, const uint32_t dt_sec, const uint32_t dt_delay)
{
    if(!m_noradPrc) return false;

    CNoradProcessor::NORAD_ERROR e = m_noradPrc->genSchedule(satelliteNumber, m_vecNoradSchedule, m_noradSaver, dt_sec, dt_delay);
    qDebug() << "[CTleProcessor::processTleData]: status:" <<e;

    return true;
}
