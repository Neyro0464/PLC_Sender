#include <fstream>
#include "Utility.h"
#include "FileNoradScheduleSaver.h"
#include <cmath>

bool FileNoradScheduleSaver::save(const std::vector<NORAD_SCHEDULE>& vecNoradSchedule) const {
    std::ofstream outFile(m_filePath);
    if (!outFile.is_open()) {
        return false;
    }
    QString date = TrimUTC(libsgp4::DateTime::Now());
    QString resultLine{};

    //Reserved values:
    int reserved1 = 0;
    int reserved2 = 0;
    int reserved3 = 0;

    uint32_t checksum{};
    checksum ^= Utility::CalcChecksum(reserved1, reserved2, reserved3);
    for (const auto& schedule : vecNoradSchedule) {
        checksum ^= Utility::CalcChecksum(schedule.onDate.Ticks(), schedule.azm*1000, schedule.elv*1000);
    }
    resultLine = date + ';' + QString::number(vecNoradSchedule.size()) + ';' + QString::number(checksum) + ';';
    FullfilLine(resultLine);
    outFile << resultLine.toStdString();
    resultLine = QString::number(reserved1) + ';' + QString::number(reserved2) + ';' + QString::number(reserved3) + ';';
    FullfilLine(resultLine);
    outFile << resultLine.toStdString();

    for (const auto& schedule : vecNoradSchedule) {// 37 symbols on the line
        date = TrimUTC(schedule.onDate);
        resultLine = date + ';' + QString::number(schedule.azm) + ';' + QString::number(schedule.elv) + ';';
        FullfilLine(resultLine);
        outFile << resultLine.toStdString();
    }
    outFile.close();
    return true;
}

bool FileNoradScheduleSaver::saveStep(  const float estAzm, const float curAzm,
                                        const float estElv, const float curElv,
                                        const float errAzm, const float errElv
                                      ) const
{
    std::ofstream outFile(m_filePath, std::ios::app);
    if (!outFile.is_open()) {
        return false;
    }

    outFile << estAzm << "," << curAzm << ","
            << estElv << "," << curElv << ","
            << errAzm << "," << errElv
            << ";\n";


    outFile.close();
    return true;
}

bool FileNoradScheduleSaver::clear(){
    std::ofstream(m_filePath).close();
    return !std::ifstream(m_filePath).fail();
}

bool FileNoradScheduleSaver::load(std::vector<NORAD_SCHEDULE>& vecNoradSchedule) const {
    std::ifstream inFile(m_filePath);
    if (!inFile.is_open()) {
        return false;
    }

    vecNoradSchedule.clear();
    std::string line;

    while (std::getline(inFile, line)) {

        while (!line.empty() && (line.back() == ';' || line.back() == ' ' || line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }

        size_t comma1 = line.find(',');
        size_t comma2 = line.find(',', comma1 + 1);

        if (comma1 == std::string::npos || comma2 == std::string::npos) {
            continue;
        }

        std::string dateStr = line.substr(0, comma1);
        std::string azmStr = line.substr(comma1 + 1, comma2 - comma1 - 1);
        std::string elvStr = line.substr(comma2 + 1);

        trim(dateStr);
        trim(azmStr);
        trim(elvStr);

        int year, month, day, hour, minute, second, microsecond;
        if (!parseDateTime(dateStr, year, month, day, hour, minute, second, microsecond)) {
            continue;
        }

        NORAD_SCHEDULE schedule;
        schedule.onDate.Initialise(year, month, day, hour, minute, second, microsecond);

        std::setlocale(LC_NUMERIC, "C");

        try {
            schedule.azm = std::stof(azmStr);
            schedule.elv = std::stof(elvStr);
            vecNoradSchedule.push_back(schedule);
        } catch (const std::exception& e) {
            // std::cerr << "Error parsing numbers: " << e.what() << std::endl;
            continue;
        }
    }

    inFile.close();
    return true;
}

void FileNoradScheduleSaver::trim(std::string &str) {
    str.erase(0, str.find_first_not_of(" \t"));
    str.erase(str.find_last_not_of(" \t") + 1);
}

QString FileNoradScheduleSaver::TrimUTC(const libsgp4::DateTime onDate){
    QString res = QString::fromStdString(onDate.ToString());
    res.remove(".000000 UTC");
    return res;
}

void FileNoradScheduleSaver::FullfilLine(QString &line){
    while (line.size() < 37) {
        line.append(' ');
    }
    line.append("\n");
}

bool FileNoradScheduleSaver::parseDateTime(const std::string &dateTimeStr,
                          int &year, int &month, int &day,
                          int &hour, int &minute, int &second, int &microsecond)
{
    std::istringstream iss(dateTimeStr);
    char sep1, sep2, sep3, sep4, sep5; // Разделители "-", " ", ":", ".", " "

    iss >> year >> sep1 >> month >> sep2 >> day
        >> hour >> sep3 >> minute >> sep4 >> second >> sep5 >> microsecond;

    if (sep1 != '-' || sep2 != '-' || sep3 != ':' || sep4 != ':' || sep5 != '.') {
        return false;
    }

    if (month < 1 || month > 12 || day < 1 || day > 31 ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 60) {
        return false;
    }

    return true;
}
