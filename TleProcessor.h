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

    bool downloadTleFromUrl(const uint32_t satelliteNumber, const std::string& savePath = "TLE.txt");

    bool loadTleFile(const std::string& file);
    bool processTleData(const uint32_t satelliteNumber, const uint32_t dt_mks);

signals:
    void allOperationsFinished(bool success);

private:
    QScopedPointer<CNoradProcessor> m_noradPrc;
    std::shared_ptr<INoradScheduleSaver> m_noradSaver;
    std::vector<NORAD_SCHEDULE> m_vecNoradSchedule;
};

#endif // TLEPROCESSOR_H
