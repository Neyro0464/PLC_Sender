#include <QCoreApplication>
#include <QSettings>
#include <QFileInfo>

#include <QDir>

#include "TleProcessor.h"
#include "ScheduleSaver/FileNoradScheduleSaver.h"
#include "Utility.h"

int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);
    qInstallMessageHandler(Utility::СustomMessageHandler);
    Utility::СlearLogFile();

    QString tleFilePath = "definiteTLE.txt";                // File with TLE info
    QString settingsFilePath = "settings.ini";      // File with settings
    QString resultFilePath = "NoradSchedule.txt";   // File with results for one satellite

    // classs for saving results in file
    std::unique_ptr<INoradScheduleSaver> saver = std::make_unique<FileNoradScheduleSaver>(resultFilePath.toStdString());

    QFileInfo settingsFileInfo(settingsFilePath);
    // Create file if it doesn't exists
    if (!settingsFileInfo.exists()) {
        Utility::CreateSettingsFile(settingsFilePath);
        qInfo() << "[Main]: Created new settings file (settings.ini).";
    }


    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.beginGroup("TLEbySpaceTrack");
    QString login = settings.value("login").toString();
    QString pass = settings.value("pass").toString();
    settings.endGroup();
    if(login.isEmpty() or pass.isEmpty()){
        qCritical() << "[Main]:[settings]: no login or pass!";
    }

    settings.beginGroup("ObserverConfiguration");
    CTleProcessor tmp(std::move(saver), settings.value("Latitude").toDouble(), settings.value("Longitude").toDouble(), settings.value("Altitude").toDouble());
    uint numKA = settings.value("numKA").toUInt();
    uint dt_mks = settings.value("dt_mks").toUInt();
    // QObject::connect(&tmp, &CTleProcessor::allOperationsFinished, [](bool success) {
    //     if (!success) {
    //         qCritical() << "One or more operations failed. Check logs for details.";
    //     }
    //     qInfo() << "All operations completed, exiting...";
    //     QCoreApplication::exit(0);
    // });
    // Подключаем сигналы
    QObject::connect(&tmp, &CTleProcessor::tleDownloaded, [&](bool success) {
        if (!success) {
            qCritical() << "[Main]: Failed to download TLE. Exiting...";
            QCoreApplication::exit(1);
            return;
        }

        // Если загрузка успешна, загружаем и обрабатываем TLE
        tmp.loadTleFile(tleFilePath.toStdString());
        tmp.processTleData(numKA, dt_mks);

        // Завершаем приложение после обработки
        qInfo() << "[Main]: All operations completed, exiting...";
        QCoreApplication::exit(0);
    });

    tmp.downloadTleFromUrl(numKA, tleFilePath.toStdString(), login.toStdString(), pass.toStdString());

    /*tmp.loadTleFile(tleFilePath.toStdString());
    tmp.processTleData(settings.value("numKA").toUInt(), settings.value("dt_mks").toUInt());*/

    settings.endGroup();


    // // Sending formed file to PLC by SFTP
    // settings.beginGroup("SshConnection");
    // std::unique_ptr<IFileSenderPLC> sender = std::make_unique<SftpFileSender>(settings.value("host").toString(), settings.value("port").toUInt(),
    //                                                                           settings.value("login").toString(), settings.value("password").toString(),
    //                                                                           resultFilePath, resultFilePath);
    // settings.endGroup();
    // sender->send();

    return a.exec();



}
