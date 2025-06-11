#include <QSettings>
#include <QFileInfo>
#include <QDir>

#include "TleProcessor.h"
#include "FileNoradScheduleSaver.h"

int main()
{
    QString tleFilePath = "TLE.txt";
    QString settingsFilePath = "settings.ini";
    QString resultFilePath = "NoradSchedule.txt";

    std::unique_ptr<INoradScheduleSaver> saver = std::make_unique<FileNoradScheduleSaver>(resultFilePath.toStdString());

    QFileInfo settingsFileInfo(settingsFilePath);
    if (!settingsFileInfo.exists()) {
        QSettings settings(settingsFilePath, QSettings::IniFormat);
        settings.beginGroup("ObserverConfiguration");
        settings.setValue("dt_mks",  15000000);
        settings.setValue("numKA", 44714);
        settings.setValue("Latitude",  51.507406923983446);
        settings.setValue("Longitude", -0.12773752212524414);
        settings.setValue("Altitude", 0.05);
        settings.endGroup();
    }
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.beginGroup("ObserverConfiguration");

    CTleProcessor tmp(std::move(saver), settings.value("Latitude").toDouble(), settings.value("Longitude").toDouble(), settings.value("Altitude").toDouble());
    tmp.loadTleFile(tleFilePath.toStdString());
    tmp.processTleData(settings.value("numKA").toUInt());

    return 0;
}
