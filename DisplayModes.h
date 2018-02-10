extern void Frontpanel_DefaultText(int Hours, int Minutes, int Seconds, int Notation, bool MinutesOFF, bool HoursOFF);

extern bool FrontpanelMode_RandomIntro(int Hours, int Minutes, int Seconds, int Notation);
extern bool FrontpanelMode_Basic(int Hours, int Minutes, int Seconds, int Notation);
extern bool FrontpanelMode_FallingStars(int Hours, int Minutes, int Seconds, int Notation);
extern bool FrontpanelMode_HorizontalSlideLeft(int Hours, int Minutes, int Seconds, int Notation);
extern bool FrontpanelMode_HorizontalSlideRight(int Hours, int Minutes, int Seconds, int Notation);
extern bool FrontpanelMode_DissolveOutIn(int Hours, int Minutes, int Seconds, int Notation);
extern bool FrontpanelMode_DissolveCross(int Hours, int Minutes, int Seconds, int Notation);
extern void Frontpanel_DOTS(int Hours, int Minutes, int Seconds, int Notation);

struct Location{
	int X;
	int Y;
	int Chars;
} Location;

struct Texts{
	//NEDERLANDS
	struct Location HET;
	struct Location IS;
	struct Location OVER;
	struct Location VOOR;
	struct Location UUR;
	struct Location KWART;
	struct Location HALF;
	
	//ENGLISH
	struct Location A;
	struct Location AM;
	struct Location PM;
	struct Location TEEN; //TEEN
	struct Location TWENTY; //TWENTY
	struct Location DASH1; //(TEEN)-
	struct Location DASH2; //(TWENTY)-
};

struct Front{
	struct Texts	Text;
	struct Location	Hour[13];
	struct Location	Minute[15];
};

extern volatile int TransSpeedFactor;
