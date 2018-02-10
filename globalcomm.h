#define COMM_BUF_SIZE	64

extern void CommRead(void);
extern void VCOM_CheckSerialState (void);
extern int Comm_AvailChar(void);

extern void CommSend(const char *buffer, int length);
extern void CommSendByte(char Data);
extern void CommSendString(char *data);
extern void Comm_data(char *buffer, bool UpdatePntr);

extern void ParseCommand(void);
