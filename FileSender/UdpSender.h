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
    struct Point {
        uint32_t Ti;    // DT (DateTime) как DWORD
        float Az;       // REAL
        float El;       // REAL
    };

    struct Atribute {
        uint32_t Ti;    // DT как DWORD
        uint32_t d1;    // DWORD
        uint32_t d2;    // DWORD
    };

    struct Sum {
        uint32_t Ti;    // DWORD
        uint32_t Az;    // DWORD
        uint32_t El;    // DWORD
    };

    struct Mas {
        Point point;
        Atribute atr;
        Sum sum;
    };

private:
    QUdpSocket* m_udpSocket;
    QHostAddress m_targetAddress;
    quint16 m_targetPort;
    uint32_t m_checkSum = 0;
    QVector<Mas> m_result;

    void calculateCheckSum();
    QByteArray prepareDataForSend() const;

    void convertFloatToDWORD(float value, uint32_t& result);

public:
    UdpSender(std::vector<NORAD_SCHEDULE>& data,
              const QHostAddress& targetAddress = QHostAddress::LocalHost,
              quint16 targetPort = 3545);
    ~UdpSender();

    QVector<Mas> getMas() const { return m_result; }
    bool sendData();
    uint32_t getCheckSum() const { return m_checkSum; }
    size_t getDataSize() const { return m_result.size() * sizeof(Mas) + sizeof(uint32_t) * 2; }
    void debugPrintData() const;

signals:
    void dataSent(bool success, qint64 bytesSent);
    void errorOccurred(const QString& errorMessage);
};

#endif // UDPSENDER_H
