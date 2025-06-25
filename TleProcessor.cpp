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
        throw std::invalid_argument("Saver is nullptr");
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

        emit allOperationsFinished(success);
    });
    return true; // Запрос инициирован
}

bool CTleProcessor::loadTleFile(const std::string& file)
{
    if(!m_noradPrc) return false;

    m_noradPrc->loadToBufferTLE(file);

    return true;
}

bool CTleProcessor::processTleData(const uint32_t satelliteNumber)
{
    if(!m_noradPrc) return false;

    m_noradPrc->genSchedule(satelliteNumber, m_vecNoradSchedule, m_noradSaver);

    return true;
}
