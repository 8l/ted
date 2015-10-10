/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

typedef struct {
u_long fg, bg;
char fg_str[20],bg_str[20];
} ColorDef;

typedef struct {
char *keyword;
char *arg;
char coloridx;
} KW;
