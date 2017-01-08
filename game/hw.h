#ifndef __HW_H
#define __HW_H

extern void hw_waitVerticalBlank(void);
extern void hw_setupPalette(void);
extern void hw_interruptsInit(void);
extern void hw_waitBlitter(void);
extern void hw_waitScanLines(__reg("d2") uint32_t);
extern void hw_readJoystick(void);
extern void hw_waitForJoystick(void);
extern volatile uint8_t hw_joystickButton;
extern volatile uint8_t hw_joystickPos;

#define JOYSTICK_BUTTON_DOWN (hw_joystickButton&0x1)
#endif
