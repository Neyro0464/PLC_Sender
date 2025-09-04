#include "UdpListener.h"
#include <QDebug>

UdpListener::UdpListener(const QHostAddress& address, const quint16 port, QObject *parent)
    : QObject(parent)
    , listeningAddress(address)
    , listeningPort(port)
{
    socket = new QUdpSocket(this);
}

bool UdpListener::startListening() {
    if (socket->state() == QAbstractSocket::BoundState) {
        socket->close();
    }

    if (!socket->bind(listeningAddress, listeningPort)) {
        qCritical() << "[UdpListener] [startListening]: Failed to bind to"
                    << listeningAddress.toString() << ":" << listeningPort;
        return false;
    }

    connect(socket, &QUdpSocket::readyRead, this, &UdpListener::processPendingDatagrams);
    qInfo() << "[UdpListener] [startListening]: UDP Listener started on"
            << listeningAddress.toString() << ":" << listeningPort;
    return true;
}

void UdpListener::setAddress(const QHostAddress& address) {
    listeningAddress = address;
}

void UdpListener::setPort(const quint16 port) {
    listeningPort = port;
}

void UdpListener::processPendingDatagrams() {
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        qDebug() << "\n-------------------------------\n" << "Received UDP packet:\n" << datagram.toHex() << "\n-------------------------------";
        emit dataReceived(datagram, sender, senderPort);
    }
}

