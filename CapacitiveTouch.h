#include <stdbool.h>

extern void deinit_CapANDAudio(void);
extern void init_CapTouch(void);
extern void CalcCapTouch(void);
extern void SetSenCapTouch(int filter, int thresholdHigh, int thresholdLow, int shortPress, int longPress);
extern void CaptureCapTouch(int channel);

extern volatile int CapSensorOutput[2];

extern bool ProcessCapTouch(int Button);
