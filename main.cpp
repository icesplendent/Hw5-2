#include "bbcar.h"
#include "mbed.h"
#include "parallax_servo.h"

Ticker servo_ticker;
Ticker servo_feedback_ticker;

Thread measure;
EventQueue measure_queue;

PwmOut pin9(D9), pin10(D10);
PwmIn pin11(D11), pin12(D12);
BBCar car(pin9, pin11, pin10, pin12, servo_ticker, servo_feedback_ticker);
BusInOut qti_pin(D4, D5, D6, D7);
int baseSpeed = 60;
int cycle = 0;
float length = 0;
float circleLength = 20.106;  // 圓周長 cm
float angle = 0, angle_init = 0, angle_temp = 0;
float PI = 3.1415926535897932;

void length_measure() {
    angle = (car.getAngle0() + car.getAngle1()) / 2;
    printf("test, cycle = %d, angle = %f, angle_temp = %f\n", cycle, angle,
           angle_temp);
    if (angle_temp - angle > 50) {
        cycle = (cycle + 1) % 4;
        angle_temp = angle;

        if (cycle == 0) {
            length = (angle_init - angle) / 360 * 2 * PI * circleLength;
            printf("length = %f, cycle = %d", length, cycle);
            angle_init = angle;
        }
    }
}

int main() {
    parallax_qti qti1(qti_pin);
    int index = 0;
    int temp[16] = {0};
    int pattern;
    int oldpattern[3] = {0, 0, 0};
    length = 0;
    angle_init = car.getAngle1() + car.getAngle0() / 2;
    angle_temp = angle_init;
    measure.start(callback(&measure_queue, &EventQueue::dispatch_forever));

    car.goStraight(50);
    while (1) {
        pattern = (int)qti1;
        //   printf("%d\n", pattern);

        switch (pattern) {
            case 0b1000: {
                if (oldpattern[2] == 0b1100) {
                    car.Totalturn(baseSpeed + 200, 0.8);
                    ThisThread::sleep_for(30ms);
                } else if (oldpattern[1] == 0b1100) {
                    car.Totalturn(baseSpeed + 150, 0.8);
                } else if (oldpattern[0] == 0b1100) {
                    car.Totalturn(baseSpeed + 200, 0.8);
                } else {
                    car.Totalturn(baseSpeed, 0.1);
                }
                break;
            }
            case 0b1100: {
                if (oldpattern[2] == 0b1000) {
                    car.Totalturn(baseSpeed + 200, -0.8);
                } else if (oldpattern[1] == 0b1000) {
                    car.Totalturn(baseSpeed + 200, -0.8);
                } else if (oldpattern[0] == 0b1000) {
                    car.Totalturn(baseSpeed + 200, -0.5);
                } else if (oldpattern[2] == 0b0110) {
                    car.Totalturn(baseSpeed + 200, -0.5);
                } else if (oldpattern[1] == 0b0110) {
                    car.Totalturn(baseSpeed + 100, -0.5);
                } else if (oldpattern[0] == 0b0110) {
                    car.Totalturn(baseSpeed, -0.5);
                } else {
                    car.turn(baseSpeed, 0.5);
                }
                break;
            }
            case 0b1110:
                car.turn(baseSpeed, -0.5);
                break;
            case 0b0100:
                car.turn(baseSpeed, 0.7);
                break;
            case 0b0110:
                car.goStraight(baseSpeed - 40);
                break;
            case 0b0010:
                car.turn(baseSpeed, -0.7);
                break;
            case 0b0111:
                car.turn(baseSpeed, 0.5);
                break;
            case 0b0011: {
                if (oldpattern[2] == 0b0001) {
                    car.Totalturn(baseSpeed + 200, 0.8);
                } else if (oldpattern[1] == 0b0001) {
                    car.Totalturn(baseSpeed + 200, 0.8);
                } else if (oldpattern[0] == 0b0001) {
                    car.Totalturn(baseSpeed + 200, -0.5);
                } else if (oldpattern[2] == 0b0110) {
                    car.goStraight(-(baseSpeed + 200));
                } else if (oldpattern[1] == 0b0110) {
                    car.goStraight(-(baseSpeed + 120));
                } else if (oldpattern[0] == 0b0110) {
                    car.stop();
                } else {
                    car.turn(baseSpeed, -0.5);
                }
                break;
                //  car.turn(50, -0.5);
                //  break;
            }
            case 0b0001: {
                if (oldpattern[2] == 0b0011) {
                    car.Totalturn(baseSpeed + 200, -0.8);
                } else if (oldpattern[1] == 0b0011) {
                    car.Totalturn(baseSpeed + 100, -0.5);
                } else if (oldpattern[1] == 0b0011) {
                    car.Totalturn(baseSpeed + 20, -0.5);
                } else {
                    car.Totalturn(baseSpeed, -0.1);
                }
                break;
            }
            case 0b1111: {
                if (oldpattern[0] == 0b1111 && oldpattern[1] == 0b1111 &&
                    oldpattern[2] == 0b1111) {
                    car.goStraight(baseSpeed);
                    ThisThread::sleep_for(100ms);
                    //  printf("first, cycleA = %d", cycleA);
                } else if (oldpattern[2] == 0b1111) {
                    car.goStraight(-baseSpeed);
                    ThisThread::sleep_for(500ms);
                    car.stop();
                    //  printf("second, cycleA = %d\n", cycleA);
                } else {
                    car.stop();
                    // printf("third, cycleA = %d\n", cycleA);
                }

                measure_queue.call(length_measure);
                // printf("at least\n");

                break;
            }
            case 0b0000: {
                car.goStraight(-baseSpeed);
                // printf("here\n");
                break;
            }
            case 0b1011: {
                car.Totalturn(baseSpeed, -0.5);
                break;
            }
            case 0b1010: {
                car.Totalturn(baseSpeed, -0.5);
                break;
            }
            case 0b1001: {
                car.goStraight(baseSpeed);
                break;
            }
            case 0b0101: {
                car.turn(baseSpeed, 0.5);
                break;
            }
            case 0b1101: {
                car.goStraight(baseSpeed);
                break;
            }
            default: {
                car.goStraight(-10);
            }
        }
        ThisThread::sleep_for(10ms);

        oldpattern[0] = oldpattern[1];
        oldpattern[1] = oldpattern[2];
        oldpattern[2] = pattern;
    }
}
