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
        uint32_t dt;    // 4 байта - временная метка в тиках
        float Az;       // 4 байта - азимут
        float El;       // 4 байта - угол места
    };

    struct Atribute {
        uint32_t dt;    // 4 байта - временная метка
        uint32_t d1;    // 4 байта - дополнительное поле 1
        uint32_t d2;    // 4 байта - дополнительное поле 2
    };

    struct Sum {
        uint32_t dt;    // 4 байта - сумма временной метки
        uint32_t Az;    // 4 байта - сумма азимута
        uint32_t El;    // 4 байта - сумма угла места
    };

    struct Mas {
        Point point;    // 12 байт
        Atribute atr;   // 12 байт
        Sum sum;        // 12 байт
        // Итого: 36 байт на одну структуру Mas
    };

private:
    QUdpSocket* m_udpSocket;
    QHostAddress m_targetAddress;
    quint16 m_targetPort;
    uint32_t m_checkSum = 0;
    QVector<Mas> m_result;

    void calculateCheckSum();
    QByteArray prepareDataForSend() const;

public:
    UdpSender(std::vector<NORAD_SCHEDULE>& data,
              const QHostAddress& targetAddress = QHostAddress::LocalHost,
              quint16 targetPort = 3545);
    ~UdpSender();

    QVector<Mas> getMas() const { return m_result; }
    bool sendData();
    uint32_t getCheckSum() const { return m_checkSum; }
    size_t getDataSize() const { return m_result.size() * sizeof(Mas) + sizeof(uint32_t) * 2; }

signals:
    void dataSent(bool success, qint64 bytesSent);
    void errorOccurred(const QString& errorMessage);
};

#endif // UDPSENDER_H
