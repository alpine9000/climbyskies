#ifndef __MESSAGE_H
#define __MESSAGE_H

void
message_screenOn(char* message);
void
message_screenOff(void);
void
message_box(char* message, void (*callback)(void));
void
message_boxDismiss(void);
void
message_alert(char* message);
void
message_boxSaveBackground(frame_buffer_t fb);
void
message_boxRestoreBackground(void);
void
message_boxRender(frame_buffer_t fb);
void
message_boxOff(void);
#endif
