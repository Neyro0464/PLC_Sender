#include <QCoreApplication>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

#include "TleProcessor.h"
#include "ScheduleSaver/FileNoradScheduleSaver.h"
#include "FileSender/SftpFileSender.h"

int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);


    QString tleFilePath = "TLE.txt";                // File with TLE info
    QString settingsFilePath = "settings.ini";      // File with settings
    QString resultFilePath = "NoradSchedule.txt";   // File with results for one satellite

    // classs for saving results in file
    std::unique_ptr<INoradScheduleSaver> saver = std::make_unique<FileNoradScheduleSaver>(resultFilePath.toStdString());

    QFileInfo settingsFileInfo(settingsFilePath);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    // Create file if it doesn't exists
    if (!settingsFileInfo.exists()) {
        settings.beginGroup("ObserverConfiguration");
        settings.setValue("dt_mks",  15000000);
        settings.setValue("numKA", 44714);
        settings.setValue("Latitude",  51.507406923983446);
        settings.setValue("Longitude", -0.12773752212524414);
        settings.setValue("Altitude", 0.05);
        settings.endGroup();

        settings.beginGroup("SshConnection");
        settings.setValue("host",  "127.0.0.1");
        settings.setValue("port", 99);
        settings.setValue("login",  "login");
        settings.setValue("pass", "password");
        settings.endGroup();
    }

    settings.beginGroup("ObserverConfiguration");
    CTleProcessor tmp(std::move(saver), settings.value("Latitude").toDouble(), settings.value("Longitude").toDouble(), settings.value("Altitude").toDouble());
    tmp.downloadTleFromUrl(settings.value("numKA").toUInt(), "tmpTLE.txt");
    tmp.loadTleFile(tleFilePath.toStdString());
    tmp.processTleData(settings.value("numKA").toUInt());
    settings.endGroup();

    // // Sending formed file to PLC by SFTP
    // settings.beginGroup("SshConnection");
    // qDebug() << settings.value("host").toString() << settings.value("port").toUInt() <<
    //     settings.value("login").toString() << settings.value("pass").toString() <<
    //     resultFilePath << resultFilePath;
    // std::unique_ptr<IFileSenderPLC> sender = std::make_unique<SftpFileSender>(settings.value("host").toString(), settings.value("port").toUInt(),
    //                                                                           settings.value("login").toString(), settings.value("password").toString(),
    //                                                                           resultFilePath, resultFilePath);
    // settings.endGroup();
    // sender->send();

    return a.exec();
}
