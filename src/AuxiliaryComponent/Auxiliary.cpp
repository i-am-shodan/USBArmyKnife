#include "Auxiliary.h"
#include "Sensors/Motion/LD2410/LD2410.h"

#ifdef EXT_SENSOR_MOTION_LD2410
    static LD2410MotionSensorExtension LD2410;
#endif

Auxiliary::Auxiliary()
{
}

void Auxiliary::begin(Preferences &prefs)
{
#ifdef EXT_SENSOR_MOTION_LD2410
    LD2410.begin(prefs);
#endif
}

void Auxiliary::loop(Preferences& prefs)
{
#ifdef EXT_SENSOR_MOTION_LD2410
    LD2410.loop(prefs);
#endif
}