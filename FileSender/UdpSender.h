// UdpSender.h
#ifndef UDPSENDER_H
#define UDPSENDER_H

#include <QVector>
#include <QUdpSocket>
#include <QHostAddress>
#include "../ScheduleSaver/INoradScheduleSaver.h"

class UdpSender : public QObject
{
    Q_OBJECT

private:
#pragma pack(push, 1)
    struct HeaderRow {
        uint32_t col0;
        uint32_t col1;
        uint32_t col2;
    };

    struct DataRow {
        uint32_t time;
        float azimuth;
        float elevation;
    };

    union DataUnion {
        DataRow data;
        uint32_t words[3];
    };
#pragma pack(pop)

    QUdpSocket* m_udpSocket;
    QHostAddress m_targetAddress;
    quint16 m_targetPort;
    bool m_useExternalSocket; // Флаг для определения, используем ли внешний сокет

    HeaderRow m_header[2];
    QVector<DataRow> m_data;

    static inline uint32_t toLittleEndian32(uint32_t value);
    static inline float toLittleEndianFloat(float value);
    void calculateCheckSum();
    QByteArray prepareDataForSend() const;
    void closeSocket();

public:
    UdpSender(const std::vector<NORAD_SCHEDULE>& data,
              const uint32_t satelliteNumber,
              const QHostAddress& targetAddress,
              const quint16 targetPort,
              const uint32_t reserved1,
              const uint32_t statusCode,
              QUdpSocket* externalSocket = nullptr);
    ~UdpSender();

    bool sendData();
    size_t getDataSize() const {
        return (2 * sizeof(HeaderRow) + m_data.size() * sizeof(DataRow));
    }
    void debugPrintData() const;

signals:
    void dataSent(bool success, qint64 bytesSent);
    void errorOccurred(const QString& errorMessage);
};

#endif // UDPSENDER_H
