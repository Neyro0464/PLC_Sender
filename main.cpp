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
#include "ServerConfig.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);


    // Загрузка конфигурации

    ServerConfig config("./ServiceFiles/settings.ini");

    // --------------------------------------------------


    // Настройка логирования

    Utility::ClearOldLogs(config.getLogFilePath(), config.getLogsLifeTime());
    qInstallMessageHandler(Utility::СustomMessageHandler);

    // --------------------------------------------------

    // Загрузка настроек
    // QFileInfo settingsFileInfo(settingsFilePath);
    // if (!settingsFileInfo.exists()) {
    //     Utility::CreateSettingsFile(settingsFilePath);
    //     qInfo() << "[Main]: Created new settings file (settings.ini).";
    // }

    // QSettings settings(settingsFilePath, QSettings::IniFormat);

    // // Настройки подключения UDP
    // settings.beginGroup("UdpConnection");
    // QString udpHost = settings.value("host").toString();
    // quint16 udpPort = settings.value("port").toUInt();
    // settings.endGroup();

    // // Настройки наблюдателя
    // settings.beginGroup("ObserverConfiguration");
    // double latitude = settings.value("Latitude").toDouble();
    // double longitude = settings.value("Longitude").toDouble();
    // double altitude = settings.value("Altitude").toDouble();
    // uint numKA = settings.value("numKA").toUInt();
    // uint dt_mks = settings.value("dt_mks").toUInt();
    // uint dt_delay = settings.value("dt_delay_s").toUInt();
    // settings.endGroup();

    // // Настройки Space-Track
    // settings.beginGroup("TLEbySpaceTrack");
    // QString login = settings.value("login").toString();
    // QString pass = settings.value("pass").toString();
    // settings.endGroup();

    // if(login.isEmpty() || pass.isEmpty()){
    //     qCritical() << "[Main]:[settings]: no login or pass!";
    // }

    // Создаем UDP listener
    UdpListener udpListener(QHostAddress(config.getUdpListenAddress()), config.getUdpListenPort());
    if (!udpListener.startListening()) {
        qCritical() << "[Main]: Failed to start UDP listener";
        return 1;
    }



    // Обработка входящих UDP пакетов
    QObject::connect(&udpListener, &UdpListener::dataReceived, [&](const QByteArray &data, const QHostAddress &sender, quint16 senderPort) {
        qInfo() << "[Main]: Received UDP packet from" << sender.toString() << ":" << senderPort;

        std::vector<NORAD_SCHEDULE> scheduleData{};
        // Обработчик запросов
        QueryHandler queryHandler;
        queryHandler.setMaxPoints(config.getMaxPoints());

        // Проверяем корректность пакета
        QueryHandler::ErrorCodes errorCode = queryHandler.parsingPackage(data);
        if(errorCode != QueryHandler::ErrorCodes::NO_ERROR){
            // Формируем и отправляем ответ
            // Точки измерений, Номер борта, адрес получателя, порт получателя, количество точек, резервное значение, код статуса
            UdpSender udpSender(scheduleData, queryHandler.getNoradId(), sender, senderPort, queryHandler.getPointsNumb(), 0, errorCode);

            if (udpSender.sendData()) {
                qInfo() << "[Main]: Response sent successfully to" << sender.toString() << ":" << senderPort;
            } else {
                qCritical() << "[Main]: Failed to send response";
            }
        }
        else
        {

            uint32_t interval = 0;
            if(queryHandler.getPointsNumb() == 1){
                interval = 0;
            } else {
            // seconds * minuts * hours / (number of points - 1)
            interval = (60 * 60 * 24) / (queryHandler.getPointsNumb() - 1);
            }

            // Обработчик TLE
            std::unique_ptr<INoradScheduleSaver> saver = std::make_unique<FileNoradScheduleSaver>(config.getTleFilePath().toStdString());
            CTleProcessor tleProcessor(std::move(saver), queryHandler.getLatitude(), queryHandler.getLongitude(), queryHandler.getAltitude());

            // Если пакет корректен, загружаем и обрабатываем TLE данные
            QObject::connect(&tleProcessor, &CTleProcessor::tleDownloaded, [&](bool success) {

                // Начиная отсюда посмотреть коды ошибок
                if (!success) {
                    qCritical() << "[Main]: Failed to download TLE";
                    return;
                }

                tleProcessor.loadTleFile(config.getTleFilePath().toStdString());
                tleProcessor.processTleData(queryHandler.getNoradId(), interval);
                scheduleData = tleProcessor.getProcessedData();

                // Формируем и отправляем ответ
                UdpSender udpSender(scheduleData, queryHandler.getNoradId(), sender, senderPort, queryHandler.getPointsNumb(), 0, errorCode);

                if (udpSender.sendData()) {
                    qInfo() << "[Main]: Response sent successfully to" << sender.toString() << ":" << senderPort;
                } else {
                    qCritical() << "[Main]: Failed to send response";
                }
            });

            // Запускаем загрузку TLE данных
            tleProcessor.downloadTleFromUrl(queryHandler.getNoradId(), config.getTleFilePath(), config.getSpaceTrackLogin(), config.getSpaceTrackPassword(),
                                            config.getTleApiPort(), config.getTleApiUrl(), config.getTleCacheHours());
        }
    });

    qInfo() << "[Main]: Application started, listening on" <<config.getUdpListenAddress() << ":" << config.getUdpListenPort();

    return a.exec();
}
