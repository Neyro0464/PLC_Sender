#include <QCoreApplication>
#include <QSettings>
#include <QFileInfo>
#include <QTimer>
#include <QDir>

#include "TleProcessor.h"
#include "ScheduleSaver/FileNoradScheduleSaver.h"
#include "Utils/Utility.h"
#include "./Receiver/UdpListener.h"
#include "./Receiver/QueryHandler.h"
#include "./FileSender/UdpSender.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    // Настройка логирования
    Utility::ClearOldLogs("./ServiceFiles/app.log", 5);
    qInstallMessageHandler(Utility::СustomMessageHandler);


    QString tleFilePath = "ServiceFiles/definiteTLE.txt";
    QString settingsFilePath = "ServiceFiles/settings.ini";
    QString resultFilePath = "ServiceFiles/NoradSchedule.txt";

    // Загрузка настроек
    QFileInfo settingsFileInfo(settingsFilePath);
    if (!settingsFileInfo.exists()) {
        Utility::CreateSettingsFile(settingsFilePath);
        qInfo() << "[Main]: Created new settings file (settings.ini).";
    }

    QSettings settings(settingsFilePath, QSettings::IniFormat);

    // Настройки подключения UDP
    settings.beginGroup("UdpConnection");
    QString udpHost = settings.value("host").toString();
    quint16 udpPort = settings.value("port").toUInt();
    settings.endGroup();

    // Настройки наблюдателя
    settings.beginGroup("ObserverConfiguration");
    double latitude = settings.value("Latitude").toDouble();
    double longitude = settings.value("Longitude").toDouble();
    double altitude = settings.value("Altitude").toDouble();
    uint numKA = settings.value("numKA").toUInt();
    uint dt_mks = settings.value("dt_mks").toUInt();
    uint dt_delay = settings.value("dt_delay_s").toUInt();
    settings.endGroup();

    // Настройки Space-Track
    settings.beginGroup("TLEbySpaceTrack");
    QString login = settings.value("login").toString();
    QString pass = settings.value("pass").toString();
    settings.endGroup();

    if(login.isEmpty() || pass.isEmpty()){
        qCritical() << "[Main]:[settings]: no login or pass!";
    }

    // Создаем UDP listener
    UdpListener udpListener(udpPort);
    if (!udpListener.startListening()) {
        qCritical() << "[Main]: Failed to start UDP listener";
        return 1;
    }

    // Обработчик запросов
    QueryHandler queryHandler;

    // Обработчик TLE
    std::unique_ptr<INoradScheduleSaver> saver = std::make_unique<FileNoradScheduleSaver>(resultFilePath.toStdString());
    CTleProcessor tleProcessor(std::move(saver), latitude, longitude, altitude);

    // Обработка входящих UDP пакетов
    QObject::connect(&udpListener, &UdpListener::dataReceived, [&](const QByteArray &data, const QHostAddress &sender, quint16 senderPort) {
        qInfo() << "[Main]: Received UDP packet from" << sender.toString() << ":" << senderPort;

        // Проверяем корректность пакета
        QueryHandler::ErrorCodes errorCode = queryHandler.validatePacket(data);

        if (errorCode != QueryHandler::NO_ERROR) {
            qCritical() << "[Main]: Invalid packet received, error code:" << errorCode;
            return; // Продолжаем слушать порт
        }

        // Если пакет корректен, загружаем и обрабатываем TLE данные
        QObject::connect(&tleProcessor, &CTleProcessor::tleDownloaded, [&](bool success) {
            if (!success) {
                qCritical() << "[Main]: Failed to download TLE";
                return;
            }

            tleProcessor.loadTleFile(tleFilePath.toStdString());
            tleProcessor.processTleData(numKA, dt_mks, dt_delay);
            std::vector<NORAD_SCHEDULE> scheduleData = tleProcessor.getProcessedData();

            // Формируем и отправляем ответ
            UdpSender udpSender(scheduleData, numKA, sender, senderPort);

            if (udpSender.sendData()) {
                qInfo() << "[Main]: Response sent successfully to" << sender.toString() << ":" << senderPort;
            } else {
                qCritical() << "[Main]: Failed to send response";
            }
        });

        // Запускаем загрузку TLE данных
        tleProcessor.downloadTleFromUrl(numKA, tleFilePath.toStdString(), login.toStdString(), pass.toStdString());
    });

    qInfo() << "[Main]: Application started, listening on UDP port" << udpPort;

    return a.exec();
}
