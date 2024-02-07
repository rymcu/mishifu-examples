//
// Created by ronger on 2024/2/7.
//
#include "Arduino.h"

#include "motor.hpp"

#include "esp_system.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

//#define debug(fmt, ...)
#define debug(fmt, ...) Serial.printf("%s: " fmt "\r\n", __func__, ##__VA_ARGS__)

// Attach one motor

void Motor::attachMotor(uint8_t gpioIn1, uint8_t gpioIn2) {
    attachMotorInit(gpioIn1, gpioIn2, 0, 0);
}

void Motor::attachMotorInit(uint8_t gpioIn1, uint8_t gpioIn2, uint8_t motor, uint8_t ioSignal) {
    debug("init MCPWM_UNIT_", motor, " IO_SIGNAL", ioSignal);
    mcpwm_unit_t pwmNum = mcpwm_unit_t(motor);
    mcpwm_io_signals_t ioSignalA = mcpwm_io_signals_t(0);
    mcpwm_io_signals_t ioSignalB = mcpwm_io_signals_t(1);
    if (ioSignal == 1) {
        ioSignalA = mcpwm_io_signals_t(2);
        ioSignalB = mcpwm_io_signals_t(3);
    } else if (ioSignal == 2) {
        ioSignalA = mcpwm_io_signals_t(4);
        ioSignalB = mcpwm_io_signals_t(5);
    }
    mcpwm_gpio_init(pwmNum, ioSignalA, gpioIn1);
    mcpwm_gpio_init(pwmNum, ioSignalB, gpioIn2);
    if (motor == 0) {
        this->mMotorAttached[ioSignal] = true;
    } else {
        this->mMotorAttached[ioSignal + 3] = true;
    }

    // Initial MCPWM configuration

    debug ("Configuring Initial Parameters of MCPWM...");

    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency,
    pwm_config.cmpr_a = 0;            //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;            //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &pwm_config);    //Configure PWM0A & PWM0B with above settings

    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_2, &pwm_config);    //Configure PWM0A & PWM0B with above settings


    debug ("MCPWM initialized");
}

void Motor::motorFullForward(uint8_t motor) {
    mcpwm_unit_t pwmNum = mcpwm_unit_t(motor);
    for (int i = 0; i < 3; ++i) {
        if (!isMotorValid(motor, i)) {
            continue;
        }
        configTICK_RATE_HZ;
        mcpwm_timer_t timerNum = mcpwm_timer_t(i);
        mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_B);
        mcpwm_set_signal_high(pwmNum, timerNum, MCPWM_OPR_A);
    }
}

void Motor::motorForward(uint8_t motor, uint8_t timer, uint8_t speed) {
    if (!isMotorValid(motor, timer)) {
        return;
    }
    mcpwm_unit_t pwmNum = mcpwm_unit_t(motor);
    mcpwm_timer_t timerNum = mcpwm_timer_t(timer);
    mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_B);
    mcpwm_set_duty(pwmNum, timerNum, MCPWM_OPR_A, speed);
    // call this each time, if operator was previously in low/high state
    mcpwm_set_duty_type(pwmNum, timerNum, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    uint8_t motorTimer = timer;
    if (motor == 1) {
        motorTimer += 3;
    }
    mMotorSpeed[motorTimer] = speed; // Save it
    mMotorForward[motorTimer] = true;

    debug("Motor %u forward speed %u", motorTimer, speed);
}

void Motor::motorFullReverse(uint8_t motor) {
    mcpwm_unit_t pwmNum = mcpwm_unit_t(motor);
    for (int i = 0; i < 3; ++i) {
        if (!isMotorValid(motor, i)) {
            continue;
        }
        mcpwm_timer_t timerNum = mcpwm_timer_t(i);
        mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_A);
        mcpwm_set_signal_high(pwmNum, timerNum, MCPWM_OPR_B);
    }
}

void Motor::motorReverse(uint8_t motor, uint8_t timer, uint8_t speed) {
    if (!isMotorValid(motor, timer)) {
        return;
    }
    mcpwm_unit_t pwmNum = mcpwm_unit_t(motor);
    mcpwm_timer_t timerNum = mcpwm_timer_t(timer);
    mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_A);
    mcpwm_set_duty(pwmNum, timerNum, MCPWM_OPR_B, speed);
    // call this each time, if operator was previously in low/high state
    mcpwm_set_duty_type(pwmNum, timerNum, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
    uint8_t motorTimer = timer;
    if (motor == 1) {
        motorTimer += 3;
    }
    mMotorSpeed[motorTimer] = speed; // Save it
    mMotorForward[motorTimer] = false;

    debug("Motor %u forward speed %u", motorTimer, speed);
}

void Motor::motorStop(uint8_t motor, uint8_t timer) {
    if (!isMotorValid(motor, timer)) {
        return;
    }
    mcpwm_unit_t pwmNum = mcpwm_unit_t(motor);
    mcpwm_timer_t timerNum = mcpwm_timer_t(timer);
    mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_A);
    mcpwm_set_signal_low(pwmNum, timerNum, MCPWM_OPR_B);
}

void Motor::motorsStop() {
    for (int i = 0; i < 3; ++i) {
        motorStop(0, i);
        motorStop(1, i);
    }
    debug("Motors stop");
}

//// Privates

// Is motor valid ?

boolean Motor::isMotorValid(uint8_t motor, uint8_t timer) {
    if (motor > 1) {
        return false;
    }
    uint8_t motorTimer = timer;
    if (motor == 1) {
        motorTimer += 3;
    }
    return mMotorAttached[motorTimer];
}


///// End