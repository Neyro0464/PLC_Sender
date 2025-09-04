#include "ServerConfig.h"
#include "./Utils/Utility.h"

#include <QSettings>
#include <QDir>
#include <QFile>
#include <QDebug>

ServerConfig::ServerConfig(const QString& filename){
    QFileInfo settingsFileInfo(filename);
    if (!settingsFileInfo.exists()) {
        Utility::CreateSettingsFile(filename);
        qInfo() << "[Main]: Created new settings file.";
    }

    QSettings settings(filename, QSettings::IniFormat);

    // Настройки прослушивания сервера (UDP)
    settings.beginGroup("UdpConnection");
    m_config.udpListenAddress= settings.value("host").toString();
    m_config.udpListenPort = settings.value("port").toUInt();
    settings.endGroup();

    // Настройки Space-Track
    settings.beginGroup("TLEbySpaceTrack");
    m_config.tleApiUrl = settings.value("url").toString();
    m_config.tleApiPort = settings.value("port").toUInt();
    m_config.spaceTrackLogin = settings.value("login").toString();
    m_config.spaceTrackPassword = settings.value("pass").toString();
    settings.endGroup();

    // Настройка основных параметров
    settings.beginGroup("GeneralSettings");
    m_config.tleCacheFile = settings.value("tleCachePath").toString();
    m_config.tleCacheHours = settings.value("lifeTimeOfTleCache_H").toUInt();
    m_config.logFilePath = settings.value("appLogPath").toString();
    m_config.logsLifeTimeDays = settings.value("lifeTimeOfLogs_DD").toUInt();
    m_config.maxPoints = settings.value("maxNumberOfPoints").toUInt();
    settings.endGroup();

    // settings.clear();

    validate();

    qInfo() << "Configuration successfull";
}

void ServerConfig::validate() const {
    // Проверка настроек прослушивания сервера
    if (m_config.udpListenPort == 0) {
        throw std::runtime_error("UDP listen port is required");
    }
    if (m_config.udpListenAddress.isEmpty()) {
        throw std::runtime_error("UDP listen address is required");
    }

    // Проверка настроек для скачивания TLE данных со Space-Track
    if (m_config.spaceTrackLogin.isEmpty()) {
        throw std::runtime_error("Space-Track login is required");
    }
    if (m_config.spaceTrackPassword.isEmpty()) {
        throw std::runtime_error("Space-Track password is required");
    }
    if (m_config.tleApiPort == 0) {
        throw std::runtime_error("Space-Track api port is required");
    }
    if (m_config.tleApiUrl.isEmpty()) {
        throw std::runtime_error("Space-Track api url is required");
    }

    // Проверка основных параметров
    if(m_config.tleCacheFile.isEmpty()){
        throw std::runtime_error("Tle cache file path is required");
    }
    if (m_config.maxPoints <= 0) {
        throw std::runtime_error("Max points must be greater than 0");
    }
    if (m_config.logFilePath.isEmpty()) {
        throw std::runtime_error("Log file path is required");
    }
    if (m_config.tleCacheHours <= 0) {
        throw std::runtime_error("Tle cache hours must be greater than 0");
    }
    if (m_config.logsLifeTimeDays <= 0) {
        throw std::runtime_error("Tle cache hours must be greater than 0");
    }

}
