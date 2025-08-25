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
#pragma pack(push, 1)  // Важно для правильного выравнивания
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
        struct {
            uint32_t time;
            float azimuth;
            float elevation;
        } data;
        uint32_t words[3];  // 3 слова по 4 байта
    };
#pragma pack(pop)

    QUdpSocket* m_udpSocket;
    QHostAddress m_targetAddress;
    quint16 m_targetPort;

    // Отдельные массивы для заголовка и данных
    HeaderRow m_header[2];  // Две строки заголовка
    QVector<DataRow> m_data;// Данные точек

    // Приватные методы для работы с endianness
    static inline uint32_t toLittleEndian32(uint32_t value);

    static inline float toLittleEndianFloat(float value);

    void calculateCheckSum();
    QByteArray prepareDataForSend() const;

public:
    UdpSender(std::vector<NORAD_SCHEDULE>& data,
              uint32_t satelliteNumber,
              const QHostAddress& targetAddress = QHostAddress::LocalHost,
              quint16 targetPort = 3545);
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
