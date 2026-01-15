#pragma once
#include <stdint.h>

void servo_init();
void servo_set_angle(uint8_t index, int16_t angle);   // -900 ~ +900
void servo_commit();                                  // 批量更新
