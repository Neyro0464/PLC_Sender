#ifndef RECEIVERUDP_H
#define RECEIVERUDP_H

#include <QObject>
#include <QUdpSocket>
#include <QDateTime>
#include <QHostAddress>
#include <QQueue>

class ReceiverUDP : public QObject
{
    Q_OBJECT

public:

    explicit ReceiverUDP(QObject *parent = nullptr);
    ~ReceiverUDP();

    bool startListening(quint16 port = 3545);

signals:
    void responseReceived(const QByteArray&);

private slots:
    void readPendingDatagrams();

private:
    QUdpSocket *m_udpSocket;
    QQueue<QByteArray> qResponse;
};

#endif // RECEIVERUDP_H
