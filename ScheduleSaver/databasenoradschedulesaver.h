#ifndef DATABASENORADSCHEDULESAVER_H
#define DATABASENORADSCHEDULESAVER_H

#include <QtGlobal>

#include "INoradScheduleSaver.h"

class DatabaseNoradScheduleSaver : public INoradScheduleSaver {
public:
    DatabaseNoradScheduleSaver() = default;
    bool save(const std::vector<NORAD_SCHEDULE>& vecNoradSchedule) const override;
};


#endif // DATABASENORADSCHEDULESAVER_H
