#ifndef UTILRESPONSEPARSER_H
#define UTILRESPONSEPARSER_H

#include <QDateTime>
#include <QObject>
#include <QByteArray>

class UtilResponseParser
{
public:
    enum ErrorCode {
        NoError = 0,
        ChecksumError = 1,
        TimeMismatch = 2,
        WrongSatellite = 3,
        InsufficientData = 4,
        CalculationFailed = 5
    };

    enum DataFlag {
        ResponseRequest = 0,
        GeostationaryRecalculation = 1,
        GeostationaryPositioning = 2
    };

    struct ResponseHeader {
        QDateTime fileSendTime;    // [0],[0] - Time when file was sent (DT format)
        quint16 reserved;          // [0],[1] - Always 00
        quint32 checksum;          // [0],[2] - Checksum

        QDateTime responseTime;    // [1],[0] - Controller's response time (DT format)
        ErrorCode errorCode;       // [1],[1] - Error code of last file
        DataFlag dataFlag;         // [1],[2] - Data flag/command

        void print() const;
    };

public:
    // Правило пяти
    UtilResponseParser() = delete; // конструктор
    UtilResponseParser(const UtilResponseParser&) = delete; // конструктор копирования
    UtilResponseParser &operator=(const UtilResponseParser&) = delete; // операция присваивания
    UtilResponseParser (UtilResponseParser&&) noexcept; //конструктор переещения/передачи владения
    UtilResponseParser &operator=(UtilResponseParser&&) noexcept; //оператор присваивания перемещения

    static std::optional<ResponseHeader> parseResponse(const QByteArray &data);
    static QDateTime parseDTFormat(const QByteArray &data);

private:
    ResponseHeader m_header;
    QByteArray m_data;

signals:

public slots:


};

#endif // UTILRESPONSEPARSER_H
