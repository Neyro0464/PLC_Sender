#include <QCoreApplication>
#include <QSettings>
#include <QFileInfo>
#include <QTimer>
#include <QDir>

#include "TleProcessor.h"
#include "ScheduleSaver/FileNoradScheduleSaver.h"
#include "Utils/Utility.h"
#include "Receiver/ReceiverUDP.h"
#include "Utils/UtilResponseParser.h"
#include "FileSender/UdpSender.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    qInstallMessageHandler(Utility::СustomMessageHandler);
    Utility::СlearLogFile();

    QString tleFilePath = "ServiceFiles/definiteTLE.txt";
    QString settingsFilePath = "ServiceFiles/settings.ini";
    QString resultFilePath = "NoradSchedule.txt";
    QString resultBinFile = "NoradSchedule.bin";

    std::unique_ptr<INoradScheduleSaver> saver = std::make_unique<FileNoradScheduleSaver>(resultFilePath.toStdString());

    QFileInfo settingsFileInfo(settingsFilePath);
    if (!settingsFileInfo.exists()) {
        Utility::CreateSettingsFile(settingsFilePath);
        qInfo() << "[Main]: Created new settings file (settings.ini).";
    }

    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.beginGroup("Command");
    saver->setCommand(settings.value("cmd").toInt());
    settings.endGroup();

    settings.beginGroup("TLEbySpaceTrack");
    QString login = settings.value("login").toString();
    QString pass = settings.value("pass").toString();
    settings.endGroup();
    if(login.isEmpty() or pass.isEmpty()){
        qCritical() << "[Main]:[settings]: no login or pass!";
    }

    settings.beginGroup("ObserverConfiguration");
    CTleProcessor tleProcessor(std::move(saver),
                               settings.value("Latitude").toDouble(),
                               settings.value("Longitude").toDouble(),
                               settings.value("Altitude").toDouble());
    uint numKA = settings.value("numKA").toUInt();
    uint dt_mks = settings.value("dt_mks").toUInt();
    settings.endGroup();

    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(&timeoutTimer, &QTimer::timeout, [&]() {
        qInfo() << "[Main]: 10-second timeout reached, no UDP response received. Exiting...";
        QCoreApplication::exit(0);
    });

    QObject::connect(&tleProcessor, &CTleProcessor::tleDownloaded, [&](bool success) {
        if (!success) {
            qCritical() << "[Main]: Failed to download TLE. Exiting...";
            QCoreApplication::exit(1);
            return;
        }

        tleProcessor.loadTleFile(tleFilePath.toStdString());
        tleProcessor.processTleData(numKA, dt_mks);
        std::vector<NORAD_SCHEDULE> data = tleProcessor.getProcessedData();

        settings.beginGroup("UdpConnection");
        QString udpHost = settings.value("host").toString();
        quint16 udpPort = settings.value("port").toUInt();
        settings.endGroup();

        // Создаем UdpSender с дополнительным параметром numKA
        UdpSender dataForSend(data, numKA, QHostAddress(udpHost), udpPort);
        qDebug() << "[Main]: UdpSender created successfully";

        try {
            if (dataForSend.sendData()) {
                qDebug() << "[Main]: Data sent successfully via UDP";
                qDebug() << "[Main]: Total data size:" << dataForSend.getDataSize() << "bytes";
            } else {
                qWarning() << "[Main]: Failed to send data via UDP";
            }
        } catch (const std::exception& e) {
            qCritical() << "[Main]: Exception during UDP send:" << e.what();
            QCoreApplication::exit(1);
        }

        ReceiverUDP receiver{};

        if (!receiver.startListening()) {
            qCritical() << "[Main]: Failed to start UDP receiver";
            QCoreApplication::exit(1);
            return;
        }

        QObject::connect(&receiver, &ReceiverUDP::responseReceived, [&](QByteArray receivedData) {
            std::optional<UtilResponseParser::ResponseHeader> answer = UtilResponseParser::parseResponse(receivedData);
            if (answer.has_value()) {
                answer.value().print();
            } else {
                qDebug() << "[Main]: No value in answer!";
            }
            timeoutTimer.stop();
            qInfo() << "[Main]: UDP response received, exiting...";
            QCoreApplication::exit(0);
        });

        timeoutTimer.start(10000);
    });

    tleProcessor.downloadTleFromUrl(numKA, tleFilePath.toStdString(), login.toStdString(), pass.toStdString());

    return a.exec();
}
