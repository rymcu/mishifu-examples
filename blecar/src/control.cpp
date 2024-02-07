//
// Created by ronger on 2024/2/7.
//

#include "control.hpp"

void Control::motorInit(Motor leftFrontMotor, Motor rightFrontMotor, Motor leftRearMotor, Motor rightRearMotor) {
    motors[0] = leftFrontMotor;
    motors[1] = rightFrontMotor;
    motors[2] = leftRearMotor;
    motors[3] = rightRearMotor;
}

void Control::pointXInit(uint8_t point) {
    pointInit(point, 100);
}

void Control::pointYInit(uint8_t point) {
    pointInit(100, point);
}

void Control::pointInit(uint8_t pointX, uint8_t pointY) {
    pointXZero = pointX;
    pointYZero = pointY;
}

void Control::run(uint8_t pointX, uint8_t pointY) {
    // 左右
    int x = pointX - pointXZero;
    // 前后
    int y = pointY - pointYZero;
    if (x == 0) {
        if (y == 0) {
            this->stop();
        } else if (y < 0) {
            // 前进
            uint8_t speed = Control::calculationSpeed(y);
            Motor leftFrontMotor = motors[0];
            Motor rightFrontMotor = motors[1];
            Motor leftRearMotor = motors[2];
            Motor rightRearMotor = motors[3];
            leftFrontMotor.motorForward(0, 0, speed);
            rightFrontMotor.motorForward(0, 1, speed);
            leftRearMotor.motorForward(1, 0, speed);
            rightRearMotor.motorForward(1, 1, speed);
        } else {
            // 倒车
            uint8_t speed = Control::calculationSpeed(y);
            Motor leftFrontMotor = motors[0];
            Motor rightFrontMotor = motors[1];
            Motor leftRearMotor = motors[2];
            Motor rightRearMotor = motors[3];
            leftFrontMotor.motorReverse(0, 0, speed);
            rightFrontMotor.motorReverse(0, 1, speed);
            leftRearMotor.motorReverse(1, 0, speed);
            rightRearMotor.motorReverse(1, 1, speed);
        }
    } else if (x > 0) {
        // 右转
        Motor leftFrontMotor = motors[0];
        Motor rightFrontMotor = motors[1];
        Motor leftRearMotor = motors[2];
        Motor rightRearMotor = motors[3];
        uint8_t turnSpeed = Control::calculationSpeed(50 - Control::calculationSpeed(x));
        if (y == 0) {
            uint8_t speed = Control::calculationSpeed(x);
            leftFrontMotor.motorForward(0, 0, speed);
            rightFrontMotor.motorForward(0, 1, turnSpeed);
            leftRearMotor.motorForward(1, 0, speed);
            rightRearMotor.motorForward(1, 1, speed);
        } else if (y < 0) {
            // 前进
            uint8_t speed = Control::calculationSpeed(y);
            leftFrontMotor.motorForward(0, 0, speed);
            rightFrontMotor.motorForward(0, 1, turnSpeed);
            leftRearMotor.motorForward(1, 0, speed);
            rightRearMotor.motorForward(1, 1, speed);
        } else {
            // 倒车
            uint8_t speed = Control::calculationSpeed(y);
            leftFrontMotor.motorReverse(0, 0, speed);
            rightFrontMotor.motorReverse(0, 1, turnSpeed);
            leftRearMotor.motorReverse(1, 0, speed);
            rightRearMotor.motorReverse(1, 1, speed);
        }
    } else {
        // 左转
        Motor leftFrontMotor = motors[0];
        Motor rightFrontMotor = motors[1];
        Motor leftRearMotor = motors[2];
        Motor rightRearMotor = motors[3];
        uint8_t turnSpeed = Control::calculationSpeed(50 - Control::calculationSpeed(x));
        if (y == 0) {
            uint8_t speed = Control::calculationSpeed(x);
            leftFrontMotor.motorForward(0, 0, turnSpeed);
            rightFrontMotor.motorForward(0, 1, speed);
            leftRearMotor.motorForward(1, 0, speed);
            rightRearMotor.motorForward(1, 1, speed);
        } else if (y > 0) {
            // 前进
            uint8_t speed = Control::calculationSpeed(y);
            leftFrontMotor.motorForward(0, 0, 100 - turnSpeed);
            rightFrontMotor.motorForward(0, 1, speed);
            leftRearMotor.motorForward(1, 0, speed);
            rightRearMotor.motorForward(1, 1, speed);
        } else {
            // 倒车
            uint8_t speed = Control::calculationSpeed(y);
            leftFrontMotor.motorReverse(0, 0, 100 - turnSpeed);
            rightFrontMotor.motorReverse(0, 1, speed);
            leftRearMotor.motorReverse(1, 0, speed);
            rightRearMotor.motorReverse(1, 1, speed);
        }
    }
}

void Control::stop() {
    Motor leftFrontMotor = motors[0];
    leftFrontMotor.motorStop(0, 0);
    Motor rightFrontMotor = motors[1];
    rightFrontMotor.motorStop(0, 1);
    Motor leftRearMotor = motors[2];
    leftRearMotor.motorStop(1, 0);
    Motor rightRearMotor = motors[3];
    rightRearMotor.motorStop(1, 1);
}

//// Privates

uint8_t Control::calculationSpeed(int speed) {
    return abs(speed);
}