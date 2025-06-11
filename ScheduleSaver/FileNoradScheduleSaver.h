#ifndef FILENORADSCHEDULESAVER_H
#define FILENORADSCHEDULESAVER_H

#include <string>

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

    static void trim(std::string &str);

    static bool parseDateTime(const std::string &dateTimeStr,
                              int &year, int &month, int &day,
                              int &hour, int &minute, int &second, int &microsecond);

private:
    std::string m_filePath;
};

#endif // FILENORADSCHEDULESAVER_H
