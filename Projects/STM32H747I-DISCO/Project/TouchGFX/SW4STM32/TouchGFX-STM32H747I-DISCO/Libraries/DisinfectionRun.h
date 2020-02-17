//
// Created by Paco Pronk on 2020-02-06.
//

#ifndef UVSMARTD25_DISINFECTIONRUN_H
#define UVSMARTD25_DISINFECTIONRUN_H

#include "Unit.h"

UnitRun unitDisinfectionRun();
UnitDisinfectionState disinfection_state_open_lid();
UnitDisinfectionState disinfection_state_disinfection();
UnitDisinfectionState disinfection_state_lid_opened_unauthorized_error();
UnitDisinfectionState disinfection_state_check_runs();
UnitDisinfectionState disinfection_state_successful();
UnitDisinfectionState disinfection_state_unsuccessful();



#endif //UVSMARTD25_DISINFECTIONRUN_H
