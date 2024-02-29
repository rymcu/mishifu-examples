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
    if (pointX == pointXZero) {
        if (pointY == pointYZero) {
            // 静止
            this->stop();
        } else if (pointY < pointYZero) {
            // 前进
            uint8_t speed = pointYZero - pointY;
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
            uint8_t speed = pointY - pointYZero;
            Motor leftFrontMotor = motors[0];
            Motor rightFrontMotor = motors[1];
            Motor leftRearMotor = motors[2];
            Motor rightRearMotor = motors[3];
            leftFrontMotor.motorReverse(0, 0, speed);
            rightFrontMotor.motorReverse(0, 1, speed);
            leftRearMotor.motorReverse(1, 0, speed);
            rightRearMotor.motorReverse(1, 1, speed);
        }
    } else if (pointX > pointXZero) {
        // 右转
        Motor leftFrontMotor = motors[0];
        Motor rightFrontMotor = motors[1];
        Motor leftRearMotor = motors[2];
        Motor rightRearMotor = motors[3];
        // 值越大, 轮胎转速越小
        uint8_t turnSpeed = 2 * pointYZero - pointX;
        if (pointY == pointYZero) {
            uint8_t speed = turnSpeed + 50 > 100 ? 100 : turnSpeed + 50;
            leftFrontMotor.motorForward(0, 0, speed);
            rightFrontMotor.motorForward(0, 1, turnSpeed);
            leftRearMotor.motorForward(1, 0, speed);
            rightRearMotor.motorForward(1, 1, speed);
        } else if (pointY < pointYZero) {
            // 前进
            uint8_t speed = pointYZero - pointY;
            if (speed < turnSpeed) {
                speed = turnSpeed + 50 > 100 ? 100 : turnSpeed + 50;
            }
            leftFrontMotor.motorForward(0, 0, speed);
            rightFrontMotor.motorForward(0, 1, turnSpeed);
            leftRearMotor.motorForward(1, 0, speed);
            rightRearMotor.motorForward(1, 1, speed);
        } else {
            // 倒车
            uint8_t speed = pointY - pointYZero;
            if (speed < turnSpeed) {
                speed = turnSpeed + 50 > 100 ? 100 : turnSpeed + 50;
            }
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
        uint8_t turnSpeed = pointX;
        if (pointY == pointYZero) {
            uint8_t speed = turnSpeed + 50 > 100 ? 100 : turnSpeed + 50;
            leftFrontMotor.motorForward(0, 0, speed);
            rightFrontMotor.motorForward(0, 1, speed);
            leftRearMotor.motorForward(1, 0, turnSpeed);
            rightRearMotor.motorForward(1, 1, speed);
        } else if (pointY > pointYZero) {
            // 前进
            uint8_t speed = pointYZero - pointY;
            if (speed < turnSpeed) {
                speed = turnSpeed + 50 > 100 ? 100 : turnSpeed + 50;
            }
            leftFrontMotor.motorForward(0, 0, speed);
            rightFrontMotor.motorForward(0, 1, speed);
            leftRearMotor.motorForward(1, 0, turnSpeed);
            rightRearMotor.motorForward(1, 1, speed);
        } else {
            // 倒车
            uint8_t speed = pointY - pointYZero;
            if (speed < turnSpeed) {
                speed = turnSpeed + 50 > 100 ? 100 : turnSpeed + 50;
            }
            leftFrontMotor.motorReverse(0, 0, speed);
            rightFrontMotor.motorReverse(0, 1, speed);
            leftRearMotor.motorReverse(1, 0, turnSpeed);
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