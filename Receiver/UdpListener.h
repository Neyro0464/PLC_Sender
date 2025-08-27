#ifndef UDPLISTENER_H
#define UDPLISTENER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class UdpListener : public QObject {
    Q_OBJECT

public:
    explicit UdpListener(quint16 port, QObject *parent = nullptr);
    ~UdpListener();

    bool startListening();

signals:
    void dataReceived(const QByteArray &data, const QHostAddress &sender, quint16 senderPort);

private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *socket;
    quint16 listeningPort;
};

#endif // UDPLISTENER_H
