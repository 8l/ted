/*
	Copyright (C)   1995
	Edward  Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "ted.h"


static char *boxtitle=0;
static int bx,by,bwidth,blen,bframe,bback;

void box(int x,int y,int width,int len,int frame,int back,char *msg)
{
int i,j;
int ex=x+width-1;
int ey=y+len-1;
set_att(back);
for(j=y+1;j<ey;j++) {
	gotoxy(x+1,j);
	for(i=x+1;i<ex;i++) xprintf(" ");
}
set_att(frame);
for(i=x;i<=ex;i++) {
	gotoxy(i,y); xprintf(" ");
	gotoxy(i,ey); xprintf(" ");
}
for(j=y+1;j<ey;j++) {
	gotoxy(x,j); xprintf(" ");
	gotoxy(ex,j); xprintf(" ");
}
i=x+(width-strlen(msg))/2;
gotoxy(i,y);
xprintf(msg);
bx=x; by=y; bwidth=width; blen=len; bframe=frame; bback=back;
boxtitle=msg;
}

void refresh_box()
{
if (boxtitle) {
box(bx,by,bwidth,blen,bframe,bback,boxtitle);
}
}

void close_box()
{
boxtitle=0;
}

static XYstr *lxystr;
static int xystrn=0;
void xystr(XYstr *xx,int nn)
{
int i;
set_att(8);
for(i=0;i<nn;i++) {
	gotoxy(xx[i].x,xx[i].y);
	xprintf("%s",xx[i].str);
}
xystrn=nn;
lxystr=xx;
}

void refresh_xystr()
{
if (xystrn) xystr(lxystr,xystrn);
}

void  clr_xystr()
{ xystrn=0;
}

void goto_line(int row)
{
	if (row >= Lines || row<0) {
		disp_page();
		message(4,"Only %d lines",Lines);
		return;
	}
	hide_cursor_w();
	cursor_col=0;
	cursor_row=row;
	page_row=cursor_row-(page_len/2);
	if (page_row<0)page_row=0;
	pos_cur();
	disp_page();
	disp_cur_pos();
	show_cursor_w();
}

void Goto_line()
{
	extern char * return_item();
	char *s;
	int row=atoi(return_item(0))-1;
	goto_line(row);
	save_win_context(active_win);
}

static int g_xget_handle;

void goto_line_init()
{
g_xget_handle=xgets_handle();
add_edln(g_xget_handle,0,6,20,10,20);
add_action(g_xget_handle,Goto_line);
}

K_GotoRow()
{
input_handler=1;
save_win_context(active_win);
inactive_cursor();
box(18,8,24,5,7,8,"Goto line");
init_xgets(g_xget_handle);
return 1;
}

TAG tag[MAXTAG];

void init_tag()
{
int i;
for(i=0;i<MAXTAG;tag[i++].fno=-1);
}

K_SetTag(int key)
{
if (key < 0||key>=MAXTAG) return 1;
tag[key].page_row=page_row;
tag[key].page_col=page_col;
tag[key].cursor_row=cursor_row;
tag[key].cursor_col=cursor_col;
tag[key].fno=cur_file;
return 0;
}

K_GotoTag(int key)
{
if (key<0||key>=MAXTAG) return 1;
if (tag[key].fno<0) return 1;
hide_cursor_w();
if (tag[key].fno!=cur_file) {
	goto_file(tag[key].fno);
}
if (tag[key].page_row>=Lines || tag[key].cursor_row>=Lines) return;
page_row=tag[key].page_row;
page_col=tag[key].page_col;
cursor_row=tag[key].cursor_row;
cursor_col=tag[key].cursor_col;
disp_page();
disp_cur_pos();
pos_cur();
show_cursor_w();
return 0;
}

void adj_tag_fno(int cfno)
{
int i;
for(i=0;i<MAXTAG;i++)
	if (tag[i].fno==cfno) tag[i].fno=-1;
	else
	if (tag[i].fno>cfno) tag[i].fno--;
}

void leading_space_tab()
{
int row,len,nlen,i,ll,tabno,ofs;
char *ss;
for(row=0;row<Lines;row++) {
	if (!(len=edbf[row].len)) continue;
	ss=edbf[row].cstr;
	for(ll=0;ll<len && ss[ll]==' ';ll++);
	if (ll<tab_width) continue;
	tabno=ll/tab_width;
	ofs=tabno+(ll%tab_width);
	mmemmove(ss+ofs,ss+ll,len-ll);
	for(i=0;i<tabno;ss[i++]=9);
	if ((ss=mrealloc(ss,nlen=len-ll+ofs,"leading_space_tab"))==NULL)
		return;
	edbf[row].cstr=ss;
	edbf[row].len=nlen;
}
}

void trim_blank_tail()
{
int row,len,i,ll;
char *ss;
for(row=0;row<Lines;row++) {
	if (!(len=edbf[row].len)) continue;
	ss=edbf[row].cstr;
	for(ll=len;ll>0 && (ss[ll-1]==' '||ss[ll-1]==9) ;ll--);
	if (!ll) {
		free(ss);
		ss=0;
		goto lll1;
	} else
	if (ll==len) continue;
	if ((ss=mrealloc(ss,ll,"trim_blank_tail"))==NULL)
		return;
lll1:
	edbf[row].cstr=ss;
	edbf[row].len=ll;
}
if (cursor_col>edbf[cursor_row].len) cursor_col=edbf[cursor_row].len;
}

K_ExecMarkWord(char *arg1,char *arg2)
{
char ss[128],tt[256];
int len=Min(mblk_col1,edbf[mblk_row0].len)-mblk_col0+1;
if (mblk_row0<0) return;
memcpy(ss,&edbf[mblk_row0].cstr[mblk_col0],len);
ss[len]=0;
if (arg2)
	sprintf(tt,"%s %s %s",arg1,ss,arg2);
else
	sprintf(tt,"%s %s",arg1,ss);
K_ReadPipe(tt);
return 0;
}

K_MarkWord(char *sep_str)
{
char *ss,*tt,*uu,*vv;
int mlen,len;

chg_mark_owner();
ss=edbf[cursor_row].cstr;
len=edbf[cursor_row].len;
if (!len) return;
vv=ss+len;
tt=uu=ss+cursor_col;
if (strchr(sep_str,*tt))
	while (tt>ss && strchr(sep_str,*(tt-1)))  tt--;
else 	while (tt>ss && !strchr(sep_str,*(tt-1))) tt--;

if (strchr(sep_str,*uu))
	while (uu<vv && strchr(sep_str,*uu)) uu++;
else 	while (uu<vv && !strchr(sep_str,*uu)) uu++;
mlen=uu-tt;
mblk_row0=mblk_row1=cursor_row;
mblk_col0=tt-ss;
mblk_col1=uu-ss-1;
if (mblk_col1<mblk_col0) mblk_col1=mblk_col0;
disp_page();
show_cursor_w();
}

extern char sep_chars[];

void mark_word_xy(int x, int y)
{
char *ss,*tt,*uu,*vv;
int mlen,len;
int row;
int col;

x-=win_x;
y-=win_y;
if (x<0 || y<0 || x>=page_width || y>=page_len)
	return;

row=page_row+y;
if (row>=Lines) return;
col=page_col+x;

chg_mark_owner();
untab_lines(row,row);
ss=edbf[row].cstr;
len=edbf[row].len;
if (!ss || col>=len) return;
vv=ss+len;
tt=uu=ss+col;
if (in_sep_ch(*tt))
	while (tt>ss && in_sep_ch(*(tt-1)))  tt--;
else 	while (tt>ss && !in_sep_ch(*(tt-1))) tt--;

if (in_sep_ch(*uu))
	while (uu<vv && in_sep_ch(*uu)) uu++;
else 	while (uu<vv && !in_sep_ch(*uu)) uu++;
mlen=uu-tt;
mblk_row0=mblk_row1=row;
mblk_col0=tt-ss;
mblk_col1=uu-ss-1;
if (mblk_col1<mblk_col0) mblk_col1=mblk_col0;
disp_page();
show_cursor_w();
}


GetMarkWord(char *sss)
{
int len=edbf[mblk_row0].len,mlen;
char *ss;
struct stat fst;
if (mblk_row0<0) return 0;
if (mblk_col0 > len) return 0;
if (mblk_col1 >= len) mblk_col1=len-1;
ss=edbf[mblk_row0].cstr;
while (ss[mblk_col0]==' '&&mblk_col0<len) mblk_col0++;
mlen=mblk_col1-mblk_col0+1;
if (mlen<=0) return 1;
memcpy(sss,ss+mblk_col0,mlen);
sss[mlen]=0;
while (mlen && sss[mlen-1]==' ') {
	sss[mlen-1]=0;
	mlen--;
}
return 1;
}


K_EditMarkWord()
{
char tfname[128];
struct stat fst;

if (!GetMarkWord(tfname)) return;
save_edstate();
if (stat(tfname,&fst)<0) return;
if (S_ISDIR(fst.st_mode)) {
	K_CD(tfname);
}
else	{
#if	0
	strcpy(fname,tfname);
#endif
	edit_file(tfname);
}
return 0;
}

K_EditFName(char *s)
{
save_edstate();
#if	0
strcpy(fname,s);
#endif
edit_file(s);
return 0;
}

K_system(char *s)
{
system(s);
return 1;
}

K_SystemFname(char *s)
{
char tt[128];
strcat(strcat(strcpy(tt,s)," "),fname);
system(tt);
return 1;
}

mtolower(int k)
{ return (k>='A'&&k<='Z'?k+0x20:k) ;
}

mtoupper(int k)
{ return (k>='a'&&k<='z'?k-0x20:k) ;
}

K_GotoGrep(int replace)
{
char *ss=edbf[cursor_row].cstr;
int len=edbf[cursor_row].len;
char *p,*q,*fn,*t;
int row,i;
char s[2048];

if (!ss||!len) return;
memcpy(s,ss,len);
if (!(p=strchr(s,':'))) return;
t=p-1;
while (*t==' ') t--;
t++;
*t=0;
p++;
fn=s;
if (!(q=strchr(p,':'))) return;
*q=0;
while(*p==' ') p++;
#if	0
printf("--%s---%s---\n",fn,p);
#endif
row=atoi(p);
row--;

for(i=0;i<file_no;i++) {
	if (!strcmp(edstate[i].fname,fn)) break;
}
if (i<file_no) {
	goto_file(i);
	goto_line(row);
	return;
}
if (replace) {
	nosave=1;
	close_file_noexit();
}
save_edstate();
#if	0
strcpy(fname,fn);
#endif
edit_file(fn);
goto_line(row);
}

K_Tab2Space()
{
int row,i,nlen;

for(row=0;row<Lines;row++) {
	char *t,*s=edbf[row].cstr;
	int len=edbf[row].len;
	if (!s) continue;
	for(nlen=i=0;i<len;i++)
		if (s[i]=='\t') nlen+=tab_width-(nlen%tab_width);
		else
			nlen++;
	if ((t=mmalloc(nlen,"K_Tab2Space"))==NULL) return;
	for(nlen=i=0;i<len;i++)
		if (s[i]=='\t') {
		  int tl=tab_width-(nlen%tab_width);
			memset(t+nlen,' ',tl);
			nlen+=tl;
		} else
			t[nlen++]=s[i];
	edbf[row].cstr=t;
	edbf[row].len=nlen;
	free(s);
}
message(4,"Tab to Space");
f_modified=1;
}

K_SumRectBlock()
{
  int i,j;
  double sd=0.0;
  char uu[256];
  int hasdot=0;

  if (mblk_row0<0)
    return;

  for(i=mblk_row0;i<=mblk_row1;i++) {
    char *cstr=edbf[i].cstr;
    int len=edbf[i].len;
    char t[256],*u;
    int l=Min(len-1,mblk_col1)-mblk_col0+1;

    u=t;
    memcpy(t,&cstr[mblk_col0],l);
    while (*u==' '||*u==9)
      u++;
    while (l && t[l-1]==' ')
      l--;
    t[l]=0;
    if (strchr(t,'.'))
      hasdot=1;
    sd+=atof(t);
  }
  if (hasdot)
    sprintf(uu,"%.2lf",sd);
  else
    sprintf(uu,"%.0lf",sd);
  K_PutStr(uu);
}
