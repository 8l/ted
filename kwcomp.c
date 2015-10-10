/*
	Copyright (C)   1997
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <dirent.h>
#include <string.h>
#include "ted.h"
#include "color.h"

extern KW *kwp;
extern int KwN;

static int mkw[256];
static int pg_ofs=0,mcount=0;

int pg_len()
{
return Min(Min(26,XW_ROW-4),mcount);
}

void disp_kwc_pg()
{
int eofs=Min(pg_ofs+pg_len(),mcount);
int i;
char tt[128];

if (input_handler!=5) return;
show_cursor_w();
inactive_cursor();
box(XW_COL-22,0,22,Min(pg_len()+2,mcount+2),7,8,"Keyword completion");
for(i=pg_ofs;i<eofs;i++) {
int no=i-pg_ofs;
set_att(9);
tt[0]=no+'a';
tt[1]=0;
gotoxy(XW_COL-20,no+1);
xputstrd(tt);

strcpy(tt,kwp[mkw[i]].keyword);
if (kwp[mkw[i]].arg) {
	strcat(tt,kwp[mkw[i]].arg);
}
tt[18]=0;
gotoxy(XW_COL-18,no+1);
set_att(6);
xputstrd(tt);
}
draw_scr();
}

static int slen;

K_KeywordComp()
{
int len=edbf[cursor_row].len;
char *ss=edbf[cursor_row].cstr;
char mstr[80], *pp;
int i;

if (!kwp) return;
if (!cursor_col) return;
i=cursor_col-1;
while (i>=0 && isvarchr(ss[i]) && (cursor_col-i<79)) i--;
i++;
slen=cursor_col-i;
if (!slen) return;
memcpy(mstr,ss+i,slen);
mstr[slen]=0;
pg_ofs=mcount=0;
for(i=3;i<KwN;i++)
if (strlen(pp=kwp[i].keyword) < slen) continue;
else
if (!memcmp(pp,mstr,slen)) {
	mkw[mcount++]=i;
#if	0
	printf("%d %s\n",mcount, pp);
#endif
}
if (!mcount) {
	bell();
	return;
}
if (mcount==1) {
	K_PutStr(kwp[mkw[0]].keyword + slen);
	return;
}
save_edstate();
inactive_cursor();
input_handler=5;
disp_kwc_pg();
}

void exit_kwcomp()
{
	input_handler=0;
	xgets_enter=-1;
	close_box();
	clr_xystr();
	redraw_twin();
}

void process_kwcomp(char *astr, int skey)
{
char ch=astr[0];

switch (skey) {
	case XK_Escape:
		exit_kwcomp();
		return;
	case XK_Prior:
		if (pg_ofs-pg_len() < 0) return;
		pg_ofs-=pg_len();
		disp_kwc_pg();
		return;
	case XK_Next:
		if (pg_ofs+pg_len() >= mcount) return;
		pg_ofs+=pg_len();
		disp_kwc_pg();
		return;
}

if (ch>='a' && ch<='z' || ch>='A' && ch<='Z') {
	int no=ch>='a'?ch-'a':ch-'A';
	if (no<0) no=9;
	if (pg_ofs+no >= mcount) return;
	exit_kwcomp();
	K_PutStr(kwp[mkw[pg_ofs+no]].keyword + slen);
	if (ch<='Z' && kwp[mkw[pg_ofs+no]].arg ) {
		K_PutStr(kwp[mkw[pg_ofs+no]].arg);
	}
	return;
}
}
