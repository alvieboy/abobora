#ifndef __UART_H__
#define __UART_H__

void uart_init();
void outbyte(int c);
int inbyte(unsigned char *c);

void outstring(const char *str);
void printhexbyte(unsigned int c);
void printhex(unsigned int c);

#endif
