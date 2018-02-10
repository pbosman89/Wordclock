
#include <stdbool.h>

extern void TLC5925Init(void);
extern void TLC5925ShiftOut(int Data, int Length);
extern void TLC5925Latch(void);
extern void TLC5925Output(bool State);

extern void HC595Init(void);
extern void HC595ShiftOut(int Data, int Length);
extern void HC595Latch(void);
extern void HC595Output(bool State);

extern void init_ScreenRefresh(void);

extern void GetTime(int *Hr, int *Min, int *Sec);
extern void SetTime(int H, int I, int S);

extern void init_Bluetooth(void);
