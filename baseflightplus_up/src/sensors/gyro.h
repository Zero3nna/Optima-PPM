/*
 * Copyright (c) 2012 Baseflight U.P.
 * Licensed under the MIT License
 * @author  Scott Driessens v0.1 (August 2012)
 */
 
#pragma once

#include "sensors/sensors.h"

extern int16_t rawGyro[3];

extern int16_t rawGyroTemperature;

extern gyro_t *gyro;

///////////////////////////////////////////////////////////////////////////////
// Read Gyro
///////////////////////////////////////////////////////////////////////////////

void readGyro(void);

void readGyroTemp(void);

///////////////////////////////////////////////////////////////////////////////
// Compute Gyro Temperature Compensation Bias
///////////////////////////////////////////////////////////////////////////////

void computeGyroTCBias(void);

///////////////////////////////////////////////////////////////////////////////
// Compute Gyro Runtime Bias
///////////////////////////////////////////////////////////////////////////////

void computeGyroRTBias(void);

///////////////////////////////////////////////////////////////////////////////
// Gyro Temperature Calibration
///////////////////////////////////////////////////////////////////////////////

void gyroTempCalibration(void);

///////////////////////////////////////////////////////////////////////////////

void initGyro(void);