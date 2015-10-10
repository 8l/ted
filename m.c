/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include <sys/types.h>
#include "ted.h"
#include <memory.h>
#ifndef	NULL
#include <stdio.h>
#endif

void *mrealloc(void *s,int len,char *err)
{
char *ttt;
if (!s) {
	if ((ttt=(char *)malloc(len))==NULL)
		error(err);
}
else
if ((ttt=(char *)realloc(s,len))==NULL)
	error(err);
return ttt;
}

void *mmalloc(int len, char *err)
{
char *ttt;
if ((ttt=(char *)malloc(len))==NULL)
	error(err);
return ttt;
}

void mmemmove(char *t, char *s, int len)
{
int i;
	if (s > t) {
		for(i=0;i<len;i++)
			*(t+i)=*(s+i);
	}
	else {
		for(i=len-1;i>=0;i--)
			*(t+i)=*(s+i);
	}
}

void pos_cur()
{
int xx;
int len=edbf[cursor_row].len;
if (cursor_col>len) cursor_col=len;
if ((xx=col2x(cursor_col))!=cur_x) cur_x=xx;
cur_y=cursor_row-page_row;
if (cur_y >= page_len)
	page_row+=cur_y-page_len+1;
else
if (cur_y < 0) page_row+=cur_y-1;
if (page_row < 0) page_row=0;
else
if (page_row >= Lines) page_row=Lines-1;
cur_y=cursor_row-page_row;
}

K_return()
{
int nlen,len;
char *str;

s_fmod();
u_join_line(1);
hide_cursor_w();
len=edbf[cursor_row].len;
mmemmove((char *)&edbf[cursor_row+1],(char *)&edbf[cursor_row],
	 (Lines-cursor_row)*sizeof(LIN));
if (!cursor_col) {
	edbf[cursor_row].cstr=0;
	edbf[cursor_row].len=0;
} else
if (cursor_col==len) {
	edbf[cursor_row+1].cstr=0;
	edbf[cursor_row+1].len=0;
}
else {
	nlen=len-cursor_col;
	if ((str=mmalloc(nlen,"K_return: malloc"))==NULL)
		return;
	memcpy(str,&edbf[cursor_row].cstr[cursor_col],nlen);
	edbf[cursor_row+1].cstr=str;
	edbf[cursor_row+1].len=nlen;
	if (!(str=(char *)realloc(edbf[cursor_row].cstr,cursor_col))) {
		error("K_return: realloc");
		return;
	}
	edbf[cursor_row].cstr=str;
	edbf[cursor_row].len=cursor_col;
}

adj_mark_insline();
cursor_row++;
if (cur_y<page_len-1) {
	cur_y++;
} else {
	page_row++;
}
cur_x=cursor_col=page_col=0;
incr_line();
disp_page();
show_cursor_w();
return 0;
}


int Indent=1;
K_ReturnIndent()
{
int len0,i;
char *tt,*str0;
char buf[1024];

if (!insert_mode) {
	K_cur_down();
	K_home();
	return 0;
}
if (!Indent) {
lll1:
	K_return();
	return 0;
}
str0=edbf[cursor_row].cstr;
len0=edbf[cursor_row].len;
if (!str0 || !len0) goto lll1;
for(i=0;i<len0 && (str0[i]==9||str0[i]==' ');i++);
if (!i || cursor_col<=i) goto lll1;
tt=buf;
if (i>=sizeof(buf)-2) {
	if ((tt=mmalloc(i+1,"K_return_Indent"))==NULL) return 0;
}
memcpy(tt+1,str0,i);
tt[0]='\n';
tt[i+1]=0;
K_PutStr(tt);
if (i>=sizeof(buf)-2)
  free(tt);
return 0;
}

K_TogIndent()
{
Indent^=1;
return 0;
}

void del_mline(int lno)
{
int i;
	for(i=lno;i<Lines-1;i++)
	edbf[i]=edbf[i+1];
	edbf[Lines-1].cstr=0;
	edbf[Lines-1].len=0;
}

void join_line()
{
char *str=edbf[cursor_row].cstr;
char *str1;
int len=edbf[cursor_row].len;
int len1=edbf[cursor_row+1].len;
int lenn;

	if (cursor_row==Lines-1) return;
	lenn=len+len1;
	if (str && len1) {
		if ((str=mrealloc(str,lenn,"join_line"))==NULL)
			return;
	}
	if (!str) {
		adj_mark_delline(cursor_row+1);
		del_mline(cursor_row);
	} else {
		if (lenn && len1) {
			str1=edbf[cursor_row+1].cstr;
			memcpy(str+len,str1,len1);
			free(str1);
			edbf[cursor_row].cstr=str;
			edbf[cursor_row].len=lenn;
		}
		if (Lines-cursor_row-2 > 0) {
			adj_mark_delline(cursor_row+1);
			del_mline(cursor_row+1);
		}
	}
	decr_line();
	edbf[Lines].len=0;
	edbf[Lines].cstr=0;
	pos_cur();
	disp_page();
}

/* Zsolt Koppany */
static int in_selection()
{
	if (cur_file!=mark_holder)
		return 0;

	if (mline_begin >= 0) {
		if (cursor_row >= mline_begin && cursor_row <= mline_end)
			return 1;
	}
	else if (mblk_row0 >= 0) {
		if (cursor_row >= mblk_row0 && cursor_row <= mblk_row1 &&
			cursor_col >= mblk_col0 && cursor_col <= mblk_col1)
			return 1;
	}
	return 0;
}

int AutoBlockRepDel=1;

K_del()
{
char *str=edbf[cursor_row].cstr;
int len=edbf[cursor_row].len;
int i,dellen,need_scancmt;
extern int c_comment;

if (AutoBlockRepDel && in_selection()) {
	K_DelArea();
	return;
}

s_fmod();
if (cursor_col>=len) {
	if (cursor_row>=Lines-1) return 1;
	u_split_line(len,1);
	join_line();
} else {
#if     CHINESE
	if (B1st(cur_y,cur_x) && B2nd(cur_y,cur_x+1))
		dellen=2;
	else
#endif
		dellen=1;
	need_scancmt=0;
	if (c_comment && str[cursor_col]=='/' || str[cursor_col]=='*') {
		need_scancmt=1;
	}
	u_ins_str(dellen,1);
	adj_mblk_col_del(dellen);
	for(i=cursor_col;i<len-dellen;i++)
		str[i]=str[i+dellen];
	len-=dellen;
	if (len>0) {
		if ((str=mrealloc(str,len,"key_delete"))==NULL)
			return;
	} else {
		free(str);
		str=0;
		len=0;
	}
	edbf[cursor_row].cstr=str;
	edbf[cursor_row].len=len;
	pos_cur();
	if (need_scancmt) {
		scan_comment(cursor_row-page_len,cursor_row+page_len);
		disp_page();
	} else
	disp_cursor_line();
}
show_cursor_w();
return 0;
}

void K_delvi()
{
int len=edbf[cursor_row].len;

if (cursor_col==len-1) {
	K_del();
	K_cur_left();
} else {
	K_del();
}
}


void del_str(int dellen)
{
char *str=edbf[cursor_row].cstr;
int len=edbf[cursor_row].len;
int i;

	if (cursor_col+dellen > len) {
		extern int undo_idx;
error("del_str:dellen:%d len:%d cursor_col:%d undo_idx:%d\n",
dellen,len,cursor_col,undo_idx);
		return;
	}
	for(i=cursor_col;i<len-dellen;i++)
		str[i]=str[i+dellen];
	len-=dellen;
	if (len>0) {
		if ((str=(char *)mrealloc(str,len,
				"del_str:realloc"))==NULL) return;
	} else {
		free(str);
		str=0;
		len=0;
	}
	edbf[cursor_row].cstr=str;
	edbf[cursor_row].len=len;
	pos_cur();
	disp_cursor_line();
	show_cursor_w();
}


static void del_ch()
{
char *str=edbf[cursor_row].cstr;
int len=edbf[cursor_row].len;
int i,dellen;

if (cursor_col>=len) {
	if (cursor_row>=Lines-1) return;
	join_line();
} else {
		dellen=1;
	for(i=cursor_col;i<len-dellen;i++)
		str[i]=str[i+dellen];
	len-=dellen;
	if (len>0) {
		if ((str=mrealloc(str,len,"del_ch"))==NULL)
			return;
	} else {
		free(str);
		str=0;
		len=0;
	}
	edbf[cursor_row].cstr=str;
	edbf[cursor_row].len=len;
}
}


void del_Str(int dellen)
{
int i;
for (i=0;i<dellen;i++)
	del_ch();
if (cursor_col<page_width) page_col=0;
pos_cur();
disp_page();
show_cursor_w();
}


K_BS()
{
char *str=edbf[cursor_row].cstr;
int len=edbf[cursor_row].len;
int i,dellen,need_scancmt;
extern int c_comment;

s_fmod();
if (!cursor_col) {
	char *str1;
	int len1,nlen;

	if (!cursor_row) return 1;
	hide_cursor_w();
	cursor_row--;
	if (cursor_row<page_row) page_row=cursor_row;
	len1=edbf[cursor_row].len;
	u_split_line(len1,1);
	if (len) {
		str1=edbf[cursor_row].cstr;
		nlen=len+len1;
		if ((str1=(char *)realloc(str1,nlen))==NULL) {
			error("key_backspace:realloc");
			return;
		}
		memcpy(str1+len1,str,len);
		edbf[cursor_row].cstr=str1;
		edbf[cursor_row].len=nlen;
		free(str);
	}
	adj_mark_delline(cursor_row+1);
	del_mline(cursor_row+1);
	decr_line();
	cursor_col=len1;
	pos_cur();
	adj_page_col();
	disp_page();
	show_cursor_w();
	return 0;
}
#if     CHINESE
if (cur_x > 1 && B2nd(cur_y,cur_x-1) && B1st(cur_y,cur_x-2)) dellen=2;
else
#endif
	dellen=1;
need_scancmt=0;
if (c_comment && str[cursor_col-1]=='/' || str[cursor_col-1]=='*') {
	need_scancmt=1;
}
cursor_col-=dellen;
u_ins_str(dellen,1);
for(i=cursor_col;i<len-dellen;i++)
	str[i]=str[i+dellen];
len-=dellen;
if (len) {
	if ((str=(char *)realloc(str,len))==NULL) {
			error("key_backspace2:realloc");
			return;
	}
} else {
	free(str);
	str=0;
}
edbf[cursor_row].len=len;
edbf[cursor_row].cstr=str;
pos_cur();
adj_page_col();
if (need_scancmt) {
	scan_comment();
	disp_page();
} else
	disp_cursor_line();
show_cursor_w();
return 0;
}

void ins_return()
{
int nlen,len;
char *str;

len=edbf[cursor_row].len;
mmemmove((char *)&edbf[cursor_row+1],(char *)&edbf[cursor_row],
	 (Lines-cursor_row)*sizeof(LIN));
if (!cursor_col) {
	edbf[cursor_row].cstr=0;
	edbf[cursor_row].len=0;
} else
if (cursor_col==len) {
	edbf[cursor_row+1].cstr=0;
	edbf[cursor_row+1].len=0;
}
else {
	nlen=len-cursor_col;
	if ((str=mmalloc(nlen,"K_return: malloc"))==NULL)
		return;
	memcpy(str,&edbf[cursor_row].cstr[cursor_col],nlen);
	edbf[cursor_row+1].cstr=str;
	edbf[cursor_row+1].len=nlen;
	if (!(str=(char *)realloc(edbf[cursor_row].cstr,cursor_col))) {
		error("K_return: realloc");
		return;
	}
	edbf[cursor_row].cstr=str;
	edbf[cursor_row].len=cursor_col;
}

adj_mark_insline();
cursor_row++;
if (cur_y<page_len-1) {
	cur_y++;
} else {
	page_row++;
}
cur_x=cursor_col=page_col=0;
incr_line();
disp_page();
}

extern int insert_mode;
int vertical_mode=0;

void put_str(char *s, int slen)
{
char *str=edbf[cursor_row].cstr;
int len=edbf[cursor_row].len;
int nlen;
int i;
char *t,*ends;
s_fmod();
if (insert_mode) {
	ends=s+slen;

	while (s<ends) {
		if (*s=='\n') {
			ins_return();
			len=edbf[cursor_row].len;
			str=edbf[cursor_row].cstr;
			s++;
			continue;
		}
		t=s;
		slen=0;
		while (t<ends && *t!='\n') {
			t++;
			slen++;
		}
		nlen=len+slen;
		if ((str=mrealloc(str,nlen,"put_str:realloc"))==NULL)
			return;
		if (cursor_col==len) memcpy(str+len,s,slen);
		else {
			mmemmove(str+cursor_col+slen,str+cursor_col,len-cursor_col);
			memcpy(str+cursor_col,s,slen);
		}
		edbf[cursor_row].cstr=str;
		edbf[cursor_row].len=nlen;
		cursor_col+=slen;
		len=nlen;
		s=t;
	}
	pos_cur();
	adj_page_col();
	disp_page();
	show_cursor_w();
} else {
	nlen=cursor_col+slen;
	if (nlen < len) nlen=len;
	if ((str=mrealloc(str,nlen,"put_str:realloc"))==NULL)
		return;
	memcpy(str+cursor_col,s,slen);
	edbf[cursor_row].cstr=str;
	edbf[cursor_row].len=nlen;
	cursor_col+=slen;
	pos_cur();
	adj_page_col();
	disp_cursor_line();
	show_cursor_w();
}
}

static char *str_save=NULL;
static int str_alen, str_col, str_row, str_len;
int wrap_width=0;
int max_wrap_word=12;
int no_save=0;

K_PutStr(char *s)
{
int len=edbf[cursor_row].len;
char *ss=edbf[cursor_row].cstr;
int i,sep,slen=strlen(s);
char tt[1024];
int xlen;

if (AutoBlockRepDel && in_selection())
	K_DelArea();

if (wrap_width) {
	for(i=0;i<slen;i++) if (s[i]<=' ') break;
	if (cursor_col==len && i==slen) {
		i=col2x(cursor_col)+page_col;
		if (i+1>=wrap_width) {
			sep=0;
			for(i=cursor_col-1;i>cursor_col-max_wrap_word;i--)
			if (!isvarchr(ss[i]) && !isdig(ss[i])) {
				sep=1;
				break;
			}
#if	0
			printf("%d %d %d\n",col2x(i),page_col,wrap_width);
#endif
			if (col2x(i)+page_col < wrap_width && sep) {
				cursor_col=i+1;
				K_return();
				cursor_col=edbf[cursor_row].len;
			}
		}
	}
}
if (cursor_col==len) {
	xlen=x2eol();
	if (xlen>0) {
		memset(tt,' ',xlen);
		memcpy(tt+xlen,s,slen+1);
		s=tt;
		slen=slen+xlen;
	}
}
if (insert_mode)
	u_del_str(s,slen,1);
else
	u_rep_lines(cursor_row,cursor_row,1);

if (!no_save) {
	extern int saveBufType;
	if (str_len+slen+1>=str_alen) {
		int init_str=str_alen==0;
		str_alen=str_len+slen+16;
		if ((str_save=mrealloc(str_save,str_alen,"K_PutStr"))==NULL)
			return;
		if (init_str)
			str_save[0]=0;
	}
	if (str_row==cursor_row && str_col==cursor_col) {
		strcat(str_save,s);
		str_len+=slen+1;
	} else {
		strcpy(str_save,s);
		str_len=slen+1;
	}
	saveBufType=0;
}

put_str(s,slen);
str_row=cursor_row;
str_col=cursor_col;
if (vertical_mode) {
	if (xlen)
		K_cur_left();
	else
		K_cur_leftN(slen);
	K_cur_down();
}
return 0;
}

K_TogVertical()
{
vertical_mode^=1;
if (vertical_mode) message(4,"Vertical input mode");
show_cursor_w();
}

putStrSave()
{
if (!str_save) return;
no_save=1;
K_PutStr(str_save);
no_save=0;
}

static void SetWordWrap()
{
int i=atoi(return_item(0));
if (i<0) return;
wrap_width=i;
}

static int WordWrap_xgets_handle;

void SetWordWrap_init()
{
WordWrap_xgets_handle=xgets_handle();
add_edln(WordWrap_xgets_handle,0,6,20,10,6);
add_action(WordWrap_xgets_handle,SetWordWrap);
}

K_SetWordWrap()
{
input_handler=1;
save_win_context(active_win);
inactive_cursor();
box(18,8,19,5,7,8,"Word-Wrap Width");
init_xgets(WordWrap_xgets_handle);
return 0;
}

void rep_lines(int row, char *s, int slen)
{
char *str=edbf[row].cstr;
int len=edbf[row].len;
int i;
char *t,*ends;

ends=s+slen;
while (s<ends) {
	if (row>=Lines) {
		incr_line();
		edbf[row].cstr=0;
	}
	t=s;
	slen=0;
	while (t<ends && *t!='\n') {
		t++;
		slen++;
	}
	if (slen &&(str=mrealloc(edbf[row].cstr,slen,"put_str:realloc"))==NULL)
		return;
	if (slen) {
		memcpy(str,s,slen);
		edbf[row].cstr=str;
		edbf[row].len=slen;
	} else {
		edbf[row].len=0;
		edbf[row].cstr=0;
	}
	s=t+1;
	row++;
}
disp_page();
show_cursor_w();
}

K_pgtop()
{
hide_cursor_w();
cursor_row=page_row;
cursor_col=0;
pos_cur();
show_cursor_w();
return 0;
}

K_pgbottom()
{
hide_cursor_w();
cursor_row=page_row+page_len-1;
cursor_col=0;
pos_cur();
show_cursor_w();
return 0;
}

K_beginfile()
{
if (!page_row && !cursor_row && !cursor_col) return;
page_col=page_row=cursor_row=cursor_col=0;
pos_cur();
disp_page();
show_cursor_w();
return 0;
}

K_endfile()
{
page_row=Lines-page_len;
if (page_row < 0) page_row=0;
cursor_row=Lines-1;
cursor_col=edbf[cursor_row].len;
pos_cur();
adj_page_col();
disp_page();
show_cursor_w();
return 0;
}

void del_line()
{
del_mline(cursor_row);
if(cursor_row<Lines-1) decr_line();
else cur_x=cursor_col=0;
adj_mark_delline(cursor_row);
pos_cur();
disp_page();
show_cursor_w();
}

K_DelLn()
{
s_fmod();
u_ins_line(1);
del_line();
return 0;
}

K_DelLnN(int n)
{
int i;
for(i=0;i<n;i++)
	K_DelLn();
}

void ins_line(char *s, int len)
{
char *str;

mmemmove((char *)&edbf[cursor_row+1],(char *)&edbf[cursor_row],
	(Lines-cursor_row)*sizeof(LIN));
if (len) {
	if ((str=mmalloc(len,"K_return: malloc"))==NULL)
		return;
	memcpy(str,s,len);
} else str=0;
edbf[cursor_row].cstr=str;
edbf[cursor_row].len=len;
incr_line();
pos_cur();
disp_page();
show_cursor_w();
}

void split_line()
{
int nlen,len;
char *str;

hide_cursor_w();
len=edbf[cursor_row].len;
mmemmove((char *)&edbf[cursor_row+1],(char *)&edbf[cursor_row],
	 (Lines-cursor_row)*sizeof(LIN));
if (!cursor_col) {
	edbf[cursor_row].cstr=0;
	edbf[cursor_row].len=0;
} else
if (cursor_col==len) {
	edbf[cursor_row+1].cstr=0;
	edbf[cursor_row+1].len=0;
}
else {
	nlen=len-cursor_col;
	if ((str=mmalloc(nlen,"K_return: malloc"))==NULL)
		return;
	memcpy(str,&edbf[cursor_row].cstr[cursor_col],nlen);
	edbf[cursor_row+1].cstr=str;
	edbf[cursor_row+1].len=nlen;
	if (!(str=(char *)realloc(edbf[cursor_row].cstr,cursor_col))) {
		error("K_return: realloc");
		return;
	}
	edbf[cursor_row].cstr=str;
	edbf[cursor_row].len=cursor_col;
}

adj_mark_insline();
cursor_row++;
if (cur_y<page_len-1) {
	cur_y++;
} else {
	page_row++;
}
cur_x=cursor_col=page_col=0;
incr_line();
pos_cur();
disp_page();
show_cursor_w();
}

K_DelEol()
{
int len=edbf[cursor_row].len;
char *ss=edbf[cursor_row].cstr;

if (cursor_col>=len) return 1;
s_fmod();
u_rep_lines(cursor_row,cursor_row,1);
if (!cursor_col) {
	free(ss);
	edbf[cursor_row].len=0;
	edbf[cursor_row].cstr=0;
} else {
	if ((ss=mrealloc(ss,cursor_col,"delete_to_eol"))==NULL)
		return;
	edbf[cursor_row].len=cursor_col;
	edbf[cursor_row].cstr=ss;
}
hide_cursor_w();
disp_page();
show_cursor_w();
return 0;
}


K_DelBol()
{
int len=edbf[cursor_row].len;
char *ss=edbf[cursor_row].cstr;
int nlen;

if (!cursor_col) return 1;
s_fmod();
u_rep_lines(cursor_row,cursor_row,1);
if (cursor_col==len) {
	free(ss);
	edbf[cursor_row].len=0;
	edbf[cursor_row].cstr=0;
} else {
	mmemmove(ss,ss+cursor_col,nlen=len-cursor_col);
	if ((ss=mrealloc(ss,nlen,"delete_to_bol"))==NULL)
		return;
	edbf[cursor_row].len=nlen;
	edbf[cursor_row].cstr=ss;
}
cursor_col=0;
hide_cursor_w();
pos_cur();
adj_page_col();
disp_page();
show_cursor_w();
return 0;
}


char sep_chars[]=" {}()[]<>+-*/=%\\|&!,.:;\t\"";

in_sep_ch(char ch)
{
return ((int)strchr(sep_chars,ch));
}

K_DelWord()
{
int nlen,len=edbf[cursor_row].len;
char *ss=edbf[cursor_row].cstr;
int dellen=0;
char *tt=ss+cursor_col, *uu=ss+len;

if (cursor_col==len) return 1;
s_fmod();
u_rep_lines(cursor_row,cursor_row,1);
if (ispc(*tt))
	while (tt<uu && ispc(*tt)) { dellen++; tt++;}
else
if (in_sep_ch(*tt))
	while (tt<uu && in_sep_ch(*tt)) { dellen++; tt++;}
else 	while (tt<uu && !in_sep_ch(*tt)){ dellen++; tt++;}
if (cursor_col+dellen==len) {
	nlen=cursor_col;
} else {
	mmemmove(ss+cursor_col,ss+cursor_col+dellen,len-cursor_col-dellen);
	nlen=len-dellen;
}
if (!nlen) {
	free(ss);
	ss=0;
} else
if ((ss=mrealloc(ss,nlen,"delete_word"))==NULL)	return;

edbf[cursor_row].len=nlen;
edbf[cursor_row].cstr=ss;
hide_cursor_w();
disp_page();
show_cursor_w();
return 0;
}
