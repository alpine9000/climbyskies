#ifndef __HW_H
#define __HW_H

#if defined(__GNUC__)
#define hw_waitBlitter()  while (((volatile struct Custom*)CUSTOM)->dmaconr & 1<<14);
#else
#define hw_waitBlitter() _hw_waitBlitter();
void 
_hw_waitBlitter(void);
#endif

void 
hw_waitVerticalBlank(void);
void
hw_setupPalette(void);
void 
hw_interruptsInit(void);
void 
hw_waitRaster(__REG("d0", uint32_t));
void 
hw_waitScanLines(__REG("d2", uint32_t));
void 
hw_readJoystick(void);
void 
hw_waitForJoystick(void);
uint32_t
hw_getRasterLine(void);

extern volatile uint8_t hw_joystickButton;
extern volatile uint8_t hw_joystickPos;
extern uint32_t hw_verticalBlankCount;

#define JOYSTICK_BUTTON_DOWN (hw_joystickButton&0x1)

#define JOYSTICK_IDLE() (hw_joystickPos == 0)
#define JOYSTICK_LEFT() (hw_joystickPos == 7)
#define JOYSTICK_RIGHT() (hw_joystickPos == 3)
#define JOYSTICK_UP() (hw_joystickPos == 1)
#define JOYSTICK_DOWN() (hw_joystickPos == 5)

#endif
