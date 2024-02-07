//
// Created by ronger on 2024/2/7.
//

#ifndef CONTROL_HPP
#define CONTROL_HPP

#include <motor.hpp>

class Control {
public:
    Motor motors[4] = {Motor(), Motor(), Motor(), Motor()};

    uint8_t pointXZero = 100;

    uint8_t pointYZero = 100;

    void motorInit(Motor leftFrontMotor, Motor rightFrontMotor, Motor leftRearMotor, Motor rightRearMotor);

    void pointXInit(uint8_t point);

    void pointYInit(uint8_t point);

    void pointInit(uint8_t pointX, uint8_t pointY);

    void run(uint8_t pointX, uint8_t pointY);

    void stop();
private:
    static uint8_t calculationSpeed(int speed);
};


#endif //CONTROL_HPP
