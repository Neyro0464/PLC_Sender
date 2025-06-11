#include "TleProcessor.h"

CTleProcessor::CTleProcessor(std::unique_ptr<INoradScheduleSaver> saver, double lat, double lon, double altm)
{
    if (saver == nullptr)
        throw std::invalid_argument("Saver is nullptr");
    m_noradSaver = std::move(saver);
    m_noradPrc.reset(new CNoradProcessor(this, lat, lon, altm));
}

bool CTleProcessor::loadTleFile(const std::string& file)
{
    if(!m_noradPrc) return false;

    m_noradPrc->loadToBufferTLE(file);

    return true;
}

bool CTleProcessor::processTleData(const uint32_t satelliteNumber)
{
    if(!m_noradPrc) return false;

    m_noradPrc->genSchedule(satelliteNumber, m_vecNoradSchedule, m_noradSaver);

    return true;
}
