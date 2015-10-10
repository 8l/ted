/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include "ted.h"

char cur_dir[128];

void get_cdir()
{
getcwd(cur_dir,sizeof(cur_dir));
}

void disp_cdir()
{
int i;
int len=strlen(cur_dir);
int ofs=0;

ofs=len - (MCOL-20) ;
if (ofs<0) ofs=0;
xputs(0,edwin[0].MROW-2+edwin[0].y,cur_dir+ofs,2);
}

char *decode_dir(char *s)
{
char *orgs=s;
static char path[80];
if (*s=='~') {
	s++;
	if (*s=='/' || !(*s)) {
		strcpy(path,getenv("HOME"));
		if (*s=='/') strcat(strcat(path,"/"),s+1);
		return path;
	} else {
		char ss[80],*t;
		struct passwd *pw;
		strcpy(ss,s);
		if (t=strchr(ss,'/')) {
			*t=0;
			t++;
		}
		if (!(pw=getpwnam(ss))) return orgs;
		strcat(strcpy(path,pw->pw_dir),"/");
		if (t) strcat(path,t);
	}
} else strcpy(path,s);
return path;
}


K_CD(char *s)
{
char tt[128];
s=decode_dir(s);
if (!chdir(s)) {
	get_cdir();
	draw_cmd_line();
#if	0
	setenv("PWD",cur_dir,1);
#else
	strcat(strcpy(tt,"PWD="),s);
#endif
	return 0;
}
return 1;
}

void exe_chdir()
{
char dire[80];
strcpy(dire,return_item(0));
redraw_twin();
if (K_CD(dire)) {
	message(4,"Error CD %s",dire);
	save_win_context(active_win);
}
}

static int chdir_xgets_handle;
void chdir_init()
{
chdir_xgets_handle=xgets_handle();
add_edln(chdir_xgets_handle,0,6,20,10,27);
add_action(chdir_xgets_handle,exe_chdir);
}

void K_ChDir()
{
input_handler=1;
save_win_context(active_win);
inactive_cursor();
box(18,8,32,5,7,8,"Change Directory");
init_xgets(chdir_xgets_handle);
}
