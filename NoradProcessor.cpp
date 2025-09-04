#include <fstream>
#include <QDebug>

#include "NoradProcessor.h"


CNoradProcessor::CNoradProcessor(QObject *parent, double lat, double lon, double altm)
    : QObject{parent}, m_Lat0{lat}, m_Lon0{lon}, m_H0{altm}, m_SatTleMap{}
{}

CNoradProcessor::NORAD_ERROR CNoradProcessor::genSchedule(const uint32_t &satelliteNumber,
                                                          std::vector<NORAD_SCHEDULE> &vecNoradSchedule,
                                                          std::shared_ptr<INoradScheduleSaver> saver,
                                                          const uint32_t posCalcDelayMin,
                                                          const int delaySec,
                                                          libsgp4::DateTime onDate) const
{
    if (m_SatTleMap.empty()) return NORAD_MAP_EMPTY;

    auto it = m_SatTleMap.find(satelliteNumber);
    if (it == m_SatTleMap.end()) return NORAD_SAT_NOT_FOUND;

    const RAW_TLE& rawStr = it.value();
    vecNoradSchedule.clear();

    try {

        libsgp4::Tle tle(rawStr.s1, rawStr.s2, rawStr.s3);
        libsgp4::Observer obs(m_Lat0, m_Lon0, m_H0/1000.);
        libsgp4::SGP4 sgp4(tle);

        onDate = onDate.AddSeconds(delaySec);
        libsgp4::DateTime endDate = onDate.AddHours(NORAD_LIMIT_HOURS);

        long stepMin = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::minutes(posCalcDelayMin)).count();

        if(posCalcDelayMin == 0){
            stepMin = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::hours(25)).count();
        }


        while(1) {

            libsgp4::Eci eci = sgp4.FindPosition(onDate);
            libsgp4::CoordTopocentric topo = obs.GetLookAngle(eci);

            vecNoradSchedule.push_back(NORAD_SCHEDULE(  libsgp4::Util::RadiansToDegrees(topo.azimuth),
                                                        libsgp4::Util::RadiansToDegrees(topo.elevation),
                                                        onDate));

            onDate = onDate.AddMicroseconds(stepMin);

            if(onDate.Compare(endDate) >= 0) break;
        }

    } catch (libsgp4::TleException& e) {

        qDebug() << "Error: " << e.what();

        return NORAD_BAD_TLE;
    }

    if (!saver->save(vecNoradSchedule)) {
        return NORAD_SAVE_FAILED;
    }


    return NORAD_OK;
}

CNoradProcessor::NORAD_ERROR CNoradProcessor::loadToBufferTLE(const std::string &fileName)
{ // WARNING!!! 3 line only format

    std::ifstream file(fileName);

    m_SatTleMap.clear();

    if (!file.is_open()) {

        return NORAD_BAD_TLEFILE;
    }

    while (!file.eof()) {

        std::string line[3];
        for(int i = 0; i < 3; ++i) {

            std::getline(file, line[i]);
            libsgp4::Util::Trim(line[i]);

            if (line[i].length() != libsgp4::Tle::LineLength()
                && i > 0
                && line[i].length() > 0)    {

                return NORAD_BAD_TLEFILE;
            }
        }

        if(line[0].length() == 0) continue;

        libsgp4::Tle tle(line[0], line[1], line[2]);

        RAW_TLE rawTle;
        rawTle.s1 = line[0];
        rawTle.s2 = line[1];
        rawTle.s3 = line[2];

        m_SatTleMap[tle.NoradNumber()] = rawTle;
    }

    return NORAD_OK;
}
