/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include <sys/types.h>
#include "ted.h"

int mark_holder=0;
int mline_begin=-1;
int mline_end=-1;

int mblk_row0=-1,mblk_col0=-1;
int mblk_row1=-1,mblk_col1=-1;

void swapint(int *a, int *b)
{
	int tmp;
	tmp=*a;
	*a=*b;
	*b=tmp;
}

void clr_mark()
{
	mline_begin=mline_end=-1;
	mblk_row0=mblk_row1=mblk_col0=mblk_col1=-1;
}

void chg_mark_owner()
{
if (mark_holder!=cur_file) {
	clr_mark();
	mark_holder=cur_file;
	save_win_context(active_win);
	redraw_twin();
}
}

K_MarkLn()
{
chg_mark_owner();
if (mline_begin<0) {
	mline_end=mline_begin=cursor_row;
}
else {
	if (cursor_row < mline_begin) mline_begin=cursor_row;
	else
	if (cursor_row > mline_end) mline_end=cursor_row;
	else
	if (cursor_row < mline_end)
		mline_end=cursor_row;
}
set_selection();
mblk_row0=mblk_row1=mblk_col0=mblk_col1=-1;
disp_page();
show_cursor_w();
}

void mark_liney(int y)
{
y=y-win_y;
if (y<0 || y>=page_len) return;
y+=page_row;
if (y>=Lines) return;
chg_mark_owner();
if (mline_begin<0) mline_begin=mline_end=y;
else
if (y <= mline_begin) mline_begin=y;
else mline_end=y;
mblk_row0=mblk_col0=mblk_row1=mblk_col1=-1;
disp_page();
show_cursor_w();
}

void mark_liney2(int y)
{
chg_mark_owner();
y=y-win_y;
if (y<0 || y>=page_len) return;
y+=page_row;
if (y>=Lines) return;
if (mline_begin<0) mline_begin=mline_end=y;
else
if (y > mline_begin) mline_begin=y;
mblk_row0=mblk_col0=mblk_row1=mblk_col1=-1;
disp_page();
show_cursor_w();
}

void untab_lines(int frm, int to)
{
int i,j,k;
int len,wc,nlen,tab;
char *str,*cstr;
int nmblk_col0=mblk_col0;
int nmblk_col1=mblk_col1;

for(i=frm;i<=to;i++) {
	len=edbf[i].len;
	str=edbf[i].cstr;
	if (!str) continue;
	wc=0;
	tab=0;
	for(j=0;j<len;j++)
		if (str[j]==9) {
			wc+=tab_width-(wc%tab_width);
			tab=1;
		} else wc++;
	if (!tab) continue;
	nlen=wc;
	if ((cstr=mmalloc(nlen,"untab_lines"))==NULL)
		return;
	wc=0;
	for(j=0;j<len;j++) {
		if (i==mblk_row0 && j==mblk_col0) nmblk_col0=wc;
		if (i==mblk_row1 && j==mblk_col1) nmblk_col1=wc;
		if (str[j]==9) {
			k=tab_width-(wc%tab_width);
			memset(&cstr[wc],' ',k);
			wc+=k;
		} else cstr[wc++]=str[j];
	}
	free(str);
	edbf[i].cstr=cstr;
	edbf[i].len=nlen;
}
#if	0
printf("mblk_col0:%d %d\n",mblk_col0,mblk_col1);
#endif
mblk_col0=nmblk_col0;
mblk_col1=nmblk_col1;
if (cursor_row>=frm && cursor_row<=to) {
	cursor_col=page_col+cur_x;
#if	0
	printf("$$$ cursor_col:%d\n",cursor_col);
#endif
}
}

K_MarkBlk()
{
chg_mark_owner();
untab_lines(cursor_row,cursor_row);
if (mblk_row0 < 0) {
	mblk_row0=mblk_row1=cursor_row;
	mblk_col0=mblk_col1=cursor_col;
} else {
	if (cursor_row < mblk_row0) mblk_row0=cursor_row;
	else
	if (cursor_row > mblk_row1) mblk_row1=cursor_row;
	else
		mblk_row1=cursor_row;
	if (cursor_col < mblk_col0) mblk_col0=cursor_col;
	else
	if (cursor_col > mblk_col1) mblk_col1=cursor_col;
	else
		mblk_col1=cursor_col;
}
untab_lines(mblk_row0,mblk_row1);
set_selection();
mline_begin=mline_end=-1;
disp_page();
show_cursor_w();
return 0;
}

K_MarkBlkxy(int x, int y)
{
x-=win_x;
y-=win_y;
if (x>=page_width||y>=page_len) return;
x+=page_col;
y+=page_row;
if (y>=Lines) return;
chg_mark_owner();
untab_lines(y,y);
if (mblk_row0 < 0) {
	mblk_row0=mblk_row1=y;
	mblk_col0=mblk_col1=x;
} else {
	if (y < mblk_row0) mblk_row0=y;
	else
	if (y > mblk_row1) mblk_row1=y;
	else
		mblk_row1=y;
	if (x < mblk_col0) mblk_col0=x;
	else
	if (x > mblk_col1) mblk_col1=x;
	else
		mblk_col1=x;
}
untab_lines(mblk_row0,mblk_row1);
set_selection();
mline_begin=mline_end=-1;
disp_page();
show_cursor_w();
return 0;
}

void mark_blkxy(int x0,int x1,int y0)
{
if (x0==x1) return;
x0-=win_x;
x1-=win_x;
y0-=win_y;
if (x0<0 || x1<0 || y0<0 || x0>=page_width || x1>=page_width || y0>=page_len)
	return;
x1+=page_col;
x0+=page_col;
y0+=page_row;
if (y0>=Lines) return;
chg_mark_owner();
mblk_row0=mblk_row1=y0;
untab_lines(mblk_row0,mblk_row1);
if (x0>x1) {
	mblk_col0=x1;
	mblk_col1=x0;
}
else {
	mblk_col0=x0;
	mblk_col1=x1;
}
mline_begin=mline_end=-1;
disp_page();
show_cursor_w();
}

in_mlines(int row)
{
if (cur_file==mark_holder && row >= mline_begin && row <= mline_end)
	return 1;
else	return 0;
}

in_mblock(int row,int col)
{
if (cur_file==mark_holder && row>=mblk_row0 &&row<=mblk_row1 &&
 col >= mblk_col0 && col <= mblk_col1)
	return 1;
else return 0;
}


K_Unmark()
{
clr_mark();
disp_page();
show_cursor_w();
return 0;
}

void copy_mlines()
{
char *str;
int i,len;
int ilen=mline_end-mline_begin+1;
LIN *edbf0=edstate[mark_holder].edbf;

if (cur_file==mark_holder && cursor_row>=mline_begin && cursor_row<mline_end){
	message("source-destination conflict");
	return;
}
ins_lines(cursor_row+1,ilen);
if (cur_file==mark_holder) edbf0=edbf;
if (cur_file==mark_holder && cursor_row < mline_begin) {
	mline_begin+=ilen;
	mline_end+=ilen;
}
for(i=0;i<ilen;i++) {
	len=edbf[cursor_row+1+i].len=edbf0[mline_begin+i].len;
	if (len) {
		if ((str=mmalloc(len,"copy_lines"))==NULL)
			return;
		memcpy(str,edbf0[mline_begin+i].cstr,len);
	} else str=0;
	edbf[cursor_row+i+1].cstr=str;
}
u_del_lines(cursor_row+1,cursor_row+ilen,1);
disp_page();
show_cursor_w();
}

void del_mlines()
{
if (cur_file!=mark_holder) return;
del_lines(mline_begin,mline_end);
if (cursor_row > mline_end)
	cursor_row-=mline_end-mline_begin+1;
else
if (cursor_row>=mline_begin && cursor_row <=mline_end) {
	cursor_row=mline_begin;
}
if (page_row>=mline_begin && page_row<=mline_end) {
	page_row=mline_begin;
}
mline_begin=mline_end=-1;
if (Lines<=0) {
	Lines=1;
	edbf[0].cstr=0;
	cursor_col=edbf[0].len=0;
}
if (cursor_row>=Lines) cursor_row=Lines-1;
if (cursor_row<0) cursor_row=0;
pos_cur();
adj_page_col();
disp_page();
show_cursor_w();
}

void del_mblock()
{
int i,row,len,dd=mblk_col1-mblk_col0+1;
char *cstr;

if (cur_file!=mark_holder) return;
for(row=mblk_row0;row<=mblk_row1;row++) {
	cstr=edbf[row].cstr;
	if (!cstr) continue;
	len=edbf[row].len;
	if (mblk_col0>=len) continue;
	for(i=mblk_col0;i+dd<len;i++)
		cstr[i]=cstr[i+dd];
	if (mblk_col1>=len-1)
		len=mblk_col0;
	else
		len=len-dd;
	if (len < 0) len=0;
	if (len==0) {
		free(cstr);
		cstr=0;
	}
	else
	if ((cstr=(char *)mrealloc(cstr,len,"del_mblock"))==NULL)
		return;
	edbf[row].cstr=cstr;
	edbf[row].len=len;
}
mblk_row0=mblk_row1=mblk_col0=mblk_col1=-1;
pos_cur();
adj_page_col();
disp_page();
show_cursor_w();
}

K_DelArea()
{
if (cur_file!=mark_holder) return 1;
if (mline_begin>=0) {
	s_fmod();
	u_ins_mark_lines(1);
	del_mlines();
}
else
if (mblk_row0>=0) {
	int row1=cursor_row+(mblk_row1-mblk_row0);
	int col1=cursor_col+(mblk_col1-mblk_col0);
	u_rep_mark_lines(mblk_row0,mblk_col0,mblk_row1,mblk_col1,1);
	s_fmod();
	del_mblock();
}
return 0;
}

void copy_block()
{
int row,mrow,len,blen=mblk_col1-mblk_col0+1;
int olen,nlen,i,slen;
char *str,*ss;
int bln=mblk_row1-mblk_row0+1;
LIN *edbf0=edstate[mark_holder].edbf;

if (mblk_row0 < 0 || cursor_row+bln>Lines) return;
untab_lines(cursor_row,cursor_row+bln-1);
cursor_col=page_col+cur_x;
for(mrow=mblk_row0;mrow<=mblk_row1;mrow++)
{
	row=mrow-mblk_row0+cursor_row;
	slen=edbf0[mrow].len;
	if (row>=Lines) {
		incr_line();
		edbf[Lines-1].cstr=0;
		edbf[Lines-1].len=0;
		if (cur_file==mark_holder) edbf0=edbf;
	}
	olen=edbf[row].len;
	nlen=Max(olen,cursor_col)+blen;
	str=edbf[row].cstr;
	if (olen < nlen) {
		if ((str=(char *)mrealloc(edbf[row].cstr,nlen,"copy_block"))
				==NULL) return;
		for(i=olen;i<nlen;str[i++]=' ');
		edbf[row].cstr=str;
		edbf[row].len=nlen;
	}
	ss=edbf0[mrow].cstr;
	if (cursor_col < olen) {
		mmemmove(str+cursor_col+blen,str+cursor_col, olen-cursor_col);
	}
	if (mblk_col0 >= slen) {
		memset(str+cursor_col,' ',blen);
	} else
	if (mblk_col1>=slen) {
		memset(str+cursor_col+slen-mblk_col0,' ',mblk_col1+1-slen);
	}
	if (mblk_col0 < slen) {
		if (cursor_col<=mblk_col0 && row>=mblk_row0 && row<=mblk_row1)
			memcpy(str+cursor_col,ss+mblk_col0+blen,
					Min(blen,slen-mblk_col0));
		else
			memcpy(str+cursor_col,ss+mblk_col0,
					Min(blen,slen-mblk_col0));

	}
}

if (cur_file==mark_holder) {
	if (cursor_row==mblk_row0 && cursor_col<=mblk_col0) {
		mblk_col0+=blen;
		mblk_col1+=blen;
	} else if (cursor_row>mblk_row0&&cursor_row<=mblk_row1)
		mblk_row0=mblk_row1=-1;
}

disp_page();
show_cursor_w();
}

K_CpArea()
{
s_fmod();
if (mline_begin>=0) {
	copy_mlines();
} else
if (mblk_row0>=0) {
	u_rep_lines(cursor_row,cursor_row+mblk_row1-mblk_row0,1);
	copy_block();
}
return 0;
}

K_OwCp()
{
int row,mrow,len,blen=mblk_col1-mblk_col0+1;
int olen,nlen,i, slen;
char *str,*ss;
int bln=mblk_row1-mblk_row0+1;
LIN *edbf0=edstate[mark_holder].edbf;

if (mblk_row0 < 0||cursor_row+bln>Lines) return 1;
u_rep_lines(cursor_row,cursor_row+mblk_row1-mblk_row0,1);
untab_lines(cursor_row,cursor_row+bln-1);
cursor_col=page_col+cur_x;
s_fmod();
for(mrow=mblk_row0,row=cursor_row;mrow<=mblk_row1;mrow++)
{
	row=mrow-mblk_row0+cursor_row;
	slen=edbf0[mrow].len;
	if (row>=Lines) {
		incr_line();
		edbf[Lines-1].cstr=0;
		edbf[Lines-1].len=0;
	}
	olen=edbf[row].len;
	nlen=cursor_col+blen;
	str=edbf[row].cstr;
	if (olen < nlen) {
		if ((str=(char *)mrealloc(edbf[row].cstr,nlen,"ovwrite_copy"))
				==NULL) return;
		for(i=olen;i<nlen;i++) str[i]=' ';
		edbf[row].cstr=str;
		edbf[row].len=nlen;
	}
	ss=edbf0[mrow].cstr;
	if (mblk_col0 >= slen) {
		memset(str+cursor_col,' ',blen);
	} else
	if (mblk_col1>=slen) {
		memset(str+cursor_col+slen-mblk_col0,' ',mblk_col1+1-slen);
	}
	if (mblk_col0 < slen)
		memcpy(str+cursor_col,ss+mblk_col0,Min(blen,slen-mblk_col0));
}
disp_page();
show_cursor_w();
return 0;
}

void adj_mark_delline(row)
{
if (mark_holder!=cur_file) return;
if (mline_begin>=0) {
	if (row==mline_end && row==mline_begin) {
		mline_begin=mline_end=-1;
		return;
	}
	if (row < mline_begin) mline_begin--;
	if (row <= mline_end) mline_end--;
} else
if (mblk_row0>=0) {
	if (row==mblk_row0 && row==mblk_row1) {
		mblk_row0=mblk_row1=mblk_col0=mblk_col1=-1;
		return;
	}
	if (row < mblk_row0) mblk_row0--;
	if (row <= mblk_row1) mblk_row1--;
}
}

void adj_mark_insline()
{
if (cur_file!=mark_holder) return;
if (mline_begin>=0) {
	if (cursor_row < mline_begin) mline_begin++;
	if (cursor_row < mline_end) mline_end++;
} else
if (mblk_row0>=0) {
	if (cursor_row==mblk_row0 && mblk_col1 > cursor_col) mblk_row0++;
	else
	if (cursor_row < mblk_row0) mblk_row0++;
	if (cursor_row==mblk_row1 && mblk_col1 > cursor_col) mblk_row1++;
	else
	if (cursor_row < mblk_row1) mblk_row1++;
}
}

void adj_mblk_col_del(int len)
{
if (cur_file!=mark_holder) return;
	if (cursor_row==mblk_row0 && cursor_row==mblk_row1 && cursor_col< mblk_col0)
	{ mblk_col0-=len; mblk_col1-=len; }
}

K_ExtMarkUp()
{
if (mblk_row0>=0) K_MarkBlk();
else
	K_MarkLn();
K_cur_up();
return 0;
}

K_ExtMarkDown()
{
if (mblk_row0>=0) K_MarkBlk();
else
	K_MarkLn();
K_cur_down();
return 0;
}

K_ExtMarkLeft()
{
if (mline_begin < 0) K_MarkBlk();
K_cur_left();
return 0;
}

K_ExtMarkRight()
{
if (mline_begin < 0) K_MarkBlk();
K_cur_right();
return 0;
}

K_MvArea()
{
int row,col;
int len=-1,wid=-1;
int col0,col1;
int row0,row1;
int less;

row0=mblk_row0;
row1=mblk_row1;
less=cursor_col < mblk_col0;

if (mline_begin>=0) {
	len=mline_end-mline_begin;
} else
if (mblk_row0>=0) {
	len=mblk_row1-mblk_row0;
	wid=mblk_col1-mblk_col0;
} else return 1;
K_CpArea();
K_DelArea();
if (wid>=0) {
	if (cursor_row!=row0 || less)
		mblk_col0=cursor_col;
	else mblk_col0=cursor_col-wid-1;
	mblk_col1=mblk_col0+wid;
	mblk_row0=cursor_row;
	mblk_row1=mblk_row0+len;
} else {
	mline_begin=cursor_row+1;
	mline_end=mline_begin+len;
}
pos_cur();
disp_page();
show_cursor_w();
return 0;
}

K_UpLowBlk(int up)
{
int row,col;
if (mblk_row0>=0) {
	u_rep_lines(mblk_row0,mblk_row1,1);
	s_fmod();
	for(row=mblk_row0;row<=mblk_row1;row++) {
		int len=edbf[row].len;
		for(col=mblk_col0;col<len&&col<=mblk_col1;col++)
			if (up)
				edbf[row].cstr[col]=mtoupper(edbf[row].cstr[col]);
			else
				edbf[row].cstr[col]=mtolower(edbf[row].cstr[col]);
	}
} else if (mline_begin>=0) {
	u_rep_lines(mline_begin,mline_end,1);
	s_fmod();
	for(row=mline_begin;row<=mline_end;row++) {
		int len=edbf[row].len;
		for(col=0;col<len;col++)
			if (up)
				edbf[row].cstr[col]=mtoupper(edbf[row].cstr[col]);
			else
				edbf[row].cstr[col]=mtolower(edbf[row].cstr[col]);
	}
} else return 1;
disp_page();
show_cursor_w();
return 0;
}

K_BShiftUpDn(int updn)
{
int row,blen,len,sign;
char *ss;

if (mblk_row0<0) return 1;
s_fmod();
u_rep_lines(mblk_row0,mblk_row1,1);
blen=mblk_col1-mblk_col0+1;
sign=updn?-1:1;
for(row=updn?mblk_row1:mblk_row0;(updn)?row>mblk_row0:row<mblk_row1;
row+=sign) {
	int len1=edbf[row+sign].len;
	char *ss1=edbf[row+sign].cstr;
	len=edbf[row].len;
	ss=edbf[row].cstr;
#define DEB 0
#if	DEB
	printf("%d:",row+1);
#endif
	if (mblk_col1<len && mblk_col1<len1) {
#if	DEB
		puts("aa");
#endif
		memcpy(&ss[mblk_col0],&ss1[mblk_col0],blen);
	} else
	if (len >= len1) {
	if (mblk_col1 < len &&  mblk_col0 < len1) {
#if	DEB
		puts("ee");
#endif
		memcpy(&ss[mblk_col0],&ss1[mblk_col0], len1-mblk_col0);
		memset(&ss[len1],' ',mblk_col1-len1+1);
	} else
	if (mblk_col0 >= len1 && mblk_col1 < len) {
#if	DEB
		puts("ff");
#endif
		memset(&ss[mblk_col0],' ',blen);
	} else
	if (mblk_col0 >=len1 && mblk_col1 >= len && mblk_col0 < len) {
#if	DEB
		puts("gg");
#endif
		edbf[row].len=mblk_col0; /* actully should realloc */
	} else
	if (mblk_col0 < len && mblk_col0 < len1 && mblk_col1>=len1) {
#if	DEB
		puts("00");
#endif
		memcpy(&ss[mblk_col0],&ss1[mblk_col0],len1-mblk_col0);
		edbf[row].len=len1;
	}
	} else {
	if (mblk_col1>=len && mblk_col1 < len1) {
#if	DEB
		puts("bb");
#endif
		if (!(ss=mrealloc(ss,mblk_col1+1,"K_BShiftUp"))) {
			return;
		}
		memcpy(&ss[mblk_col0],&ss1[mblk_col0], blen);
		if (mblk_col0 > len) memset(&ss[len],' ',mblk_col0-len);
		edbf[row].cstr=ss;
		edbf[row].len=mblk_col1+1;
	} else
	if (mblk_col0<len1 && mblk_col1 >= len1) {
#if	DEB
		puts("cc");
#endif
		if (!(ss=mrealloc(ss,len1,"K_BShiftUp"))) {
			return;
		}
		memcpy(&ss[mblk_col0],&ss1[mblk_col0], len1-mblk_col0);
		if (mblk_col0 > len) memset(&ss[len],' ',mblk_col0-len);
		edbf[row].cstr=ss;
		edbf[row].len=len1;
	} else {
#if	DEB
		puts("---");
#endif
	}
	}
}
len=edbf[updn?mblk_row0:mblk_row1].len;
ss=edbf[updn?mblk_row0:mblk_row1].cstr;
if (mblk_col1 < len-1) {
	memset(ss+mblk_col0,' ',blen);
} else
if (edbf[updn?mblk_row0:mblk_row1].cstr)
	edbf[updn?mblk_row0:mblk_row1].len=mblk_col0;
hide_cursor_w();
disp_page();
if (cursor_col > edbf[cursor_row].len) cursor_col=edbf[cursor_row].len;
pos_cur();
show_cursor_w();
return 0;
}

K_BShiftLR(int lr)
{
int row;
int nolim=0;
int tcol0=mblk_col0,tcol1=mblk_col1;
int trow0=mblk_row0,trow1=mblk_row1;

if (mblk_row0<0 && mline_begin<0) return 1;
if (mline_begin>=0) {
	tcol0=0;
	tcol1=0x7fffffff;
	trow0=mline_begin;
	trow1=mline_end;
	u_rep_lines(trow0,trow1,1);
	untab_lines(mline_begin,mline_end);
} else
	u_rep_lines(trow0,trow1,1);
s_fmod();
for(row=trow0;row<=trow1;row++) {
	int len=edbf[row].len;
	char *ss=edbf[row].cstr;
	int ll;
	if (len <= tcol0) continue;
	if (lr) {
	        if (tcol1 >= len) {
	                if (!(ss=mrealloc(ss,len+1,"K_BShiftLR"))) return;
	                edbf[row].len++;
	                edbf[row].cstr=ss;
	                ll=len-tcol0;
	        } else ll=tcol1-tcol0;
		mmemmove(&ss[tcol0+1],&ss[tcol0],ll);
	        ss[tcol0]=' ';
	} else {
		if (len-1 <= tcol1) {
			len--;
			if (!len) {
				free(ss);
				edbf[row].cstr=0;
				edbf[row].len=0;
				continue;
			}
			edbf[row].len=len;
			mmemmove(&ss[tcol0],&ss[tcol0+1],len-tcol0);
		} else {
			mmemmove(&ss[tcol0],&ss[tcol0+1],
				tcol1-tcol0);
			ss[tcol1]=' ';
		}
	}
}
hide_cursor_w();
disp_page();
if (cursor_col > edbf[cursor_row].len) cursor_col=edbf[cursor_row].len;
pos_cur();
show_cursor_w();
return 0;
}

void K_Dup1stToEndBlk()
{
char *ss,*tt;
int mstr=0,len,row,ll;
if (mblk_row0<0||mblk_row0==mblk_row1) return;
u_rep_lines(mblk_row0+1,mblk_row1,1);
len=mblk_col1-mblk_col0+1;
ll=edbf[mblk_row0].len;
if (mblk_col1>=ll) {
	if ((ss=mmalloc(len,"K_Dup1stToEndBlk"))==NULL)
		return;
	mstr=1;
	memcpy(ss,edbf[mblk_row0].cstr+mblk_col0,ll-mblk_col0);
	memset(ss+len,' ',mblk_col1+1-len);
} ss=edbf[mblk_row0].cstr+mblk_col0;
for(row=mblk_row0+1;row<=mblk_row1;row++) {
	tt=edbf[row].cstr;
	ll=edbf[row].len;
	if (mblk_col1>=ll) {
		if ((tt=mrealloc(tt,mblk_col1+1,"Dup1stToEndBlk2"))==NULL)
			goto llll1;
		edbf[row].cstr=tt;
		edbf[row].len=mblk_col1+1;
		if (mblk_col0>=ll)
			memset(tt+ll,' ',mblk_col0-ll);
	}
	memcpy(tt+mblk_col0,ss,len);
}
disp_page();
show_cursor_w();
llll1:
if (mstr) free(ss);
}

static char *paste_buffer;
static int pbuf_size,pbuf_asize;
static int paste_buf_type;


void K_ToPasteBuff()
{
int row;

pbuf_size=0;
if(mline_begin<0 && mblk_row0<0) return;
if (pbuf_asize > 4096) {
	free(paste_buffer);
	paste_buffer=NULL;
}
if (mline_begin>=0) {
	for(row=mline_begin;row<=mline_end;row++) {
		char *s=edbf[row].cstr;
		int len=edbf[row].len;
		if (len+pbuf_size+2>=pbuf_asize) {  /* one for '\n', one for '\0' */
			pbuf_asize=len+pbuf_size+len+256;
			if ((paste_buffer=mrealloc(paste_buffer,pbuf_asize,"K_ToPasteBuff"))
				==NULL) return;
		}
		memcpy(paste_buffer+pbuf_size,s,len);
		pbuf_size+=len;
		paste_buffer[pbuf_size++]='\n';
	}
	paste_buffer[pbuf_size++]=0;
}
else {
	for(row=mblk_row0;row<=mblk_row1;row++) {
		char *s=edbf[row].cstr;
		int len=edbf[row].len;
		int e=Min(mblk_col1,len-1);
		if (len && mblk_col0<len) {
			int sl=e-mblk_col0+1;

			if (sl+pbuf_size+2>=pbuf_asize) {  /* one for '\n', one for '\0' */
				pbuf_asize=sl+pbuf_size+len+256;
				if ((paste_buffer=mrealloc(paste_buffer,pbuf_asize,"K_ToPasteBuff"))
					==NULL) return;
			}
			memcpy(paste_buffer+pbuf_size,s+mblk_col0,sl);
			pbuf_size+=sl;
			paste_buffer[pbuf_size++]='\n';
		} else {
			paste_buffer[pbuf_size++]='\n';
		}
	}
	paste_buffer[pbuf_size-1]=0;
}
}

void K_PasteBuffPut()
{
if (!paste_buffer || !pbuf_size) return;
K_PutStr(paste_buffer);
}

K_PasteBuffPutAfter()
{
int s_cur=cursor_row;
int s_col=cursor_col;
extern int no_save;
if (!paste_buffer || !pbuf_size) return;
if (cursor_row<Lines-1) {
	K_cur_down();
	K_home();
}
else {
	K_end();
	K_return();
}
no_save=1;
K_PutStr(paste_buffer);
hide_cursor_w();
cursor_row=s_cur;
cursor_col=s_col;
pos_cur();
adj_page_col();
show_cursor_w();
}

K_PasteBuffer()
{
if (paste_buf_type) K_PasteBuffPutAfter();
else
	K_PasteBuffPut();
}

K_SaveNLtoPaste(int n)
{
int row;
pbuf_size=0;
for(row=cursor_row;row<cursor_row+n;row++) {
	char *s=edbf[row].cstr;
	int len=edbf[row].len;
	if (len+pbuf_size+2>=pbuf_asize) {  /* one for '\n', one for '\0' */
		pbuf_asize=len+pbuf_size+len+256;
		if ((paste_buffer=mrealloc(paste_buffer,pbuf_asize,"K_ToPasteBuff"))
			==NULL) return;
	}
	memcpy(paste_buffer+pbuf_size,s,len);
	pbuf_size+=len;
	paste_buffer[pbuf_size++]='\n';
}
paste_buffer[pbuf_size++]=0;
paste_buf_type=1;
}


K_SaveEoltoPaste()
{
char *s=edbf[cursor_row].cstr+cursor_col;
int len=edbf[cursor_row].len-cursor_col;
pbuf_size=0;
if (len<=0) return;
if (len+pbuf_size+2>=pbuf_asize) {  /* one for '\n', one for '\0' */
	pbuf_asize=len+pbuf_size+len+256;
	if ((paste_buffer=mrealloc(paste_buffer,pbuf_asize,"K_ToPasteBuff"))
		==NULL) return;
}
memcpy(paste_buffer+pbuf_size,s,len);
pbuf_size+=len;
paste_buffer[pbuf_size++]=0;
paste_buf_type=0;
}

K_SaveWordtoPaste()
{
char *s=edbf[cursor_row].cstr;
int i,len=edbf[cursor_row].len;
pbuf_size=0;
if (len<=0) return;
i=cursor_col;
while (i<len && !in_sep_ch(s[i])) i++;
len=i-cursor_col;
if (len<=0) return;
if (len+pbuf_size+2>=pbuf_asize) {  /* one for '\n', one for '\0' */
	pbuf_asize=len+pbuf_size+len+256;
	if ((paste_buffer=mrealloc(paste_buffer,pbuf_asize,"K_ToPasteBuff"))
		==NULL) return;
}
memcpy(paste_buffer+pbuf_size,s+cursor_col,len);
pbuf_size+=len;
paste_buffer[pbuf_size++]=0;
paste_buf_type=0;
}
