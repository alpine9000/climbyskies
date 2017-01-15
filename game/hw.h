#ifndef __HW_H
#define __HW_H

void 
hw_waitVerticalBlank(void);

void
hw_setupPalette(void);

void 
hw_interruptsInit(void);

void 
hw_waitBlitter(void);

void 
hw_waitRaster(__reg("d0") uint32_t);

void 
hw_waitScanLines(__reg("d2") uint32_t);

void 
hw_readJoystick(void);

void 
hw_waitForJoystick(void);

uint32_t
hw_getRasterLine(void);

extern volatile uint8_t hw_joystickButton;
extern volatile uint8_t hw_joystickPos;

#define JOYSTICK_BUTTON_DOWN (hw_joystickButton&0x1)
#endif
