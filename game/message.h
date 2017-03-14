#ifndef __MESSAGE_H
#define __MESSAGE_H

void
message_screenOn(char* message);
void
message_loading(char* message);
void
message_screenOff(void);
char*
message_prompt(uint16_t numChars);
#endif
