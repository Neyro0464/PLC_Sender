#ifndef TLEPROCESSOR_H
#define TLEPROCESSOR_H

#include "NoradProcessor.h"
#include "ScheduleSaver/INoradScheduleSaver.h"
#include <QObject>


class CTleProcessor : public QObject
{
    Q_OBJECT
public:
    CTleProcessor(std::unique_ptr<INoradScheduleSaver> saver, double lat, double lon, double altm);

    // Celestrak site parsing
    // bool downloadTleFromUrl(const uint32_t satelliteNumber, const std::string& savePath = "TLE.txt");
    // Space-track site parsing
    bool downloadTleFromUrl(const uint32_t satelliteNumber, const QString& savePath, const QString& username, const QString& password, const uint16_t port, const QString url, const uint32_t tleCacheHours);

    bool loadTleFile(const std::string& file);
    bool processTleData(const uint32_t satelliteNumber, const uint32_t dt_sec, const uint32_t dt_delay = 0);

    const std::vector<NORAD_SCHEDULE>& getProcessedData() const { return m_vecNoradSchedule; }

signals:
    void tleDownloaded(bool success);

private:
    bool isTleFileValid(const QString& filePath, uint32_t satelliteNumber, const uint32_t tleCacheHours);

    QScopedPointer<CNoradProcessor> m_noradPrc;
    std::shared_ptr<INoradScheduleSaver> m_noradSaver;
    std::vector<NORAD_SCHEDULE> m_vecNoradSchedule;
};

#endif // TLEPROCESSOR_H
