//
// Created by Paco Pronk on 2020-02-06.
//

#ifndef UVSMARTD25_DISINFECTIONRUN_H
#define UVSMARTD25_DISINFECTIONRUN_H

#include "Unit.h"

UnitRun unitDisinfectionRun(void);
UnitDisinfectionState disinfection_state_open_lid(void);
UnitDisinfectionState disinfection_state_disinfection(void);
UnitDisinfectionState disinfection_state_lid_opened_unauthorized_error(void);
UnitDisinfectionState disinfection_state_check_runs(void);
UnitDisinfectionState disinfection_state_successful(void);
UnitDisinfectionState disinfection_state_unsuccessful(void);
UvLamp SelectUVLamp(void);


#endif //UVSMARTD25_DISINFECTIONRUN_H
