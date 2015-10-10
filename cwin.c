/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "ted.h"

#define GCC (1)
#define HPCC (2)
#define SUNCC (3)
#define AIXCC (4)

int CComp=1;

static char **cmsg;
static int msgcnt,msgacnt;
static int erow,epage;

init_cmsg()
{
if ((cmsg=mmalloc((msgacnt=10)*sizeof(char *),"ini_cmsg"))==NULL)
	return 1;
return 0;
}

static void free_cmsg()
{
int i;
if (cmsg) {
for(i=0;i<msgcnt;i++) free(cmsg[i]);
free(cmsg);
}
erow=epage=msgcnt=0;
}

int cwin_len=6;
void disp_cwin()
{
int y;
int len,row;
clear_rect(0,0,page_width,cwin_len);
for(row=epage,y=0; y<cwin_len && row<msgcnt; row++,y++) {
	gotoxy(0,y);
	if (row==erow) set_att(1);
	else set_att(0);
	xputstrd(cmsg[row]);
}
flush_buffer();
draw_scr();
}


static int fds[2],pid;
static int comp_status;

K_Compile(char *cmd)
{
char *bf;
char *arg[16],*aa,*bb,*ttt;
int len,i,cnt;

free_cmsg();
if (init_cmsg()) return;

if (pipe(fds)<0) {
	error("Cannot create pipe");
	return;
}
if (!(pid=fork())) {
	close(fds[0]);
#if	1
	aa=cmd;
	for(cnt=0;;) {
		bb=strchr(aa,' ');
		if (!bb) {
			if (*aa) arg[cnt++]=aa;
			break;
		}
		*bb=0;
		arg[cnt++]=aa;
		aa=bb+1;
	}
	arg[cnt]=0;
	dup2(fds[1],2);
	dup2(fds[1],1);
	execvp(arg[0],arg);
#else
	system(cmd);
	exit(0);
#endif
} else
if (pid<0) {
	error("Cannot fork process");
	return;
}
close(fds[1]);
init_fdbuf(1);
save_edstate();
win_mode(4);
center_cursor_row();
for(;;) {
	len=fdgets(&bf,fds[0]);
	if (!len) break;
	while (len)
 		if (bf[len-1]=='\n' || bf[len-1]=='\r') bf[--len]=0;
 		else
 			break;
 	if (!len) continue;
	else {
		bf=mrealloc(bf,len+1,"Compile");
		bf[len]=0;
	}
	if (bf[0]=='\r' || bf[0]=='\n' || bf[0]=='\t') { /* javac */
		mmemmove(bf,bf+1,len);
		len--;
	}
	if (len>XW_COL) {
		int i,ll;
		char *s;

		i=0;
		while (i<len) {
			ll=Min(len-i,XW_COL);
			if ((s=mmalloc(ll+1,"K_Compile"))==NULL) {
				return;
			}
			memcpy(s,bf+i,ll);
			s[ll]=0;
			cmsg[msgcnt++]=s;
			i+=XW_COL;
			if (msgcnt>=msgacnt) {
				msgacnt+=10;
				if ((cmsg=(char **)mrealloc(cmsg,sizeof(char *)*msgacnt,
					"compile"))==NULL) return;
			}
		}
		free(bf);
	} else
		cmsg[msgcnt++]=bf;
	disp_cwin();
	flush_x();
	if (msgcnt>=msgacnt) {
		msgacnt+=10;
		if ((cmsg=(char **)mrealloc(cmsg,sizeof(char *)*msgacnt,
			"compile"))==NULL) return;
	}
}
close(fds[0]);
comp_status=0;
wait(&comp_status);
if (!msgcnt||!comp_status) {
  input_handler=0;
	show_cursor_w();
	return 0;
}
#if	0
printf("status :%d\n", comp_status);
#endif
for(erow=0;erow<msgcnt;erow++) {
switch (CComp) {
case GCC:
	if ((ttt=strchr(cmsg[erow],':')) && *(ttt+1)>='0' && *(ttt+1)<='9')
		goto lll1;
	break;
case HPCC:
	if (!memcmp(cmsg[erow],"cc:",3))
		goto lll1;
case SUNCC:
case AIXCC:
	if (strstr(cmsg[erow],"\", line"))
		goto lll1;
	break;
}
}

lll1:
if (erow==msgcnt) erow=0;
disp_cwin();
input_handler=3;
hide_cursor_w();
inactive_cursor();
return 1;
}

K_CloseCpWin()
{
input_handler=0;
save_edstate();
restore_edstate();
win_mode(0);
}


void cwin_handler(KeySym skey)
{
char ll[512],*aa,*bb,*efname;
int len,lno,i;

switch (skey) {
	case XK_Up:
		if (erow==epage) {
			if (epage) epage--;
			erow=epage;
		} else
		if (erow) erow--;
		break;
	case XK_Down:
		if (erow>=epage+cwin_len-1) {
			if (epage<msgcnt-cwin_len) epage++;
		}
		if (erow<msgcnt-1) erow++;
		break;
	case XK_Home:
		epage=erow=0;
		break;
	case XK_End:
		epage=msgcnt-cwin_len;
		if (epage<0) epage=0;
		erow=msgcnt-1;
		break;
	case XK_Escape:
		K_CloseCpWin();
		return;
	case XK_Return:
		input_handler=0;
		strcpy(ll,cmsg[erow]);
		len=strlen(ll);
switch	(CComp) {
case GCC:
		if (!(aa=strchr(ll,':'))) {
err_l_gcc:
			show_cursor_w();
			return;
		}
		*(aa++)=0;
		if (!(bb=strchr(aa,':'))) goto err_l_gcc;
		*bb=0;
		efname=ll;
		lno=atoi(aa);
		if (lno<=0) goto err_l_gcc;
		goto lll3;
case HPCC:
		if (!(aa=strchr(ll,'"'))) {
err_l_hpcc:
			show_cursor_w();
			return;
		}
		aa++;
		if (!(bb=strchr(aa,'"'))) goto err_l_hpcc;
		*(bb++)=0;
		efname=aa;
		bb+=7;
		aa=strchr(bb,':');
		*aa=0;
		lno=atoi(bb);
		if (lno<=0) goto err_l_hpcc;
		goto lll3;
case SUNCC:
		if (ll[0]!='"') {
err_l_suncc:
			show_cursor_w();
			return;
		}
		efname=ll+1;
		if (!(aa=strchr(efname,'"'))) goto err_l_suncc;
		*aa=0;
		aa+=8;
		bb=strchr(aa,':');
		*bb=0;
		lno=atoi(aa);
		if (lno<=0) goto err_l_hpcc;
		goto lll3;
case AIXCC:
		if (ll[0]!='"') {
err_l_aixcc:
			show_cursor_w();
			return;
		}
		efname=ll+1;
		if (!(aa=strchr(efname,'"'))) goto err_l_aixcc;
		*aa=0;
		aa+=8;
		bb=strchr(aa,'.');
		*bb=0;
		lno=atoi(aa);
		if (lno<=0) goto err_l_aixcc;
		goto lll3;
}
lll3:
		for(i=0;i<file_no;i++)
			if (!strcmp(edstate[i].fname,efname)) break;
		if (i==file_no) goto err_l_gcc;
		hide_cursor_w();
		goto_file(i);
		cursor_row=lno-1;
		cursor_col=0;
		center_cursor_row();
		disp_edstate();
		return;
}
disp_cwin();
}

K_TogCpEdit()
{
if (input_handler==0 && msgcnt) {
	save_edstate(cur_file);
	input_handler=3;
	win_mode(4);
	hide_cursor_w();
	inactive_cursor();
	disp_cwin();
}
}

canCloseCwin()
{
extern win_split;
if (win_split==4 && !comp_status)
  K_CloseCpWin();
}
