

extern unsigned long SetupWAVData(unsigned long songLoc);
extern unsigned int ReadNextFromWAV(void);
extern void EndWAVData(void);

extern void Flash_EraseAll(void);
extern void Flash_WriteEnable(void);
extern void Flash_WaitForIt(void);
extern void Flash_WriteInit(unsigned long Address);
extern void Flash_WriteByte(unsigned char);
extern void Flash_WriteEnd(void);
extern void Flash_WriteBegin(unsigned long Address);
extern void Flash_EraseBytes(unsigned long Bytes);


//Flash memory reserved pointers to content location:
//  CheckByte1		= 	0
//	CheckByte2		=	1
//	CheckByte3		=	2
//	VERSION			=	3	(Used to auto clean memory when new firmware is released) TODO!
//	Second chime	=	4
//	'HET'			=	10
//	'IS'			=	16
//	'HALF'			=	22
//	Hour chime		=	28	(One tick for every hour)
//	'EEN'			=	34
//	'TWEE'			=	40
//	'DRIE'			=	46
//	'VIER'			=	52
//	'VIJF'			=	58
//	'ZES'			=	64
//	'ZEVEN'			=	70
//	'ACHT'			=	76
//	'NEGEN'			=	82
//	'TIEN'			=	88
//	'ELF'			=	94
//	'TWAALF'		=	100
//	'UUR'			=	106

// <startloc high><startloc mid><startloc low> <size high> <size mid> <size low>

// First 500 bytes are pointers and info, the rest is data


//Location of audio pointer in flash memory
extern const int SoundNumbers[12];
extern const int SoundHET;
extern const int SoundIS;
extern const int SoundHALF;
extern const int SoundUUR;


#define CHECKBYTE1	0
#define CHECKBYTE2	1
#define CHECKBYTE3	2
#define VERSION		3
