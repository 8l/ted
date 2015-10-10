/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include <sys/types.h>
#include <regex.h>
#include "ted.h"

#define IGN_CASE (1)
#define BACKWARD (2)
#define REGEXP (8)

#define WHOLEWORD (16)

int search_option;
static char sstr[40],rep_str[40];
extern char *return_item();
static int s_row,s_col, e_row;
static int first_time=0, in_lblk;
static int s_fileid;

static search(int errmsg, int s_opt)
{
int i,j,k,len,slen,col, sop;
char *ss;
slen=strlen(sstr);
if (!slen) return;

s_fileid=cur_file;

switch (s_opt&15) {
case 0:
	if (s_row>=Lines) goto not_found_0;
	len=edbf[s_row].len;
	ss=edbf[s_row].cstr;
	for(j=s_col+1;j<=len-slen;j++) {
		if (j+slen > len) break;
		if (!memcmp(ss+j,sstr,slen)) {
			if ((search_option&WHOLEWORD) && (j && isvardig(ss[j-1])||
				j+slen<len && isvardig(ss[j+slen])) )
				continue;
			goto found;
		}
	}
	for(s_row++;s_row<=e_row;s_row++) {
		ss=edbf[s_row].cstr;
		len=edbf[s_row].len;
		for(j=0;j<=len-slen;j++)
		if (!memcmp(&ss[j],sstr,slen)) {
			if ((search_option&WHOLEWORD) && (j && isvardig(ss[j-1])||
				j+slen<len && isvardig(ss[j+slen])) )
				continue;
			goto found;
		}
	}
not_found_0:
	if (!errmsg && !first_time) return 0;
	show_cursor_w();
	message(4,"Cannot find string");
	save_win_context(active_win);
	return 0;
case IGN_CASE:
	if (s_row>=Lines) goto not_found_1;
	ss=edbf[s_row].cstr;
	len=edbf[s_row].len;
	for(j=s_col+1;j<=len-slen;j++) {
		if (j+slen > len) break;
		for(k=0;k<slen && mtoupper(ss[k+j])==mtoupper(sstr[k]);k++);
		if (k==slen) {
		   if ((search_option&WHOLEWORD) && (k && isvardig(ss[k-1])||
				k+slen<len && isvardig(ss[k+slen])) )
				continue;
			goto found;
		}
	}
	for(s_row++;s_row<=e_row;s_row++) {
		ss=edbf[s_row].cstr;
		len=edbf[s_row].len;
		for(j=0;j<=len-slen;j++) {
		   for(k=0;k<slen && mtoupper(ss[j+k])==mtoupper(sstr[k]);k++);
		   if (k<slen || ((search_option&WHOLEWORD) &&	(j && isvardig(ss[j-1])||
				j+slen<len && isvardig(ss[j+slen]))) )
				continue;
			goto found;
		}
	}
not_found_1:
	if (!errmsg && !first_time) return 0;
	disp_page();
	redraw_status();
	message(4,"Cannot find string");
	disp_mode();
	save_win_context(active_win);
	show_cursor_w();
	return 0;
case BACKWARD:
	if (s_row < 0) goto not_found_2;
	len=edbf[s_row].len;
	ss=edbf[s_row].cstr;
	for(j=s_col-1;j>=0;j--) {
		if (j+slen > len)	break;
		if (!memcmp(ss+j,sstr,slen)) {
			if ((search_option&WHOLEWORD) && (j && isvardig(ss[j-1])||
				j+slen<len && isvardig(ss[j+slen])) )
				continue;
			goto found;
		}
	}
	for(s_row--;s_row>=0;s_row--) {
		ss=edbf[s_row].cstr;
		len=edbf[s_row].len;
		for(j=len-slen;j>=0;j--)
		if (!memcmp(&ss[j],sstr,slen)) {
			if ((search_option&WHOLEWORD) && (j && isvardig(ss[j-1])||
				j+slen<len && isvardig(ss[j+slen])) )
				continue;
			goto found;
		}
	}
not_found_2:
	if (!errmsg && !first_time) return 0;
	show_cursor_w();
	message(4,"Cannot find string");
	save_win_context(active_win);
	return 0;
case BACKWARD|IGN_CASE:
	if (s_row<0) goto not_found_3;
	ss=edbf[s_row].cstr;
	len=edbf[s_row].len;
	for(j=s_col-1;j>=0;j--) {
		if (j+slen > len) break;
		for(k=0;k<slen && mtoupper(ss[k+j])==mtoupper(sstr[k]);k++);
		if (k==slen) {
		   if ((search_option&WHOLEWORD) && (k && isvardig(ss[k-1])||
				k+slen<len && isvardig(ss[k+slen])) )
				continue;
			goto found;
		}
	}
	for(s_row--;s_row>=0;s_row--) {
		ss=edbf[s_row].cstr;
		len=edbf[s_row].len;
		for(j=len-slen;j>=0;j--) {
		   for(k=0;k<slen && mtoupper(ss[j+k])==mtoupper(sstr[k]);k++);
		   if (k==slen) {
		   if ((search_option&WHOLEWORD) && (k && isvardig(ss[k-1])||
				k+slen<len && isvardig(ss[k+slen])) )
				continue;
		   	goto found;
		   }
		}
	}
case REGEXP|IGN_CASE:
case REGEXP:
	{
		regex_t	reg;
		u_char sch;
		char *ttt=0;
		int flag=0;
		int alen=0;

		regmatch_t regm[1];

		if (s_opt&IGN_CASE)
			flag=REG_ICASE;

		if (s_row>=Lines) goto not_found_0;
		if (regcomp(&reg,sstr,flag)) {
			disp_page();
			redraw_status();
	  	message(4,"Bad regular expr");
			disp_mode();
			save_win_context(active_win);
			show_cursor_w();
			return 0;
	  }
		len=edbf[s_row].len;
		ss=edbf[s_row].cstr;
		if (!ss) goto lll2;
		if (alen <= len && (ttt=mmalloc(len+1,"search"))==NULL) {
			return 0;
		} else alen=len+1;
		memcpy(ttt,ss,len);
		ttt[len]=0;
		if (s_col+1<len && !regexec(&reg,ttt+s_col+1,1,regm,flag)) {
				j=regm[0].rm_so+s_col+1;
				regfree(&reg);
				free(ttt);
				goto found;
		}
lll2:
		for(s_row++;s_row<=e_row;s_row++) {
			ss=edbf[s_row].cstr;
			len=edbf[s_row].len;
			if (!ss || !len) continue;
			if (alen <= len && (ttt=mrealloc(ttt,len+1,"search"))==NULL) {
				return 0;
			} else alen=len+1;
			memcpy(ttt,ss,len);
			ttt[len]=0;
			if (!regexec(&reg,ttt,1,regm,flag)) {
				j=regm[0].rm_so;
				regfree(&reg);
				free(ttt);
				goto found;
			}
		}
		if (ttt) free(ttt);
		regfree(&reg);
	}
	break;
case REGEXP|IGN_CASE|BACKWARD:
case REGEXP|BACKWARD:
	{
		regex_t	reg;
		u_char sch;
		char *ttt=0;
		int flag=0;
		int alen=0;
		regmatch_t regm[1];

		if (s_opt&IGN_CASE)
			flag=REG_ICASE;

		if (s_row<0 || !s_row&&!s_col ) goto not_found_0;
		if (regcomp(&reg,sstr,flag)) {
			disp_page();
			redraw_status();
	  	message(4,"Bad regular expr");
			disp_mode();
			save_win_context(active_win);
			show_cursor_w();
			return 0;
	  }
		len=edbf[s_row].len;
		ss=edbf[s_row].cstr;
		if (!ss) goto lll3;
		if (alen <= len && (ttt=mmalloc(len+1,"search"))==NULL) {
			return 0;
		} else alen=len+1;
		memcpy(ttt,ss,len);
		ttt[s_col]=0;
		if (s_col-1>=0 && !regexec(&reg,ttt,1,regm,flag)) {
				j=regm[0].rm_so+s_col-1;
				regfree(&reg);
				free(ttt);
				goto found;
		}
lll3:
		for(s_row--;s_row>=0;s_row--) {
			ss=edbf[s_row].cstr;
			len=edbf[s_row].len;
			if (!ss || !len) continue;
			if (alen <= len && (ttt=mrealloc(ttt,len+1,"search"))==NULL) {
				return 0;
			} else alen=len+1;
			memcpy(ttt,ss,len);
			ttt[len]=0;
			if (!regexec(&reg,ttt,1,regm,flag)) {
				j=regm[0].rm_so;
				regfree(&reg);
				free(ttt);
				goto found;
			}
		}
		if (ttt) free(ttt);
		regfree(&reg);
	}
}
not_found_3:
	if (!errmsg && !first_time) return 0;
	disp_page();
	redraw_status();
	message(4,"Cannot find string");
	disp_mode();
	save_win_context(active_win);
	show_cursor_w();
	return 0;
found:
#if	0
printf("row:%d\n",row);
#endif
hide_cursor_w();
if (s_row<page_row || s_row>=page_row+page_len) page_row=s_row-1;
if (page_row<0) page_row=0;
cursor_row=s_row;
cursor_col=j;
page_col=0;
cur_x=col2x(cursor_col);
cur_y=cursor_row-page_row;
if (cur_x+slen > page_width) {
	page_col+=cur_x-page_width+slen;
	cur_x=col2x(cursor_col);
}
save_win_context(active_win);
disp_page();
redraw_status();
show_cursor_w();
return 1;
}

int search_source;

static void start_search(char *s, char *t)
{
	if (!search_source) {
		s=return_item(0);
		t=return_item(1);
	}
	if (!s[0]) return;
	search_option=0;
	s_row=cursor_row;
	s_col=cursor_col;
	e_row=Lines-1;
	for(;*t;t++)
	if (mtoupper(*t)=='I') search_option|=IGN_CASE;
	else
	if (*t=='1') s_row=s_col=0;
	else
	if (mtoupper(*t)=='B') search_option|=BACKWARD;
	else
	if (mtoupper(*t)=='W') search_option|=WHOLEWORD;
	else
	if (mtoupper(*t)=='R') search_option|=REGEXP;
	else
  if (mtoupper(*t)=='L' && mline_begin>=0) {
    s_row=mline_begin; s_col=-1;
    e_row=mline_end;
    search_option&=~BACKWARD;
    in_lblk=1;
  }
	strcpy(sstr,s);
	search(1,search_option);
}

static int s_xgets_handle;
void search_menu_init()
{
s_xgets_handle=xgets_handle();
add_edln(s_xgets_handle,0,6,25,10,20);
add_edln(s_xgets_handle,1,6,25,12,4);
add_action(s_xgets_handle,start_search);
}

static XYstr sprompt[]={
{20,10,"Find"},
{20,12,"Opt"},
{30,12,"1:row 1 i:Ign case"},
{22,13,"w:word b:backward r:reg exp"},
{22,14,"l:in Line blk"}
};

void enable_start_search()
{
	input_handler=1;
	search_source=0;
	save_win_context(active_win);
	inactive_cursor();
	box(18,8,32,8,7,8,"Search for string");
	xystr(sprompt,5);
	init_xgets(s_xgets_handle);
}

void K_LocateStr(char *s, char *opt)
{
search_source=1;
start_search(s,opt);
}

K_RepFind()
{
	s_row=cursor_row;
	s_col=cursor_col;
	if (e_row >= Lines) e_row=Lines-1;
	if (sstr[0]) search(1,search_option);
	return 0;
}

static void rep_with_str()
{
char *s=edbf[cursor_row].cstr;
int len=edbf[cursor_row].len;
int slen=strlen(sstr);
int rlen=strlen(rep_str);
int nlen=len;
s_fmod();
if (rlen > slen) {
	if ((s=mrealloc(s,nlen=len+rlen-slen,"replace_str_yn"))==NULL)
		return;
	mmemmove(s+cursor_col+rlen,s+cursor_col+slen,
			len-cursor_col-slen);
} else
if (rlen < slen) {
	mmemmove(s+cursor_col+rlen,s+cursor_col+slen,
			len-cursor_col-slen);
	if ((s=mrealloc(s,nlen=len+rlen-slen,"replace_str_yn"))==NULL)
		return;
}
memcpy(s+cursor_col,rep_str,rlen);
edbf[cursor_row].cstr=s;
edbf[cursor_row].len=nlen;
cursor_col+=rlen;
s_row=cursor_row;
s_col=cursor_col;
}



static replace_str_yn(char ch)
{
if (ch=='Y') {
	rep_with_str();
	disp_page();
	show_cursor_w();
}
s_row=cursor_row;
s_col=cursor_col;
if (search(1,search_option)) {
	message(5,"Replace (Y/N):",fname);
	setup_getch(15,MROW-1,"YNyn",'Y',replace_str_yn);
	disp_page();
	show_cursor_w();
}
}

int replace_source;

void start_replace(char *s,char *repl,char *opt)
{
char *t,*r;
int no_prom;

if (!replace_source) {
	s=return_item(0);
	strcpy(rep_str,return_item(1));
	t=return_item(2);
} else {
	strcpy(rep_str,repl);
	strcpy(t,opt);
}
if (!s[0]) return;
in_lblk=no_prom=search_option=0;
s_row=cursor_row;
s_col=cursor_col;
e_row=Lines-1;
for(;*t;t++)
if (mtoupper(*t)=='I') search_option|=IGN_CASE;
else if (mtoupper(*t)=='N') no_prom=1;
else
if (*t=='1') s_row=s_col=0;
else
if (mtoupper(*t)=='B') search_option|=BACKWARD;
else
if (mtoupper(*t)=='R') search_option|=REGEXP;
else
if (mtoupper(*t)=='W') search_option|=WHOLEWORD;
else
if (mtoupper(*t)=='L' && mline_begin>=0) {
	s_row=mline_begin; s_col=-1;
	e_row=mline_end;
	search_option&=~BACKWARD;
	in_lblk=1;
}

strcpy(sstr,s);
if (no_prom) {
	first_time=1;
	while (search(0,search_option) && (!in_lblk || s_row <= mline_end) ) {
		first_time=0;
		s_row=cursor_row;
		s_col=cursor_col;
		rep_with_str();
	}
	disp_page();
	show_cursor_w();
	return;
} else
if (search(1,search_option)) {
	message(5,"Replace (Y/N):",fname);
	setup_getch(15,MROW-1,"YNyn",'Y',replace_str_yn);
	show_cursor_w();
}
}

static int r_xgets_handle;
void replace_init()
{
r_xgets_handle=xgets_handle();
add_edln(r_xgets_handle,0,6,25,9,23);
add_edln(r_xgets_handle,1,6,25,11,23);
add_edln(r_xgets_handle,2,6,25,13,4);
add_action(r_xgets_handle,start_replace);
}

static XYstr rprompt[]={
{20,9,"Find"},
{20,11,"Repl"},
{20,13,"Opt"},
{30,13,"1:row 1 i:Ignore case"},
{30,14,"n:No prompt b:backward"},
{21,15,"w:word r:reg exp l:in Line blk"},
};

void enable_start_replace()
{
	input_handler=1;
	replace_source=0;
	save_win_context(active_win);
	inactive_cursor();
	box(18,7,36,10,7,8,"Find and Replace String");
	xystr(rprompt,6);
	init_xgets(r_xgets_handle);
}

void K_ReplaceStr(char *s,char *r,char *o)
{
replace_source=1;
start_replace(s,r,o);
}

K_FindMarkStr()
{
char tt[128];
if (!GetMarkWord(tt)) return;
search_option=0;
strcpy(sstr,tt);
search(1,search_option);
}

K_PrevFind()
{
s_col=cursor_col;
s_row=cursor_row;
search(1,search_option^BACKWARD);
return 1;
}
