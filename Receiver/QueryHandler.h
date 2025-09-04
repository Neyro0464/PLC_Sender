#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H

#include <QObject>
#include <QDateTime>
#include <QByteArray>

class QueryHandler : public QObject {
    Q_OBJECT

public:
    struct ControllerHeader {
        uint32_t lastServerTime;
        uint32_t  n_points;
        uint32_t checksum;

        uint32_t requestTime;
        uint32_t noradId;
        uint32_t reserved1;

        uint32_t altitude;
        float latitude; // широта
        float longitude; // долгота
    };

    enum ErrorCodes {
        NO_ERROR = 0,
        INVALID_REQUEST = 1,
        NORAD_ID_NOT_FOUND = 2,
        INVALID_POINT_COUNT = 3,
        SPACE_TRACK_ERROR = 4,
        SGP4_CALCULATION_ERROR = 5,
        INTERNAL_ERROR = 6,
        TIME_MISMATCH = 7
    };

    explicit QueryHandler(QObject *parent = nullptr);

    QueryHandler::ErrorCodes parsingPackage(const QByteArray &data);

    uint32_t getLastServerTime() const {return m_data.lastServerTime;};
        uint32_t getPointsNumb() const {return m_data.n_points;};
    uint32_t getChecksum() const {return m_data.checksum;};

    uint32_t getRequestTime() const {return m_data.requestTime;};
    uint32_t getNoradId() const {return m_data.noradId;};
    uint32_t getReserved1() const {return m_data.reserved1;};


    uint32_t getAltitude() const {return m_data.altitude;};
    float getLatitude() const {return static_cast<float>(m_data.latitude);};
    float getLongitude() const {return static_cast<float>(m_data.longitude);};

    void setMaxPoints(uint32_t a) {MAX_POINTS = a;};


private:
    uint32_t MAX_POINTS;
    ErrorCodes validatePackage(const QByteArray &data) const;
    ControllerHeader m_data;

};
#endif // QUERYHANDLER_H
