#include "UdpSender.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

// UdpSender.cpp
UdpSender::UdpSender(const std::vector<NORAD_SCHEDULE>& data,
                     const uint32_t satelliteNumber,
                     const QHostAddress& targetAddress,
                     const quint16 targetPort,
                     const uint32_t numberOfPoints,
                     const uint32_t reserved1,
                     const uint32_t statusCode)
    : m_targetAddress(targetAddress), m_targetPort(targetPort),  m_socketBound(false)
{
    qDebug() << "[UdpSender]: Constructor started";

    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(3545);

    try {
        // Текущее время для заголовков
        uint32_t currentTime = libsgp4::DateTime::Now().SecondsFromTicks();

        // Первая строка заголовка [0]
        m_header[0].col0 = currentTime;
        m_header[0].col1 = static_cast<uint32_t>(data.size());
        m_header[0].col1 = static_cast<uint32_t>(numberOfPoints);

        m_header[0].col2 = 0; // checksum

        // Вторая строка заголовка [1]
        m_header[1].col0 = reserved1;
        m_header[1].col1 = satelliteNumber;
        m_header[1].col2 = statusCode; // status code

        // Заполняем данные точек
        m_data.resize(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            m_data[i].time = data[i].onDate.SecondsFromTicks();
            m_data[i].azimuth = data[i].azm;
            m_data[i].elevation = data[i].elv;
        }


        calculateCheckSum();

    } catch (const std::exception& e) {
        qCritical() << "[UdpSender]: Exception in constructor:" << e.what();
        throw;
    }
}

void UdpSender::calculateCheckSum()
{


    // Сначала XOR второй строки заголовка
    uint32_t checksum = m_header[1].col0 ^ m_header[1].col1 ^ m_header[1].col2;

    if (!m_data.empty()){
        // Теперь обрабатываем все данные через union
        DataUnion u;
        for (const auto& point : m_data) {
            // Копируем данные в union
            u.data.time = point.time;
            u.data.azimuth = point.azimuth;
            u.data.elevation = point.elevation;

            // XOR всех слов
            checksum ^= u.words[0];  // time
            checksum ^= u.words[1];  // azimuth как uint32_t
            checksum ^= u.words[2];  // elevation как uint32_t
        }
    }

    // Записываем результат в заголовок
    m_header[0].col2 = checksum;

    // qDebug() << "[UdpSender]: Checksum calculated:"
    //          << QString("0x%1").arg(checksum, 8, 16, QChar('0'));

    // // Отладочный вывод для проверки
    // DataUnion debug;
    // debug.data.azimuth = m_data[0].azimuth;
    // qDebug() << "First azimuth as float:" << m_data[0].azimuth
    //          << "as uint32:" << QString("0x%1").arg(debug.words[1], 8, 16, QChar('0'));
}

QByteArray UdpSender::prepareDataForSend() const
{
    const size_t totalSize = 2 * sizeof(HeaderRow) + m_data.size() * sizeof(DataRow);
    QByteArray data(totalSize, 0);
    uint8_t* ptr = reinterpret_cast<uint8_t*>(data.data());

    // Записываем заголовок
    for (int i = 0; i < 2; i++) {
        uint32_t* dest = reinterpret_cast<uint32_t*>(ptr + i * sizeof(HeaderRow));
        dest[0] = toLittleEndian32(m_header[i].col0);
        dest[1] = toLittleEndian32(m_header[i].col1);
        dest[2] = toLittleEndian32(m_header[i].col2);
    }

    // Записываем данные через union для гарантии правильного представления
    uint8_t* dataPtr = ptr + 2 * sizeof(HeaderRow);
    DataUnion u;

    for (const auto& point : m_data) {
        u.data.time = point.time;
        u.data.azimuth = point.azimuth;
        u.data.elevation = point.elevation;

        // Преобразуем каждое слово в little-endian
        uint32_t* dest = reinterpret_cast<uint32_t*>(dataPtr);
        dest[0] = toLittleEndian32(u.words[0]);
        dest[1] = toLittleEndian32(u.words[1]);
        dest[2] = toLittleEndian32(u.words[2]);

        dataPtr += sizeof(DataRow);
    }

    return data;
}

void UdpSender::debugPrintData() const
{
    QFile file("NoradSchedule.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Could not open NoradSchedule.txt for writing";
        return;
    }

    QTextStream out(&file);

    // Заголовок
    for (int i = 0; i < 2; i++) {
        out << QString("[%1],[0] %2 [%1],[1] %3 [%1],[2] %4\n")
                   .arg(i)
                   .arg(m_header[i].col0)
                   .arg(m_header[i].col1)
                   .arg(m_header[i].col2);
    }

    out << "\n";

    // Данные точек
    for (int i = 0; i < m_data.size(); i++) {
        out << QString("[%1],[0] %2 [%1],[1] %3 [%1],[2] %4\n")
                   .arg(i + 2)
                   .arg(m_data[i].time)
                   .arg(m_data[i].azimuth, 0, 'f', 3)
                   .arg(m_data[i].elevation, 0, 'f', 3);
    }

    file.close();
}

uint32_t UdpSender::toLittleEndian32(uint32_t value) {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    return value;
#else
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x000000FF) << 24);
#endif
}

float UdpSender::toLittleEndianFloat(float value) {
    union {
        float f;
        uint32_t i;
    } u;
    u.f = value;
    u.i = toLittleEndian32(u.i);
    return u.f;
}

bool UdpSender::sendData()
{
    closeSocket();

    // if (m_data.empty()) {
    //     qWarning() << "[UdpSender]: No data to send";
    //     emit errorOccurred("No data to send");
    //     return false;
    // }

    debugPrintData();

    QByteArray dataToSend = prepareDataForSend();
    if (dataToSend.isEmpty()) {
        qWarning() << "[UdpSender]: Prepared data is empty";
        emit errorOccurred("Prepared data is empty");
        return false;
    }

    qint64 bytesSent = m_udpSocket->writeDatagram(dataToSend, m_targetAddress, m_targetPort);

    if (bytesSent == -1) {
        QString errorMsg = QString("Failed to send UDP data: %1").arg(m_udpSocket->errorString());
        qCritical() << "[UdpSender]:" << errorMsg;
        emit errorOccurred(errorMsg);
        emit dataSent(false, bytesSent);
        return false;
    }

    qInfo() << "[UdpSender]: Successfully sent" << bytesSent << "bytes to"
            << m_targetAddress.toString() << ":" << m_targetPort;
    emit dataSent(true, bytesSent);
    return true;
}

void UdpSender::closeSocket()
{
    if (m_udpSocket && m_socketBound) {
        m_udpSocket->close();
        m_socketBound = false;
        qDebug() << "[UdpSender]: Socket closed";
    }
}

UdpSender::~UdpSender()
{
    qDebug() << "[UdpSender]: Destructor called";
    if (m_udpSocket) {
        m_udpSocket->close();
        delete m_udpSocket;
        m_socketBound = false;
        m_udpSocket = nullptr;
    }
}
