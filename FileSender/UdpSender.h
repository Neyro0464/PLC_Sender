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
    struct DataRow {
        uint32_t col0;  // DT или время
        float col1;     // Азимут или другие данные
        float col2;     // Угол места или другие данные
    };

    QUdpSocket* m_udpSocket;
    QHostAddress m_targetAddress;
    quint16 m_targetPort;
    QVector<DataRow> m_data;

    void calculateCheckSum();
    QByteArray prepareDataForSend() const;

public:
    UdpSender(std::vector<NORAD_SCHEDULE>& data,
              uint32_t satelliteNumber,
              const QHostAddress& targetAddress = QHostAddress::LocalHost,
              quint16 targetPort = 3545);
    ~UdpSender();

    bool sendData();
    size_t getDataSize() const { return m_data.size() * sizeof(DataRow); }
    void debugPrintData() const;

signals:
    void dataSent(bool success, qint64 bytesSent);
    void errorOccurred(const QString& errorMessage);
};

#endif // UDPSENDER_H
