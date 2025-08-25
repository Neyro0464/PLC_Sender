#ifndef FILENORADSCHEDULESAVER_H
#define FILENORADSCHEDULESAVER_H

#include <string>
#include <QString>


#include "INoradScheduleSaver.h"


class FileNoradScheduleSaver : public INoradScheduleSaver {
public:
    explicit FileNoradScheduleSaver(const std::string& filePath): m_filePath(filePath){};

    bool save(const std::vector<NORAD_SCHEDULE>& vecNoradSchedule) const override;

    bool saveStep(const float estAzm, const float curAzm,
                  const float estElv, const float curElv,
                  const float errAzm, const float errElv
                  ) const;

    bool clear();

    bool load(std::vector<NORAD_SCHEDULE>& vecNoradSchedule) const override;
    void setCommand(const int cmd) override {m_cmd = cmd;};

    //Preset
    void setReserved1(int32_t value) {reserved1 = value;};
    void setReserved2(int32_t value) {reserved2 = value;};

    static void trim(std::string &str);

    static QString TrimUTC(const libsgp4::DateTime onDate);

    static void FullfilLine(QString &line, int size = 37);

    static bool parseDateTime(const std::string &dateTimeStr,
                              int &year, int &month, int &day,
                              int &hour, int &minute, int &second, int &microsecond);

private:
    int32_t m_cmd;
    int32_t reserved1 = 0;
    int32_t reserved2 = 0;
    std::string m_filePath;

};

#endif // FILENORADSCHEDULESAVER_H
