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
    QNetworkAccessManager manager;

    // Сайт с которого качаем TLE файл
    QString urlString = QString("https://celestrak.org/NORAD/elements/gp.php?CATNR=%1&FORMAT=TLE").arg(QString::number(satelliteNumber));
    QUrl url(urlString);
    QNetworkRequest request(url);

    qDebug() << "Downloading TLE data for " << satelliteNumber;

    QNetworkReply *reply = manager.get(request);

    // Создаем цикл, чтобы дождаться окончания скачивания.
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QString outputFile = QString::fromStdString(savePath);
        QFile file(outputFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(reply->readAll());
            file.close();
            qDebug() << "Data successfully saved to" << outputFile;
        } else {
            qDebug() << "Error: Unable to open file" << outputFile << "for writing";
            return false;
        }
    } else {
        qDebug() << "Error downloading data:" << reply->errorString();
        return false;
    }

    reply->deleteLater();
    return true;
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
