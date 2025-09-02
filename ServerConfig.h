#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <QObject>
#include <QString>


class ServerConfig
{
public:
    struct Configuration {
        QString spaceTrackLogin;
        QString spaceTrackPassword;
        QString udpListenAddress;
        quint16 udpListenPort;
        QString tleApiUrl;
        uint16_t tleApiPort;
        QString tleCacheFile;
        QString logFilePath;
        uint32_t logsLifeTimeDays;
        uint32_t tleCacheHours;
        uint32_t maxPoints;
    };

    // Путь до файла с конфигурацией
    ServerConfig(const QString& filename);

    QString getSpaceTrackLogin() const {return m_config.spaceTrackLogin;};

    QString getSpaceTrackPassword() const {return m_config.spaceTrackPassword;};

    QString getUdpListenAddress() const {return m_config.udpListenAddress;};

    uint32_t getUdpListenPort() const {return m_config.udpListenPort;};

    QString getTleApiUrl() const {return m_config.tleApiUrl;};

    uint32_t getTleApiPort() const {return m_config.tleApiPort;};

    QString getTleFilePath() const {return m_config.tleCacheFile;};

    QString getLogFilePath() const {return m_config.logFilePath;};

    uint32_t getLogsLifeTime() const {return m_config.logsLifeTimeDays;};

    uint32_t getTleCacheHours() const {return m_config.tleCacheHours;};

    uint32_t getMaxPoints() const {return m_config.maxPoints;};

private:
    // Проверка корректности настроек
    void validate() const;

    Configuration m_config;
    QString configPath;

};

#endif // SERVERCONFIG_H
