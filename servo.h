#ifndef __SERVO_H__
#define __SERVO_H__

#define SERVO_CHANNEL_A 0
#define SERVO_CHANNEL_B 1

void servo_init();
void servo_set_channel_a(unsigned pulse);
void servo_set_channel_b(unsigned pulse);
void servo_set_channel(uint8_t channel, unsigned pulse);
void servo_enable();
void servo_disable();


#endif
