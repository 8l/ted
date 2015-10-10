/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

typedef struct {
char *keyname;
KeySym xkey;
} KEYMAP;

#define CTRL (0x100)
#define SHFT (0x200)
#define ALT (0x400)

typedef struct {
char *funname;
int (*action)();
char argtype,modfunc;
} FUNCMAP;

typedef struct {
u_short kcode[4];
char *cmd;
u_char klen,type;
} KEYDEF;

#define KEYHASH(x) (x>>4)
