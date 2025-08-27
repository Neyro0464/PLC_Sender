#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H

#include <QObject>
#include <QDateTime>
#include <QByteArray>

class QueryHandler : public QObject {
    Q_OBJECT

public:
    struct ControllerHeader {
        QDateTime lastServerTime;
        int reserved1;
        int checksum;
        QDateTime requestTime;
        int satelliteNumber;
        int statusCode;
    };

    enum ErrorCodes {
        NO_ERROR = 0,
        CHECKSUM_ERROR = 1,
        INVALID_FORMAT = 2,
        SATELLITE_NOT_FOUND = 3,
        INVALID_STATUS_CODE = 4
    };

    explicit QueryHandler(QObject *parent = nullptr);

    ErrorCodes validatePacket(const QByteArray &data) const;

private:
    bool validateChecksum(const QByteArray &data) const;
    bool validateFormat(const QByteArray &data) const;
};
#endif // QUERYHANDLER_H
