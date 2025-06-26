#ifndef NORADPROCESSOR_H
#define NORADPROCESSOR_H

#include <QObject>
#include <QMap>

#include <LIBS/Include/libsgp4/Tle.h>
#include <LIBS/Include/libsgp4/SGP4.h>
#include <LIBS/Include/libsgp4/Observer.h>
#include <LIBS/Include/libsgp4/CoordGeodetic.h>
#include <LIBS/Include/libsgp4/CoordTopocentric.h>

#include "ScheduleSaver/INoradScheduleSaver.h"


class CNoradProcessor : public QObject
{
    Q_OBJECT

public:
    enum NORAD_ERROR {
        NORAD_OK = 1,
        NORAD_MAP_EMPTY = -1,
        NORAD_BAD_TLE = -2,
        NORAD_BAD_TLEFILE = -3,
        NORAD_SAT_NOT_FOUND = -4,
        NORAD_SAVE_FAILED = -5
    };

    enum NORAD_CTRL {
        NORAD_IN_ZONE = 0,
        NORAD_LESS = 1,
        NORAD_MORE = 2,
        NORAD_LIMIT_HOURS = 24,
    };

    struct RAW_TLE {
        std::string s1;
        std::string s2;
        std::string s3;
    };

    explicit CNoradProcessor(QObject *parent = nullptr, double lat = 0, double lon = 0, double altm = 0);

    NORAD_ERROR genSchedule(const uint32_t &satelliteNumber, std::vector<NORAD_SCHEDULE>& vecNoradSchedule,
                            std::shared_ptr<INoradScheduleSaver> saver,
                            const int delayMks = 0, libsgp4::DateTime onDate = libsgp4::DateTime::Now(true), const int posCalcDelaySec = 2) const;

    NORAD_ERROR loadToBufferTLE(const std::string &fileName);

private:
    double m_Lat0;
    double m_Lon0;
    double m_H0;

    QMap<uint32_t, RAW_TLE> m_SatTleMap; // number: node
};

#endif // NORADPROCESSOR_H
