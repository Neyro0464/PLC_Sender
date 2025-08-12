#include "UtilResponseParser.h"
#include <QDebug>


void UtilResponseParser::ResponseHeader::print() const
{
    qDebug() << "-----------------------------------------------";
    qDebug() << "\n RECEIVED DATA: \n";
    qDebug() << fileSendTime << reserved << checksum << "\n";
    qDebug() << responseTime << errorCode << dataFlag << "\n";
    qDebug() << "-----------------------------------------------";
}


std::optional<UtilResponseParser::ResponseHeader> UtilResponseParser::parseResponse(const QByteArray &data)
{
    ResponseHeader header;
    // Split the data into two lines (header rows)
    QList<QByteArray> lines = data.split('\n');
    if (lines.size() < 2) {
        qWarning() << "Invalid response format - expected 2 lines, got" << lines.size();
        return std::nullopt;
    }

    // Parse first line: [0],[0] [0],[1] [0],[2]
    QList<QByteArray> firstLine = lines[0].split(' ');
    if (firstLine.size() != 3) {
        qWarning() << "Invalid first line format - expected 3 elements, got" << firstLine.size();
        return std::nullopt;
    }

    header.fileSendTime = parseDTFormat(firstLine[0]);
    header.reserved = firstLine[1].toUShort();
    header.checksum = firstLine[2].toUInt();

    // Parse second line: [1],[0] [1],[1] [1],[2]
    QList<QByteArray> secondLine = lines[1].split(' ');
    if (secondLine.size() != 3) {
        qWarning() << "Invalid second line format - expected 3 elements, got" << secondLine.size();
        return std::nullopt;
    }

    header.responseTime = parseDTFormat(secondLine[0]);
    header.errorCode = static_cast<ErrorCode>(secondLine[1].toUShort());
    header.dataFlag = static_cast<DataFlag>(secondLine[2].toUShort());

    return header;
}



QDateTime UtilResponseParser::parseDTFormat(const QByteArray &data)
{
    bool ok;
    qint64 msecs = data.toLongLong(&ok);
    if (ok) {
        return QDateTime::fromMSecsSinceEpoch(msecs);
    }
    return QDateTime();
}
