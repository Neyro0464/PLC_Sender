#include "ReceiverUDP.h"
#include <QNetworkDatagram>
#include <QDebug>

ReceiverUDP::ReceiverUDP(QObject *parent) : QObject(parent)
{
    m_udpSocket = new QUdpSocket(this);
}

ReceiverUDP::~ReceiverUDP()
{
    if (m_udpSocket->state() == QAbstractSocket::BoundState) {
        m_udpSocket->close();
        disconnect(m_udpSocket, &QUdpSocket::readyRead, this, &ReceiverUDP::readPendingDatagrams);
    }
    delete m_udpSocket;
}

bool ReceiverUDP::startListening(quint16 port)
{
    if (m_udpSocket->state() == QAbstractSocket::BoundState) {
        return true; // Already listening
    }

    if (!m_udpSocket->bind(QHostAddress::Any, port)) {
        qCritical() <<"[ReceiverUdp::startListening]: failed to bind due to" << m_udpSocket->error();
        return false;
    }

    connect(m_udpSocket, &QUdpSocket::readyRead, this, &ReceiverUDP::readPendingDatagrams);
    return true;
}

void ReceiverUDP::readPendingDatagrams()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        QByteArray data = datagram.data();
        if(!data.isEmpty()){
            emit responseReceived(data);

        }
    }
}

