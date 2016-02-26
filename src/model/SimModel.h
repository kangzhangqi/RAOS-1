/*
 * Models of RAOS
 *
 * This file is included by SimMain.cxx
 * The implementations are written in SimModel.cxx, which contains functions
 * in files qrmod.cxx plume_model.cxx wind_model.cxx
 *
 * Author: Roice (LUO Bing)
 * Date: 2016-02-23 create this file (RAOS)
 */
#ifndef SIMMODEL_H
#define SIMMODEL_H

#include "qrmod.h"

void SimModel_init(void);
qrstate_t* SimModel_get_qrstate(void);

#endif
/* End of SimModel.h */
