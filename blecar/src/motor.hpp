//
// Motor PWM
// Created by ronger on 2024/2/7.
//

#ifndef MOTOR_HPP
#define MOTOR_HPP

#include "Arduino.h"

class Motor {
public:
    uint16_t mMotorSpeed[6] = {0, 0, 0, 0, 0, 0};
    boolean mMotorForward[6] = {true, true, true, true, true, true};

    void attachMotor(uint8_t gpioIn1, uint8_t gpioIn2);
    void attachMotorInit(uint8_t gpioIn1, uint8_t gpioIn2, uint8_t motor, uint8_t ioSignal);

    void motorFullForward(uint8_t motor);
    void motorForward(uint8_t motor, uint8_t timer, uint8_t speed);
    void motorFullReverse(uint8_t motor);
    void motorReverse(uint8_t motor, uint8_t timer, uint8_t speed);
    void motorStop(uint8_t motor, uint8_t timer);
    void motorsStop();
private:

    // Fields:

    boolean mMotorAttached[6] = {false, false, false, false, false, false};

    // Methods

    boolean isMotorValid(uint8_t motor, uint8_t timer);
};


#endif //MOTOR_HPP
