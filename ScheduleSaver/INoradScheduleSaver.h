#ifndef INORADSCHEDULESAVER_H
#define INORADSCHEDULESAVER_H

#include <vector>

#include <LIBS/Include/libsgp4/Observer.h>

struct NORAD_SCHEDULE {

    float azm;
    float elv;
    libsgp4::DateTime onDate;

    NORAD_SCHEDULE(float a = 0.0, float e = 0.0, const libsgp4::DateTime& date = {})
        : azm(a), elv(e), onDate(date) {}

    NORAD_SCHEDULE(const NORAD_SCHEDULE&) = default;
    NORAD_SCHEDULE& operator=(const NORAD_SCHEDULE&) = default;
    NORAD_SCHEDULE(NORAD_SCHEDULE&&) noexcept = default;
    NORAD_SCHEDULE& operator=(NORAD_SCHEDULE&&) noexcept = default;
};


class INoradScheduleSaver {
public:
    virtual ~INoradScheduleSaver() = default;
    virtual bool save(const std::vector<NORAD_SCHEDULE>& vecNoradSchedule) const = 0;
    virtual bool load(std::vector<NORAD_SCHEDULE>& vecNoradSchedule) const = 0;
};


#endif // INORADSCHEDULESAVER_H
