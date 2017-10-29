#ifndef __SERVO_H__
#define __SERVO_H__

#define SERVO_CHANNEL_A 0
#define SERVO_CHANNEL_B 1

void servo_init();
void servo_ser_channel_a(uint16_t pulse);
void servo_ser_channel_b(uint16_t pulse);
void servo_set_channel(uint8_t channel, uint16_t pulse);



#endif
