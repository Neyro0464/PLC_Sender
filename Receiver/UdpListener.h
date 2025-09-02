#ifndef UDPLISTENER_H
#define UDPLISTENER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class UdpListener : public QObject {
    Q_OBJECT
public:
    UdpListener(const QHostAddress& address = QHostAddress::Any,
                const quint16 port = 0,
                QObject *parent = nullptr);
    ~UdpListener() = default;

    bool startListening();
    // Можно добавить методы для изменения адреса и порта
    void setAddress(const QHostAddress& address);
    void setPort(const quint16 port);

private:
    QUdpSocket* socket;
    QHostAddress listeningAddress;
    quint16 listeningPort;

private slots:
    void processPendingDatagrams();

signals:
    void dataReceived(const QByteArray& data, const QHostAddress& sender, quint16 senderPort);
};

#endif // UDPLISTENER_H
