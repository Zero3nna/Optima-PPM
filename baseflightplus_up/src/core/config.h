/*
 * Copyright (c) 2012 Baseflight U.P.
 * Licensed under the MIT License
 * @author  Scott Driessens v0.1 (August 2012)
 */

#pragma once

#include "core/command.h"
#include "actuator/pid.h"

///////////////////////////////////////////////////////////////////////////////
// Config Typedefs
///////////////////////////////////////////////////////////////////////////////

typedef struct {
    float p;
    float i;
    float d;
    float iLim;
} pidConfig;

typedef enum {
    FEATURE_PPM = 1 << 0,
    FEATURE_VBAT = 1 << 1,
    FEATURE_MOTOR_STOP = 1 << 2,
    FEATURE_SERVO_TILT = 1 << 3,
    FEATURE_FAILSAFE = 1 << 4,
    FEATURE_SONAR = 1 << 5,
} AvailableFeatures;

typedef enum {
    GIMBAL_NORMAL = 1 << 0,
    GIMBAL_TILTONLY = 1 << 1,
    GIMBAL_DISABLEAUX34 = 1 << 2,
    GIMBAL_FORWARDAUX = 1 << 3,
} GimbalFlags;

typedef struct {
    uint8_t version;
    uint32_t enabledFeatures;
    
    uint16_t failsafeOnDelay;
    uint16_t failsafeOffDelay;
    uint16_t failsafeThrottle;

    uint8_t rcMap[8];

    uint16_t escPwmRate;
    uint16_t servoPwmRate;
    
    uint16_t auxActivate[AUX_OPTIONS];

    uint8_t mixerConfiguration;

    uint16_t minCommand;
    uint16_t midCommand;
    uint16_t maxCommand;
    uint16_t minCheck;
    uint16_t maxCheck;
    uint16_t minThrottle;
    uint16_t maxThrottle;
    
    uint16_t deadBand[3];
    
    int16_t servotrim[8];                   // Adjust Servo MID Offset & Swash angles
    int8_t servoreverse[8];

    uint16_t biLeftServoMin;
    uint16_t biLeftServoMid;
    uint16_t biLeftServoMax;

    uint16_t biRightServoMin;
    uint16_t biRightServoMid;
    uint16_t biRightServoMax;

    int8_t yawDirection;
    uint16_t triYawServoMin;
    uint16_t triYawServoMid;
    uint16_t triYawServoMax;

    uint8_t gimbalFlags;
    
    uint16_t gimbalRollServoMin;
    uint16_t gimbalRollServoMid;
    uint16_t gimbalRollServoMax;
    float gimbalRollServoGain;

    uint16_t gimbalPitchServoMin;
    uint16_t gimbalPitchServoMid;
    uint16_t gimbalPitchServoMax;
    float gimbalPitchServoGain;

    int8_t rollDirectionLeft;
    int8_t rollDirectionRight;
    int8_t pitchDirectionLeft;
    int8_t pitchDirectionRight;
    
    uint16_t wingLeftMinimum;
    uint16_t wingLeftMaximum;
    uint16_t wingRightMinimum;
    uint16_t wingRightMaximum;
    
    pidConfig pids[NUM_PIDS];
    
    float angleTrim[2];
    
    uint8_t accelLPF;
    float accelLPF_Factor;
    uint8_t accelCalibrated;
    float accelBias[3];

    uint16_t gyroLPF;
    float gyroTCBiasSlope[3];
    float gyroTCBiasIntercept[3];

    uint8_t magCalibrated;
    float magBias[3];
    
    uint8_t mpu6050Scale;

    // For Mahony AHRS
    
    float imuKp;
    float imuKi;

    uint8_t magDriftCompensation;
    
    float magDeclination;
    
    float batScale;
    float batMinCellVoltage;
    float batMaxCellVoltage;

#ifdef THESIS  
    float thesisScaler;
#endif

} cfg_t;

///////////////////////////////////////////////////////////////////////////////
// External Variables
///////////////////////////////////////////////////////////////////////////////

extern cfg_t cfg;

extern const char rcChannelLetters[8];

///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////

void parseRcChannels(const char *input);
void readEEPROM(void);
void writeParams(void);
void checkFirstTime(bool reset);

bool featureGet(uint32_t mask);
void featureSet(uint32_t mask);
void featureClear(uint32_t mask);
void featureClearAll(void);
uint32_t featureMask(void);

///////////////////////////////////////////////////////////////////////////////