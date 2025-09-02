#include "QueryHandler.h"
#include <QDebug>

QueryHandler::QueryHandler(QObject *parent) : QObject(parent)
{}

QueryHandler::ErrorCodes QueryHandler::parsingPackage(const QByteArray &data) {
    // (1) Проверка размера пакета
    if (data.size() != 36) {
        qCritical() << "[QueryHandler]: Invalid packet size:" << data.size() << "Expected: 36";
        return INVALID_REQUEST;
    }
    // Cast the data to a pointer of the appropriate type for reading
    const char* rawData = data.constData();

    // Parse each field according to the specified format (Little-endian)
    m_data.lastServerTime = *reinterpret_cast<const uint32_t*>(rawData);
    m_data.n_points = *reinterpret_cast<const uint32_t*>(rawData + 4);      // Period (N_points)
    m_data.checksum = *reinterpret_cast<const uint32_t*>(rawData + 8);
    m_data.requestTime = *reinterpret_cast<const uint32_t*>(rawData + 12);
    m_data.noradId = *reinterpret_cast<const uint32_t*>(rawData + 16);      // NORAD ID
    m_data.reserved1 = *reinterpret_cast<const uint32_t*>(rawData + 20);     // Reserved field
    m_data.altitude = *reinterpret_cast<const uint32_t*>(rawData + 24);        // Changed to float
    m_data.latitude = *reinterpret_cast<const float*>(rawData + 28);
    m_data.longitude = *reinterpret_cast<const float*>(rawData + 32);

    // Проверяем данные после парсинга
    return validatePackage(data);
}

QueryHandler::ErrorCodes QueryHandler::validatePackage(const QByteArray &data) const
{

    // (1) Проверка контрольной суммы
    const uint32_t* words = reinterpret_cast<const uint32_t*>(data.constData());
    uint32_t calculatedChecksum = 0;
    for (int i = 3; i <= 8; ++i) {
        calculatedChecksum ^= words[i];
    }
    if (calculatedChecksum != m_data.checksum) {
        qCritical() << "[QueryHandler]: Checksum mismatch. Expected:" << m_data.checksum
                    << "Calculated:" << calculatedChecksum;
        return INVALID_REQUEST;
    }

    // (1) Проверка зарезервированного поля
    if (m_data.reserved1 != 0) {
        qCritical() << "[QueryHandler]: Invalid reserved field (must be 0):" << m_data.reserved1;
        return INVALID_REQUEST;
    }

    // (3) Проверка количества точек
    if (m_data.n_points == 0 || m_data.n_points > MAX_POINTS) {
        qCritical() << "[QueryHandler]: Invalid point count:" << m_data.n_points;
        return INVALID_POINT_COUNT;
    }

    // (1) Проверка координат
    if (m_data.latitude < -90.0f || m_data.latitude > 90.0f) {
        qCritical() << "[QueryHandler]: Invalid latitude:" << m_data.latitude;
        return INVALID_REQUEST;
    }
    if (m_data.longitude < -180.0f || m_data.longitude > 180.0f) {
        qCritical() << "[QueryHandler]: Invalid longitude:" << m_data.longitude;
        return INVALID_REQUEST;
    }
    if(m_data.altitude < 0){
        qCritical() << "[QueryHandler]: Invalid altitude:" << m_data.altitude;
        return INVALID_REQUEST;
    }

    // (7) Проверка времени (расхождение 3 минуты)
    uint32_t currentTime = QDateTime::currentSecsSinceEpoch();
    if (abs(static_cast<int64_t>(m_data.requestTime) - static_cast<int64_t>(currentTime)) > 180) {
        qCritical() << "[QueryHandler]: Time mismatch detected";
        return TIME_MISMATCH;
    }

    return NO_ERROR;
}

