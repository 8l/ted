/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include <sys/types.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <signal.h>
#include <string.h>
#include "ted.h"
#include "color.h"

extern int batch;
int MAX_FILE=0;
EDSTATE *edstate=0;
int cur_file=0;

EDWIN edwin[4];

int cur_y,cur_x,win_x,win_y;            /* (x,y) on screen */
int cursor_row=0,cursor_col=0;  /* in data */
int insert_mode=1,insert_mode_=-1;
int DispTab=1;
int TabChar=1;

int page_row=0,page_col=0;
int page_len=M_ROW-2,page_width=M_COL;
#define INI_Alloc_Lines (100)
int Lines,Alloc_Lines;
LIN  *edbf;

void init_edbf()
{
if ((edbf=mmalloc(sizeof(LIN)*INI_Alloc_Lines,"init_edbf"))==NULL)
	exit(-1);
edstate[cur_file].edbf=edbf;
Alloc_Lines=INI_Alloc_Lines;
}

void incr_line()
{
Lines++;
if (Alloc_Lines<=Lines+1) {
	Alloc_Lines+=200;
	if ((edbf=(LIN *)mrealloc(edbf,
		sizeof(LIN)*Alloc_Lines,"incr_line"))==NULL)	exit(-1);
	edstate[cur_file].edbf=edbf;
}
}

void decr_line()
{
Lines--;
if (Alloc_Lines>Lines+201) {
	Alloc_Lines-=200;
	if ((edbf=(LIN *)mrealloc(edbf,sizeof(LIN)*Alloc_Lines,"err:decr_line"))
		==NULL) exit(-1);
	edstate[cur_file].edbf=edbf;
}
}

void ins_lines(int row, int lines)
{
int i;
Lines+=lines;
if (Alloc_Lines<=Lines+1) {
	Alloc_Lines+=Max(lines,200);
	if ((edbf=(LIN *)mrealloc(edbf,
		 sizeof(LIN)*Alloc_Lines,"err:ins_lines"))==NULL) exit(-1);
	edstate[cur_file].edbf=edbf;
}
for(i=Lines-lines-1;i>=row;i--) edbf[i+lines]=edbf[i];
}

void del_lines(int row0, int row1)
{
int row,dd=row1+1-row0;
char *str;
for(row=row0;row<=row1;row++)
	if (str=edbf[row].cstr) free(str);
for(row=row0;row+dd < Lines ;row++)
	edbf[row]=edbf[row+dd];
Lines-=dd;
}

void disp_eof(int y)
{
	gotoxy_w(0,y);
	xputstrd("==== End of file ====");
}

int LangN=0;
int LangType=0;

KW *LangKw[MaxLang], *kwp;
int LangKwN[MaxLang];

int KwN;
char *LangExt[MaxLang];

int LangMaxKeywordLen[MaxLang];

static int MaxKeywordLen;
char DirectCommentChar;
char DirectCommentChars[MaxLang];
int c_comment=1;

#include <regex.h>

extern ColorDef pc_color[];
int comment_color=0;
void set_lang_type(char *fname)
{
int len=strlen(fname);
int i,j,k;
char *p,ttt[20];
if (!len) return;
for(p=len+fname-1; p>=fname && *p!='.'; p--) ;
if (p<fname) {
	i=LangN;
	goto search_1st_line;
}
strcat(strcpy(ttt,p)," ");
for (i=0;i<LangN;i++) {
	if (strstr(LangExt[i],ttt))
		break;
}
search_1st_line:
if (i==LangN) {
	char tt[81];
	int len;
	char *str;

	if (!edbf) goto nomatch;
	str=edbf[0].cstr;
	len=edbf[0].len;
	if (!str) {
nomatch:
		LangType=-1;
		return;
	}
	if (len>=80) len=80;
	memcpy(tt,str,len);
	tt[len]=0;
	for(i=0;i<LangN;i++) {
		char exp[80],*a=LangExt[i],*b;
		int elen;
		regex_t	reg;

		while (*a) {
			if (!(a=strchr(a,'#'))) break;
			b=a;
			while (*b!=' ' && *b) b++;
			memcpy(exp,a,elen=b-a);
			exp[elen]=0;
#if	0
			printf("%s %s\n",exp,tt);
#endif
			if (regcomp(&reg,exp,0)) goto next_1;
			if (!regexec(&reg,tt,0,0,0)) {
				regfree(&reg);
				c_comment=0;
				goto match_lang;
			}
next_1:
			regfree(&reg);
			a=b;
		}
	}
	goto nomatch;
}
if (strstr(".c .h .cc .cpp .java",p)) c_comment=1;
else c_comment=0;
match_lang:
LangType=i;
kwp=LangKw[LangType];
MaxKeywordLen=LangMaxKeywordLen[LangType];
KwN=LangKwN[LangType];
#if	0
printf("LangType:%d\n",LangType);
printf("MKWL:%d\n",MaxKeywordLen);
printf("KwN:%d\n",KwN);
for(i=3;i<KwN;i++)
	printf("%s %d\n", kwp[i].keyword, kwp[i].coloridx);
puts("\n---------------------");
#endif
DirectCommentChar=DirectCommentChars[LangType];
}


int tab_width=8;
static int inmark,inlmark;
extern u_char cur_att;
static void mxputch(char ch)
{
u_char ttt;
if ((inmark || inlmark) && cur_att!=3) {
	ttt=cur_att;
	set_att(3);
	xputch(ch);
	set_att(ttt);
} else xputch(ch);
}

void disp_row_raw(int row)
{
int wc,len,i,j,dd;
char *lin;
int inmark,inlmark;

gotoxy_w(0,row-page_row);
inlmark=inmark=0;
if (in_mlines(row)) {
	set_att(3);
	inlmark=1;
} else set_att(0);

len=edbf[row].len;
lin=edbf[row].cstr;

for(wc=i=0;i<len;) {
	if (wc >= page_col) break;
	if (lin[i]==9)
	 wc+=tab_width-(wc%tab_width);
	else wc++;
	i++;
}
for(j=0;j<wc-page_col;j++)
	xputch(' ');
for(;wc-page_col<page_width && i<len;i++) {
	if (in_mblock(row,i)) {
		if (!inmark) {
			set_att(3);
			inmark=1;
		}
	} else
	if (inmark) {
		set_att(0);
		inmark=0;
	}
	switch (lin[i]) {
	 case 9:
	   dd=tab_width-(wc%tab_width);
	   wc+=dd;
	   if (DispTab)  {
			 xputch_w(TabChar);
	     for(j=1;j<dd;j++)xputch_w(' ');
	   } else
	     for(j=0;j<dd;j++)xputch_w(' ');
	   break;
	 default:
	   xputch(lin[i]);
	   wc++;
	}
}
if (inlmark)
	for(;wc-page_col<page_width;wc++) xputch(' ');
else
if (row>=mblk_row0 && row<=mblk_row1 && cur_file==mark_holder) {
	set_att(3);
	for(;wc-page_col<page_width;wc++)
	if (wc>=mblk_col0 && wc<=mblk_col1) {
		gotoxy_w(wc-page_col,row-page_row);
		xputch(' ');
	}
}
}

void scan_comment(frm,to)
{
int row,i, inquote;
int incmt=0;
if (frm<0) frm=0;
if (to>=Lines) to=Lines-1;
for(row=frm;row<=to;row++) {
	char *s=edbf[row].cstr;
	int len=edbf[row].len;
	edbf[row].flag=incmt;
	inquote=0;
	for(i=0;i<len-1;i++) {
		if (s[i]=='"') {
			if (!inquote) {
					inquote=1;
			} else {
				if (s[i-1]!='\\') inquote=0;
			}
		} else
		if (!inquote) {
			if (s[i]=='/' && s[i+1]=='*') incmt=1;
			else
			if (s[i]=='*' && s[i+1]=='/') incmt=0;
		}
	}
}
}

void disp_row(int row)
{
int wc,len,i,j,dd,inquote,basl,incmt,match;
char *lin;
int tatt, cmt_line;

#if	0
printf("row:%d  %d %d %d %d\n",
row, mblk_row0, mblk_col0, mblk_row1, mblk_col1 );
#endif

incmt=inquote=basl=0;
gotoxy_w(0,row-page_row);
cmt_line=inlmark=inmark=0;
if (in_mlines(row)) {
	set_att(tatt=3);
	inlmark=1;
} else set_att(tatt=0);

len=edbf[row].len;
lin=edbf[row].cstr;
if (c_comment)
	cmt_line=edbf[row].flag;

if (cmt_line) {
		incmt=1;
		set_att(kwp[2].coloridx);
} else
if (len && lin[0]==DirectCommentChar) tatt=kwp[0].coloridx;

wc=0;
#if	1
if (in_mblock(row,wc)) {
	set_att(3);
	inmark=1;
} else
#endif
if (inquote) set_att(kwp[1].coloridx);
else
if (!incmt) set_att(tatt);

if (inlmark) set_att(3);

for(wc=i=0;wc-page_col<page_width && i<len;) {
	if (in_mblock(row,i)) {
		inmark=1;
		set_att(3);
	} else
	if (inmark) {
		set_att(tatt);
		inmark=0;
	}

	switch (lin[i]) {
	 case 9:
	   dd=tab_width-(wc%tab_width);
	   if (inlmark||inmark) set_att(3);
	   if (DispTab)  {
	     if (wc++>=page_col)
			   xputch_w(TabChar);
	     for(j=1;j<dd;j++)
	       if (wc++>=page_col)
	         xputch_w(' ');
	   } else
	     for(j=0;j<dd;j++)
	       if (wc++>=page_col)
	         xputch_w(' ');
	   i++;
	   continue;
	 default:
	if (!inquote && c_comment && i<len-1
		&& (lin[i]=='/' && (lin[i+1]=='*' || lin[i+1]=='/') ||
		lin[i]=='*' && lin[i+1]=='/') ) {
		set_att(kwp[2].coloridx);
		if (wc++>=page_col)
		  mxputch(lin[i]);
		if (lin[i]=='/' && lin[i+1]=='/') {
			incmt=2;
		} else
		if (incmt!=2) incmt=1;
	}
	else
	if (!inquote && c_comment && incmt==1 && i &&
				lin[i-1]=='*' && lin[i]=='/') {
		if (wc++>=page_col)
		  mxputch(lin[i]);
		incmt=0;
		set_att(0);
	}
	else
	if (incmt) {
		if (wc++>=page_col)
		  mxputch(lin[i]);
	} else
	/* for the # char in perl and shell */
	if (!c_comment && lin[0]==DirectCommentChar && !inquote) {
		set_att(kwp[0].coloridx);
		if (wc++>=page_col)
		  mxputch(lin[i]);
		incmt=2;
	} else
	if (lin[i]=='"' || lin[i]=='\'') {
		if (!inquote) {
			inquote=lin[i];
			set_att(kwp[1].coloridx);
		  if (wc++>=page_col)
			  mxputch(lin[i]);
		}
		else {
			if (!basl) {
		    if (wc++>=page_col)
				  mxputch(lin[i]);
				if (lin[i]==inquote) {
					inquote=0;
					set_att(0);
				}
			} else {
		    if (wc++>=page_col)
				  mxputch(lin[i]);
				basl=0;
			}
		}
	}
	else {
		if (inquote && lin[i]=='\\') basl^=1;
		else basl=0;
		if (!inquote) {
if ((!i || !isvarchr(lin[i-1])) && isvarchr(lin[i]) && lin[0]!=
				DirectCommentChar) {
			int k,vl=0;
			char tstr[4096],*p;
			j=i;
			while (j<len && (isvarchr(lin[j])|| (j && isdigit(lin[j]))))
				tstr[vl++]=lin[j++];
			tstr[vl]=0;
			match=0;
			if (vl<=MaxKeywordLen) {
				int rr;
				for(k=3;k<KwN;k++) {
				  rr=strcmp(kwp[k].keyword,tstr);
				  if (rr > 0) break;
				  else
				  if (rr==0) {
						set_att(tatt=kwp[k].coloridx);
						match=1;
						goto lll6;
					}
				}
			}
			if (!match) goto lll1;
lll6:
			for(p=tstr; *p && wc-page_col<page_width;p++) {
				if (in_mblock(row,i)) {
					inmark=1;
					set_att(3);
				} else {
					inmark=0;
					set_att(tatt);
				}
		    if (wc++>=page_col)
				  mxputch(*p);
				/* wc++; */
				i++;
			}
			set_att(0);
			if (*p) goto lll4;
			goto lll5;
} else if (!isvarchr(lin[i]) && lin[0]!=DirectCommentChar)
			set_att(0);
		}
lll1:
		if (wc++>=page_col)
		  mxputch(lin[i]);
	}
	}
	i++;
lll5:
	0;
}
lll4:
if (inlmark) {
	set_att(3);
	for(;wc-page_col<page_width;wc++) xputch(' ');
}
else
if (row>=mblk_row0 && row<=mblk_row1 && cur_file==mark_holder) {
	set_att(3);
	for(;wc-page_col<page_width;wc++)
	if (wc>=mblk_col0 && wc<=mblk_col1) {
		gotoxy_w(wc-page_col,row-page_row);
		xputch(' ');
	}
}
}

void disp_page()
{
int row;
if (batch) return;
clrscr_w();
if (c_comment) {
/* if there are more than 300 lines in a comment, this will fail */
	scan_comment(cursor_row-300,cursor_row+page_len);
}
for(row=page_row;row<page_row+page_len && row < Lines; row++) {
	if (LangType>=0)
		disp_row(row);
	else
		disp_row_raw(row);
}
if (row-page_row < page_len) {
	set_att(0);
	disp_eof(row-page_row);
}
flush_buffer();
draw_scr();
set_att(0);
}

void disp_cursor_line()
{
if (batch) return;
clr_line_w(cur_y);
if (LangType>=0)
	disp_row(cursor_row);
else
	disp_row_raw(cursor_row);
flush_buffer();
draw_line_w(cur_y);
}


col2x(int xx)
{
	int len,i,ww;
	char *cp;

	len=edbf[cursor_row].len;
	cp=edbf[cursor_row].cstr;
	ww=0;
	for(i=0;i<xx && i<len ;i++)
	if (cp[i]==9)
		ww+=tab_width-(ww%tab_width);
	else
		ww++;
	return ww-page_col;
}


x2eol()
{
	int len,i,ww;
	char *cp;

	len=edbf[cursor_row].len;
	cp=edbf[cursor_row].cstr;
	ww=0;
	for(i=0;i<len ;i++)
	if (cp[i]==9)
		ww+=tab_width-(ww%tab_width);
	else
		ww++;
	return cur_x-(ww-page_col);
}


void x2col()
{
	int len,i,ww, xx;
	char *cp;

	len=edbf[cursor_row].len;
	cp=edbf[cursor_row].cstr;
	xx=page_col+cur_x;
	ww=0;
	for(i=0;i<len && ww<xx;i++) {
		if (cp[i]==9) {
			ww+=tab_width-(ww%tab_width);
			if (ww > xx) break;
		}
		else
			ww++;
	}
	cursor_col=i;
#if	0
	printf("cursor_col:%d\n",cursor_col);
#endif
}


void adj_page_col()
{
	if (cur_x<0) {
		page_col=page_col+cur_x;
		cur_x=0;
		if (page_col<0) page_col=0;
		disp_page();
	} else
	if (cur_x>=page_width) {
		int dd=cur_x-page_width+1;
		page_col+=dd;
		cur_x-=dd;
		disp_page();
	}
}

void center_cursor_row()
{
page_row=cursor_row-(page_len>>1);
if (page_row<0) page_row=0;
disp_page();
pos_cur();
show_cursor_w();
}

void ch_left_adj()
{
#if	CHINESE
	if (cur_x && B2nd(cur_y,cur_x-1)) {
		cur_x-=2;
		cursor_col-=2;
	} else
#endif
	{
		cur_x--;
		cursor_col--;
	}
}

void disp_cur_pos()
{
static int llen;
char ttt[20];
int xx,len;

if (batch) return;
gotoxy_w(MCOL-10,MROW-1);
sprintf(ttt,"%d  %d", cursor_row+1, page_col+cur_x+1);
len=strlen(ttt);
set_att(0);
xputstr(ttt);
for(xx=0;xx<llen-len;xx++) xprintf(" ");
llen=len;
}

void draw_cmd_line()
{
int i;
if (batch) return;
	gotoxy_w(0,page_len);
	set_att(2);
	for(i=0;i<MCOL;i++)
		xputstr(" ");
	set_att(0);
	show_time();
	disp_cdir();
}

extern int Indent;
int Indent_=-1;
void disp_ins_mod()
{
if (batch) return;
	if (insert_mode==insert_mode_) goto lll1;
	insert_mode_=insert_mode;
	gotoxy_w(MCOL-20,MROW-1);
	if (insert_mode) xputstr("Ins");
	else
	xputstr("Ovw");
lll1:
	if (Indent==Indent_) return;
	Indent_=Indent;
	gotoxy_w(MCOL-16,MROW-1);
	if (Indent) xputstr("In't");
	else
	xputstr("    ");
}

#include <stdarg.h>
void message(int att,char *fmt,...)
{
va_list args;
char tmp[128];

if (batch) return;
if (att==4) bell();
va_start(args, fmt);
vsprintf(tmp, fmt, args);
va_end(args);
gotoxy_w(1,MROW-1);
set_att(att);
xputstr(tmp);
hide_cursor_w();
mesg=1;
set_att(0);
}

char *substr(char *s, int len)
{
static char ttt[256];
if (len<0) len=0;
strcpy(ttt,s);
ttt[len]=0;
return ttt;
}

int f_modified=0,file_modified=-1;
int mesg;
void disp_fname()
{
char ttt[128];
int i,len;
extern int cur_file,file_no;

if (batch) return;
if (mesg) {
	mesg=0;
	file_modified=-1;
	return;
}
if (f_modified==file_modified) return;
file_modified=f_modified;
gotoxy_w(0,MROW-1);
if (f_modified) set_att(4);
else
	set_att(0);
sprintf(ttt," %s  %d/%d",substr(fname,page_width-26),cur_file+1,file_no);
xputstr(ttt);
len=strlen(ttt);
set_att(0);
#if	0
for(i=0;i<33-len;i++) xputstr(" ");
#else
for(i=0;i<page_width-len;i++) xputstr(" ");
#endif
insert_mode_=Indent_=-1;
disp_ins_mod();
}


void redraw_status()
{
insert_mode_=Indent_=file_modified=-1;
if (!mesg) {
	clr_line_w(MROW-1);
}
else {
	gotoxy(16,MROW-1);
	clr_to_eol();
}
disp_fname();
disp_ins_mod();
draw_cmd_line();
disp_cur_pos();
draw_scr();
disp_kb_mode();
}

void disp_mode()
{
file_modified=Indent_=insert_mode_=-1;
}

int input_handler;

static int getch_x, getch_y, getch_defch;
static void (*getch_action)();
static char *getch_allow;
void setup_getch(int x, int y, char *allow, int defch, void (*action)())
{
	getch_x=x;
	getch_y=y;
	getch_defch=defch;
	getch_action=action;
	getch_allow=allow;
	gotoxy_w(x,y);
	xprintf("%c ",defch);
	gotoxy_w(x,y);
	show_cursor_w();
	input_handler=2;
}

void process_getch(char *astr, int skey, int state)
{
	int ch=astr[0];
	if (!ch) return;
	if (skey==XK_Return) {
		input_handler=0;
		getch_action(mtoupper(getch_defch));
	}
	else if (skey==XK_Escape) {
		input_handler=0;
		redraw_status();
	}
	else
	if (strchr(getch_allow,ch)) {
		input_handler=0;
		getch_action(mtoupper(ch));
	}
	return;
}

void f_modified_quit_getch(int ch)
{
	if (mtoupper(ch)=='Y')
		close_file(0);
}

int FModifiedQuitYN=1;
void close_quit_file()
{
	if (f_modified) {
		message(4,"Not saved. Quit (Y/N):",fname);
		setup_getch(23,MROW-1,"YNyn",FModifiedQuitYN?'Y':'N',
		  f_modified_quit_getch);
	}
	else
	close_file();
}

void rowcol_yx(int row,int col, int *y,int *x)
{
int len,i,ww;
char *cp;

len=edbf[row].len;
cp=edbf[row].cstr;
ww=0;
for(i=0;i<col && i<len ;i++)
if (cp[i]==9)
	ww+=tab_width-(ww%tab_width);
else
	ww++;
*y=row-page_row;
*x=ww-page_col;
}

int bracket_match=0;
void check_bracket(int beyond)
{
char *s=edbf[cursor_row].cstr;
int i,dire,row,x,y,cnt,len=edbf[cursor_row].len;
char ch,mch;
if (!s || len<=cursor_col) return;
ch=s[cursor_col];
switch (ch) {
	case '(':
		mch=')'; dire=1;
		break;
	case ')':
		mch='('; dire=0;
		break;
	case '[':
		mch=']'; dire=1;
		break;
	case ']':
		mch='['; dire=0;
		break;
	case '{':
		mch='}'; dire=1;
		break;
	case '}':
		mch='{'; dire=0;
		break;
	default:
		return;
}
cnt=0;
row=cursor_row;
if (dire) {
	for(i=cursor_col+1;i<len; i++) {
		if (s[i]==ch) cnt++;
		else
		if (s[i]==mch) {
			if (!cnt) goto match;
			cnt--;
		}
	}
	for(row=cursor_row+1; (row<page_row+page_len || beyond) &&
		row < Lines; row++) {
		if (!(s=edbf[row].cstr)) continue;
		len=edbf[row].len;
		for(i=0; i<len; i++) {
			if (s[i]==ch) cnt++;
			else
			if (s[i]==mch) {
				if (!cnt) goto match;
				cnt--;
			}
		}
	}
	return;
} else {
	for(i=cursor_col-1;i>=0; i--) {
		if (s[i]==ch) cnt++;
		else
		if (s[i]==mch) {
			if (!cnt) goto match;
			cnt--;
		}
	}
	for(row=cursor_row-1;row>=page_row||(beyond && row>=0); row--) {
		if (!(s=edbf[row].cstr)) continue;
		len=edbf[row].len;
		for(i=len-1;i>=0 ; i--) {
			if (s[i]==ch) cnt++;
			else
			if (s[i]==mch) {
				if (!cnt) goto match;
				cnt--;
			}
		}
	}
	return;
}
match:
if (beyond) {
if (dire) {
	if (row >= page_row+page_len)
		page_row=row-page_len+3;
	cursor_row=row;
	cursor_col=i;
	pos_cur();
	disp_page();
	redraw_status();
	show_cursor_w();
	return;
} else
if (!dire) {
	if (row<page_row)
		page_row=row-3;
	if (page_row<0) page_row=0;
	cursor_row=row;
	cursor_col=i;
	pos_cur();
	disp_page();
	redraw_status();
	show_cursor_w();
	return;
}
}

rowcol_yx(row,i,&y,&x);
if (x>=page_width) return;
set_att(10);
gotoxy_w(x,y);
xprintf("%c",mch);
bracket_match=1;
}

K_MatchBracket()
{
check_bracket(1);
}

K_cur_up()
{
if (!page_row && !cur_y) return 1;
hide_cursor_w();
if (cursor_row) cursor_row--;
if (!cur_y && page_row) {
	page_row--;
	scroll_down_w(0);
	disp_cursor_line();
	draw_scr();
}
cur_y=cursor_row-page_row;
x2col(cur_x);
show_cursor_w();
return 0;
}

K_cur_upN(int n)
{
int i;
for(i=0;i<n;i++)
	K_cur_up();
}

K_cur_down()
{
if (cursor_row >= Lines-1) return 1;
hide_cursor_w();
cursor_row++;
if (cur_y==page_len-1 && page_row < Lines - 1) {
	page_row++;
	if (c_comment)
		disp_page();
	else {
		scroll_up_w(0);
		disp_cursor_line();
		draw_scr();
	}
}
cur_y=cursor_row - page_row;
x2col(cur_x);
show_cursor_w();
return 0;
}

K_cur_downN(int n)
{
int i;
for(i=0;i<n;i++)
	K_cur_down();
}

K_cur_leftN(int n)
{
int i;

for(i=0;i<n;i++)
	K_cur_left();
}

int CurAutoUpDown=0;

K_cur_left()
{
char *cp;
int len,xx;
int xlen;

xlen=x2eol();
if (xlen > 0) { /* beyond eol */
	hide_cursor_w();
	cur_x--;
	goto lll2;
}

if (!cursor_col) {
	if (CurAutoUpDown) {
		if (!cursor_row) return 1;
		cursor_row--;
		if (cursor_row<page_row) page_row=cursor_row;
		cursor_col=edbf[cursor_row].len;
		pos_cur();
		adj_page_col();
		disp_page();
		show_cursor_w();
		return 0;
	} else {
		if (cur_x) {
			cur_x=0;
			pos_cur();
			adj_page_col();
			disp_page();
			show_cursor_w();
			return 0;
		} else return 1;
	}
}
hide_cursor_w();
cp=edbf[cursor_row].cstr;
len=edbf[cursor_row].len;
if (cursor_col>=len) {
	cursor_col=len;
	goto lll1;
} else
if (cp[cursor_col]==9 ||(cursor_col && cp[cursor_col-1]==9)){
lll1:
xx=col2x(cursor_col);
if (xx!=cur_x) cur_x=xx;
else {
	ch_left_adj();
	cur_x=col2x(cursor_col);
}
} else ch_left_adj();
lll2:
adj_page_col();
check_bracket(0);
show_cursor_w();
return 0;
}

K_cur_right()
{
char *cp;

if (cursor_col>=edbf[cursor_row].len) {
	if (CurAutoUpDown) {
		if (cursor_row>=Lines-1) return 1;
		cursor_row++;
		if (cursor_row>=page_row+page_len)
			page_row=cursor_row-page_len;
		cursor_col=0;
		pos_cur();
		adj_page_col();
		disp_page();
		show_cursor_w();
		return 0;
	} else {
		hide_cursor_w();
		if (cur_x >= page_width-1) {
			page_col++;
			disp_page();
		} else cur_x++;
		show_cursor_w();
		return 0;
	}
}
hide_cursor_w();
cp=edbf[cursor_row].cstr;
if (cp[cursor_col]==9) {
	cur_x=col2x(cursor_col+1);
} else {
#if	CHINESE
	if (cur_x<page_width-1 && B2nd(cur_y,cur_x+1)) {
			cur_x+=2;
			cursor_col++;
	}
	else
#endif
	cur_x++;
}
cursor_col++;
adj_page_col();
check_bracket(0);
show_cursor_w();
return 0;
}

isvarnum(int ch)
{
return (isvarchr(ch) || (ch>='0' && ch<='9'));
}

K_CurLeftWord()
{
char *cp;
int len,xx;

if (!cursor_col) {
	if (cur_x>0) {
		hide_cursor_w();
		cur_x=0;
		goto ll1;
	}
	return 1;
}
hide_cursor_w();
cp=edbf[cursor_row].cstr;
len=edbf[cursor_row].len;
if (isvarnum(cp[cursor_col])) {
	cursor_col--;
	if (isvarnum(cp[cursor_col])) {
		while(cursor_col && isvarnum(cp[cursor_col])) cursor_col--;
	} else {
		while(cursor_col && !isvarnum(cp[cursor_col])) cursor_col--;
		while(cursor_col && isvarnum(cp[cursor_col])) cursor_col--;
	}
	if (!isvarnum(cp[cursor_col])) cursor_col++;
} else {
	cursor_col--;
	while(cursor_col && !isvarnum(cp[cursor_col])) cursor_col--;
	while(cursor_col && isvarnum(cp[cursor_col])) cursor_col--;
	if (!isvarnum(cp[cursor_col])) cursor_col++;
}
ll1:
pos_cur();
adj_page_col();
disp_page();
show_cursor_w();
return 0;
}


K_CurRightWord()
{
char *cp;
int len,xx;

hide_cursor_w();
cp=edbf[cursor_row].cstr;
len=edbf[cursor_row].len;
if (!cursor_col >= len)
	return 1;
if (isvarnum(cp[cursor_col])) {
	while(cursor_col<len && isvarnum(cp[cursor_col])) cursor_col++;
	while(cursor_col<len && !isvarnum(cp[cursor_col])) cursor_col++;
} else 	while(cursor_col<len && !isvarnum(cp[cursor_col])) cursor_col++;
pos_cur();
adj_page_col();
disp_page();
show_cursor_w();
return 0;
}


K_pgup()
{
if (!page_row) return 1;
page_row-=page_len-1;
if (page_row < 0) page_row=0;
cursor_row-=page_len-1;
if (cursor_row < 0) cursor_row=0;
cur_y=cursor_row-page_row;
disp_page();
show_cursor_w();
x2col(cur_x);
return 0;
}

K_pgdn()
{
if (Lines<page_len) return 1;
page_row+=page_len-1;
if (page_row>=Lines) page_row=Lines-1;
cursor_row+=page_len -1;
if (cursor_row>=Lines) cursor_row=Lines-1;
cur_y=cursor_row-page_row;
disp_page();
show_cursor_w();
x2col(cur_x);
return 0;
}


K_spgup()
{
if (!page_row) return 1;
hide_cursor_w();
page_row--;
if (cursor_row) cursor_row--;
cur_y=cursor_row-page_row;
disp_page();
show_cursor_w();
return 0;
}

K_spgdn()
{
if (page_row<Lines-1) page_row++;
else return 1;
hide_cursor_w();
if (cursor_row<Lines-1) cursor_row++;
cur_y=cursor_row-page_row;
disp_page();
show_cursor_w();
return 0;
}

K_home()
{
hide_cursor_w();
if (page_col) {
	page_col=0;
	disp_page();
}
cursor_col=cur_x=0;
show_cursor_w();
return 0;
}

K_end()
{
hide_cursor_w();
cursor_col=edbf[cursor_row].len;
cur_x=col2x(cursor_col);
adj_page_col();
show_cursor_w();
return 0;
}

K_quit()
{
close_quit_file();
return 0;
}

K_quit2()
{
nosave=1;
close_file();
return 0;
}

K_TogIns()
{
insert_mode^=1;
return 0;
}

K_EdFile()
{
enable_read_file();
return 1;
}

K_FindStr()
{
enable_start_search();
return 1;
}

K_RepStr()
{
enable_start_replace();
return 1;
}

K_WrFile()
{
enable_write_file();
return 1;
}

K_JoinLine()
{
s_fmod();
u_split_line(edbf[cursor_row].len,1);
join_line();
show_cursor_w();
return 0;
}

K_FileBrowser()
{
	input_handler=4;
	save_win_context(active_win);
	inactive_cursor();
	Draw_file_browser();
	return 1;
}

K_TogDispTab()
{
DispTab^=1;
disp_page();
show_cursor_w();
}

void handle_key(char *astr, int skey, int state)
{
	if (input_handler==1) {
		process_edln(astr,skey,state);
		return;
	} else if (input_handler==2) {
		process_getch(astr,skey,state);
		if (input_handler==0) {
			redraw_status();
			show_cursor_w();
		}
		return;
	} else if (input_handler==3) {
		cwin_handler(skey);
		return;
	} else if (input_handler==4) {
		process_file_browser(skey,0,0);
		return;
	} else if (input_handler==5) {
		process_kwcomp(astr,skey);
		return;
	}
	if (bracket_match) {
		bracket_match=0;
		disp_page();
		show_cursor_w();
	}
	feed_key(skey,state,astr[0]);
}

void setup_xgets()
{
	search_menu_init();
	read_file_init();
	goto_line_init();
	replace_init();
	write_file_init();
	chdir_init();
	include_file_init();
	lookup_tag_init();
	ExecCMd_init();
	SetWordWrap_init();
	write_blk_init();
}

static void ign_intr(int a)
{
signal(SIGINT,ign_intr);
}

int fnamesN=0, fnamesN_a=10;
char *fnames[512];
int fgps=0,bgps=1;

void put_arg_fname(char *s)
{
fnames[fnamesN++]=strdup(s);
}

char *exec_name;

main(int argc, char **argv)
{
int i;
extern int cdtedrc;
extern int ReLoadTed;
extern char *ini_fcmd,*goto_tagname;

set_kdef();
exec_name=argv[0];
K_LoadTedrc(tedrcfname);

init_X_arg(argc,argv);
#ifndef DEBUG
if (!fgps) fgps=!bgps;
if (!fgps) {
	int pid;
	if ((pid=fork())<0) error("Cannot fork a process");
	if (pid>0) exit(0);
}
#endif
if (!batch) init_X_win(argc,argv);
get_cdir();
setup_xgets();
init_tag();
init_undo_fname();
get_hostname();
init_edwin_state();
if (!fnamesN) {
	if (goto_tagname) {
		if (lookup_tag(goto_tagname))
			p_err("cannot lookup tag '%s'",goto_tagname);
	} else
	edit_file(fname);
}
else {
	for (i=0;i<fnamesN;i++) {
		edit_file(fnames[i]);
	}
	goto_file(0);
}
ReLoadTed=0;

if (ini_fcmd) {
	char *cmd,cmdstr[1024];
	char count;
	FILE *fp;
	if ((fp=fopen(ini_fcmd,"r"))==NULL)
		p_err("cannot open %s",ini_fcmd);
	fgets(cmdstr,sizeof(cmdstr),fp);
	if (cmdstr[strlen(cmdstr)-1]=='\n')
		cmdstr[strlen(cmdstr)-1]=0;
	if (parse_stmt(ini_fcmd,0,cmdstr,&cmd,&count))
		ExecCommand(cmd,count);
	free(cmd);
}
if (batch) exit(0);
AutoMouseFocus();
run_event();
}
