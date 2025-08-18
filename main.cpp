#include <QCoreApplication>
#include <QSettings>
#include <QFileInfo>
#include <QTimer>
#include <QDir>

#include "TleProcessor.h"
#include "ScheduleSaver/FileNoradScheduleSaver.h"
#include "Utils/Utility.h"
#include "Receiver/ReceiverUDP.h"
#include "FileSender/SftpFileSender.h"
#include "Utils/UtilResponseParser.h"

int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);
    // Create second output stream to log console messages in the file "app.log"
    qInstallMessageHandler(Utility::СustomMessageHandler);
    Utility::СlearLogFile();

    QString tleFilePath = "ServiceFiles/definiteTLE.txt";        // File with TLE info
    QString settingsFilePath = "ServiceFiles/settings.ini";      // File with settings
    QString resultFilePath = "NoradSchedule.txt";   // File with results for one satellite

    // class for saving results in file
    std::unique_ptr<INoradScheduleSaver> saver = std::make_unique<FileNoradScheduleSaver>(resultFilePath.toStdString());

    QFileInfo settingsFileInfo(settingsFilePath);
    // Create file if it doesn't exists
    if (!settingsFileInfo.exists()) {
        Utility::CreateSettingsFile(settingsFilePath);
        qInfo() << "[Main]: Created new settings file (settings.ini).";
    }


    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.beginGroup("Command");
    saver->setCommand(settings.value("cmd").toInt());
    settings.endGroup();

    // Group for log in on Space-Track.org
    settings.beginGroup("TLEbySpaceTrack");
    QString login = settings.value("login").toString();
    QString pass = settings.value("pass").toString();
    settings.endGroup();
    if(login.isEmpty() or pass.isEmpty()){
        qCritical() << "[Main]:[settings]: no login or pass!";
    }

    settings.beginGroup("ObserverConfiguration");
    CTleProcessor tleProcessor(std::move(saver), settings.value("Latitude").toDouble(), settings.value("Longitude").toDouble(), settings.value("Altitude").toDouble());
    uint numKA = settings.value("numKA").toUInt();
    uint dt_mks = settings.value("dt_mks").toUInt();
    settings.endGroup();

    // Create a timer for the 10-second timeout
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(&timeoutTimer, &QTimer::timeout, [&]() {
        qInfo() << "[Main]: 10-second timeout reached, no UDP response received. Exiting...";
        QCoreApplication::exit(0);
    });

    // Make connection to start calculations after TLE data file will be downloaded
    QObject::connect(&tleProcessor, &CTleProcessor::tleDownloaded, [&](bool success) {
        if (!success) {
            qCritical() << "[Main]: Failed to download TLE. Exiting...";
            QCoreApplication::exit(1);
            return;
        }

        // Start calculations
        tleProcessor.loadTleFile(tleFilePath.toStdString());
        tleProcessor.processTleData(numKA, dt_mks);

        // Sending formed file to PLC by SFTP
        settings.beginGroup("SshConnection");
        std::unique_ptr<IFileSenderPLC> sender = std::make_unique<SftpFileSender>(settings.value("host").toString(), settings.value("port").toUInt(),
                                                                                  settings.value("login").toString(), settings.value("password").toString(),
                                                                                  resultFilePath, resultFilePath);
        settings.endGroup();
        sender->send();

        // Adding UDP receiver
        ReceiverUDP receiver{};


        // Start listening on default port 3545
        if (!receiver.startListening()) {
            qCritical() << "[Main]: Failed to start UDP receiver";
            QCoreApplication::exit(1);
            return;
        }

        // Connect UDP receiver to handle response
        QObject::connect(&receiver, &ReceiverUDP::responseReceived, [&](QByteArray receivedData) {
            std::optional<UtilResponseParser::ResponseHeader> answer = UtilResponseParser::parseResponse(receivedData);
            if (answer.has_value()) {
                answer.value().print();
            } else {
                qDebug() << "[Main]: No value in answer!";
            }
            // Stop the timer since we received a response
            timeoutTimer.stop();
            qInfo() << "[Main]: UDP response received, exiting...";
            QCoreApplication::exit(0);
        });

        // Start the 10-second timeout timer after sending the file
        timeoutTimer.start(10000); // 10 seconds in milliseconds
    });

    // Download TLE data from Space-Track.org (3-line format)
    tleProcessor.downloadTleFromUrl(numKA, tleFilePath.toStdString(), login.toStdString(), pass.toStdString());

    return a.exec();
}
