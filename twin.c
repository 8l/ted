/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include "ted.h"
#include <sys/types.h>

EDWIN edwin[4];
int active_win;
int win_split=0;
int win_no=1;

#include <time.h>
void show_time()
{
time_t cur_time=time(NULL);
char *timestr=ctime(&cur_time);
timestr[16]=0;
xputs(XW_COL-13,edwin[0].MROW-2+edwin[0].y,timestr+4,2);
}

void load_win_context(int win)
{
	cur_file=edwin[win].cur_file;
	win_x=edwin[win].x;
	win_y=edwin[win].y;
	MROW=edwin[win].MROW;
	MCOL=edwin[win].MCOL;
	page_len=MROW-2;
	page_width=MCOL;
	cursor_row=edwin[win].pg_pos[cur_file].cursor_row;
	cursor_col=edwin[win].pg_pos[cur_file].cursor_col;
	cur_x=edwin[win].pg_pos[cur_file].cur_x;
	cur_y=edwin[win].pg_pos[cur_file].cur_y;
	page_row=edwin[win].pg_pos[cur_file].page_row;
	page_col=edwin[win].pg_pos[cur_file].page_col;
	insert_mode=edwin[win].insert_mode;
	Indent=edwin[win].Indent;
	mesg=edwin[win].mesg;

	f_modified=edstate[cur_file].f_modified;
	nosave=edstate[cur_file].nosave;
	edbf=edstate[cur_file].edbf;
	Lines=edstate[cur_file].Lines;
	Alloc_Lines=edstate[cur_file].Alloc_Lines;
	strcpy(fname,edstate[cur_file].fname);
	set_lang_type(fname);
	K_SwitchKeyDef(edstate[cur_file].cmapno);
}

void init_edwin_state()
{
int i;
for(i=0;i<4;i++) {
	edwin[i].Indent=Indent;
	edwin[i].insert_mode=insert_mode;
	edwin[i].MROW=MROW;
	edwin[i].MCOL=MCOL;
}
}

void save_win_context(int win)
{
	edwin[win].cur_file=cur_file;
	edwin[win].x=win_x;
	edwin[win].y=win_y;
	edwin[win].MROW=MROW;
	edwin[win].MCOL=MCOL;
	edwin[win].pg_pos[cur_file].cursor_row=cursor_row;
	edwin[win].pg_pos[cur_file].cursor_col=cursor_col;
	edwin[win].pg_pos[cur_file].cur_x=cur_x;
	edwin[win].pg_pos[cur_file].cur_y=cur_y;
	edwin[win].pg_pos[cur_file].page_row=page_row;
	edwin[win].pg_pos[cur_file].page_col=page_col;
	edwin[win].insert_mode=insert_mode;
	edwin[win].Indent=Indent;
	edwin[win].mesg=mesg;

	edstate[cur_file].f_modified=f_modified;
	edstate[cur_file].nosave=nosave;
	edstate[cur_file].edbf=edbf;
	edstate[cur_file].Lines=Lines;
	edstate[cur_file].Alloc_Lines=Alloc_Lines;
	strcpy(edstate[cur_file].fname,fname);
	edstate[cur_file].cmapno=cmapno;
}

void draw_seperate()
{
int x,y;
set_att(5);
if (win_split!=4) {
	if (win_split&1)
		for(y=0;y<XW_ROW;y++) {
			gotoxy(edwin[0].MCOL,y);
			xputstrd("|");
		}
	gotoxy(0,edwin[0].MROW);
	if (win_split&2)
		for(x=0;x<XW_COL;x++) xputstrd("=");
} else {
	gotoxy(0,cwin_len);
	for(x=0;x<XW_COL;x++) xputstrd("=");
}
set_att(0);
draw_scr();
}

void redraw_twin()
{
int i,aw=active_win;

if (batch) return;
for(i=0;i<win_no;i++) {
	load_win_context(active_win=i);
	pos_cur();
	adj_page_col();
	redraw_status();
	disp_page();
}
load_win_context(active_win=aw);
if (cursor_row-page_row >= page_len) {
	page_row=cursor_row-(page_len>>1);
}
pos_cur();
adj_page_col();
disp_page();
if (win_split==4) disp_cwin();
redraw_status();
draw_seperate();
show_cursor_w();
refresh_box();
refresh_xystr();
}

void resize_twin()
{
int x,y,i;

clrscr_all();
switch (win_split) {
	case 0:
		win_no=1;
		edwin[0].y=0;
		edwin[0].MROW=XW_ROW;
		edwin[0].MCOL=XW_COL;
		break;
	case 1:
		win_no=2;
		edwin[0].y=0;
		edwin[0].MCOL=edwin[1].MCOL=(XW_COL-1)/2;
		edwin[0].MROW=edwin[1].MROW=XW_ROW;
		edwin[1].x=(XW_COL-1)/2 + 1;
		edwin[1].y=0;
		break;
	case 2:
		win_no=2;
		edwin[0].y=0;
		edwin[0].MCOL=edwin[1].MCOL=XW_COL;
		edwin[0].MROW=edwin[1].MROW=(XW_ROW-1)/2;
		edwin[1].x=0;
		edwin[1].y=(XW_ROW-1)/2 + 1;
		break;
	case 3:
		edwin[0].y=0;
		edwin[1].x=(XW_COL-1)/2 + 1;
		edwin[1].y=0;
		edwin[2].x=0;
		edwin[2].y=(XW_ROW-1)/2 + 1;
		edwin[3].x=(XW_COL-1)/2 + 1;
		edwin[3].y=(XW_ROW-1)/2 + 1;
		edwin[0].MROW=edwin[1].MROW=edwin[2].MROW=edwin[3].MROW=
							(XW_ROW-1)/2;
		edwin[0].MCOL=edwin[1].MCOL=edwin[2].MCOL=edwin[3].MCOL=
							(XW_COL-1)/2;
		win_no=4;
		break;
	case 4:
		edwin[0].y=cwin_len+1;
		edwin[0].MROW=XW_ROW-cwin_len-1;
		edwin[0].MCOL=XW_COL;
		win_no=1;
}
for(i=0;i<4;i++) {
	if (edwin[i].cur_file >= file_no) edwin[i].cur_file=file_no-1;
	if (edwin[i].insert_mode>1) edwin[i].insert_mode=1;
}
if (win_split==4) disp_cwin();
redraw_twin();
}

int win_split_idx;

void win_mode(int mode)
{
if (mode==0) win_split_idx=0;
win_split=mode;
active_win=0;
resize_twin();
active_win=0;
}

int split_seq[]={0,1,2,3};
int split_r_n=4;
int WinSplitSeq=12;

void split_decode()
{
int k=WinSplitSeq;
if (!k) return;
WinSplitSeq=0;
if (k<10) {
	split_r_n=2;
	split_seq[1]=k;
} else
if (k<100) {
	split_r_n=3;
	split_seq[2]=k%10;
	split_seq[1]=k/10;
} else {
	if (k>=400) p_err("bad window split seq number : %d",k);
	split_r_n=4;
	split_seq[3]=k%10;
	k/=10;
	split_seq[2]=k%10;
	split_seq[1]=k/10;
}
}

K_SplitWin()
{
split_decode();
win_split_idx=(win_split_idx+1)%split_r_n;
save_win_context(active_win);
win_mode(split_seq[win_split_idx]);
}

K_SwWin()
{
	save_win_context(active_win);
	active_win=(active_win+1)%win_no;
	redraw_twin();
}

void clrscr_w()
{
	clear_rect(win_x,win_y,page_width,page_len);
}

int hot_file=-1, org_file;
K_SetHotFile()
{
hot_file=cur_file;
return 0;
}

K_TgHotFile()
{
if (hot_file < 0) return;
if (cur_file!=hot_file) {
	org_file=cur_file;
	cur_file=hot_file;
} else
	cur_file=org_file;
if (cur_file >= file_no) cur_file=file_no-1;
restore_edstate();
save_win_context(active_win);
redraw_twin();
show_cursor_w();
return 0;
}

goto_twin(int x, int y)
{
int xx,yy,i;
for(i=0;i<win_no;i++) {
	xx=x-edwin[i].x;
	yy=y-edwin[i].y;
	if (xx>=0 && xx<edwin[i].MCOL &&
		yy>=0 && yy<edwin[i].MROW-2) break;
}
if (i==active_win) return 0;
if (i==win_no) return 1;
save_win_context(active_win);
active_win=i;
redraw_twin();
return 1;
}

goto_twin_mv_cur(int x, int y)
{
int xx,yy,i;
for(i=0;i<win_no;i++) {
	xx=x-edwin[i].x;
	yy=y-edwin[i].y;
	if (xx>=0 && xx<edwin[i].MCOL &&
		yy>=0 && yy<edwin[i].MROW-2) break;
}
if (i==win_no) return 1;
if (yy+page_row>=Lines)	return 1;

if (i==active_win) {
	hide_cursor_w();
	cursor_row=yy+page_row;
	cur_x=xx;
	cur_y=yy;
	x2col(cur_x);
	disp_cur_pos();
	show_cursor_w();
	return 0;
}
save_win_context(active_win);
active_win=i;
redraw_twin();
return 1;
}

int out_active_win_y(int y)
{
extern int fheight;

if (y<0) return -1;
y/=fheight;
if (y>edwin[active_win].y+edwin[active_win].MROW-2) return 1;
else
if (y<edwin[active_win].y) return -1;
return 0;
}
