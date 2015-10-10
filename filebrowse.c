/*
	 by JULIAN MESA LLOPIS (a00409@eps.ua.es)
 */
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include "ted.h"

#ifndef NAME_MAX
#define NAME_MAX 256
#endif

static int fx,fy,lx,ly;
static int curx,cury;
static int fromf=0,fromd=0;
static char file_cad[30];
static unsigned short int postab=0,positem=0;
static char **files;
static char **dir;
extern char cur_dir[128];
static char direct[128]="";
static char busca_cad[30]="";
static int cnt_busca=0;

static int cntd=0,cntf=0;

void Itemup(int);
void Itemdown(int);
void Showfiles(void);
void Showdir(void);

char *selreg[16];
int SELREGN;

#define WIDTH_FILE 20

void init_reg_exp(char *s)
{
char tt[64];
int i,k;

for(i=0;i<SELREGN;i++)
	free(selreg[i]);

SELREGN=0;
strcpy(tt,"^");
k=1;
for(i=0;;i++)
if (s[i]=='*'||s[i]=='?') {
	strcat(tt,".");
	if (s[i]=='*')
		strcat(tt,"*");
} else
if (s[i]=='.')
	strcat(tt,"\\.");
else
if (s[i]==' ' || !s[i]) {
	strcat(tt,"$");
	selreg[SELREGN++]=strdup(tt);
	strcpy(tt,"^");
	if (!s[i]) {
		break;
	}
}
else strncat(tt,s+i,1);

}

static void open_file(char *s)
{
int i;

if (extend_edstate()) return;

hide_cursor_w();
if(!s[0]) return;
save_edstate();
s=decode_dir(s);
for(i=0;i<file_no;i++)
if (!strcmp(s,edstate[i].fname)) {
	goto_file(i);
	return;
}
#if	0
strcpy(fname,s);
#endif
edit_file(s);
disp_win_title();
}

void Itemup(int n)
{
int dif,from;
dif=positem-n;
if(dif<0)
{
	positem=0;
	if(!postab) from=fromf;
	else from=fromd;
	dif=dif*(-1);
	from-=dif;
	if(from<0) from=0;
	if(!postab)
	{
		if(fromf!=from)
		{
			fromf=from;
			Showfiles();
		}
	}
	else
	{
		if(fromd!=from)
		{
			fromd=from;
			Showdir();
		}
	}
}
else positem-=n;
}

void Itemdown(int n)
{
	int from,cnt;
	if(!postab)
	{
		from=fromf;
		cnt=cntf-1;
	}
	else
	{
		from=fromd;
		cnt=cntd-1;
	}
	if(!postab && from+positem==cntf-1) return;
	if(postab && from+positem==cntd-1) return;
	if(positem+n>ly-5 && positem+n<=cnt)
	{
		if(positem+n+from>cnt)
		{
			from=cnt-(ly-5);
			positem=ly-5;
		}
		else
		{
			from+=n-((ly-5)-positem);
			positem=ly-5;
		}
		if(!postab)
		{
			fromf=from;
			Showfiles();
		}
		else
		{
			fromd=from;
			Showdir();
		}
	}
	else
	{
		positem+=n;
		if(positem>ly-5)
		{
			if(!postab)
			{
				fromf=cnt-(ly-5);
				if(fromf<0) fromf=0;
			}
			else
			{
				fromd=cnt-(ly-5);
				if(fromd<0) fromd=0;
			}
			positem=ly-5;
			if(positem>cnt) positem=cnt;
			/*printf("pos: %d, from\n",positem);*/
			if(!postab) Showfiles();
			else Showdir();
		}
		else if(positem>cnt) positem=cnt;
	}
}

void Drawitem(int clr)
{
	void dispcad(char *,int);
	if(!clr) set_att(6);
	else set_att(1);
	if(!postab)
	{
		gotoxy(fx+2,fy+2+positem);
		dispcad(files[positem+fromf],WIDTH_FILE);
	}
	else
	{
		gotoxy(fx+23,fy+2+positem);
		dispcad(dir[positem+fromd],WIDTH_FILE);
	}
	draw_line(positem+fy+2);
}

match_selreg(char *fname)
{
int i;
#ifdef	USE_BSD_REG
char *err;
#else
regex_t	reg;
#endif
for(i=0;i<SELREGN;i++) {
#ifdef	USE_BSD_REG
	if (err=re_comp(selreg[i])) {
		error(err);
		return 1;
	}
	if (re_exec(fname)) {
		return 1;
	}
#else
	if (regcomp(&reg,selreg[i],0)) {
		return 1;
	}
	if (!regexec(&reg,fname,0,0,0)) {
		regfree(&reg);
		return 1;
	}
	regfree(&reg);
#endif
}
return 0;
}

void long_dir(int *lenfiles,int *lendir)
{
	DIR *dire;
	struct dirent *list;
	struct stat fst;
	*lenfiles=0;
	*lendir=0;

	dire=opendir(cur_dir);
	while((list=readdir(dire))!=NULL)	{
		if (!strcmp(list->d_name,".")) continue;
		stat(list->d_name,&fst);
		if (S_ISDIR(fst.st_mode))
			(*lendir)++;
		else
		if (match_selreg(list->d_name))
			(*lenfiles)++;
	}
	closedir(dire);
}

void freefiles()
{
	int i;
	for(i=0;files[i][0]!='\0';i++)
		free(files[i]);
	free(files);
	for(i=0;dir[i][0]!='\0';i++)
		free(dir[i]);
	free(dir);
}

void errmem()
{
	fprintf(stderr,"Error in memory allocation(file.c).\n");
	exit(-1);
}

int sort_function( const void *a, const void *b)
{
	 return( strcmp((char *)a,(char *)b) );
}

void leer_dir()
{
	int i;
	int ld,lf;
	DIR *dire;
	struct dirent *list;
	struct stat fst;
	char *aux1,*aux2;
	cntd=0;
	cntf=0;

	long_dir(&lf,&ld);
	lf+=2;
	ld+=2;
	aux1=(char*)malloc(sizeof(char)*(NAME_MAX+1)*lf);
	if(aux1==NULL) errmem();
	aux2=(char*)malloc(sizeof(char)*(NAME_MAX+1)*ld);
	if(aux2==NULL) errmem();
	files=(char**)malloc(sizeof(char*)*(lf));
	if(files==NULL) errmem();
	for(i=0;i<lf;i++)
	{
		files[i]=(char*)malloc(sizeof(char)*(NAME_MAX+1));
		if(files[i]==NULL) errmem();
	}
	dir=(char**)malloc(sizeof(char*)*(ld));
	if(dir==NULL) errmem();
	for(i=0;i<ld;i++)
	{
		dir[i]=(char*)malloc(sizeof(char)*(NAME_MAX+1));
		if(dir[i]==NULL) errmem();
	}

	dire=opendir(cur_dir);
	while((list=readdir(dire))!=NULL)	{
		if (!strcmp(list->d_name,".")) continue;
		stat(list->d_name,&fst);
		if (S_ISDIR(fst.st_mode))
		{
			strcpy(&aux2[cntd*(NAME_MAX+1)],list->d_name);
			cntd++;
		}
		else
		if (match_selreg(list->d_name))	{
			strcpy(&aux1[cntf*(NAME_MAX+1)],list->d_name);
			cntf++;
		}
	}
	closedir(dire);
	qsort((void *)aux1, cntf, sizeof(char)*(NAME_MAX+1), sort_function);
	qsort((void *)aux2, cntd, sizeof(char)*(NAME_MAX+1), sort_function);
	for(i=0;i<cntf;i++)
		strcpy(files[i],&aux1[i*(NAME_MAX+1)]);
	free(aux1);

	for(i=0;i<cntd;i++)
		strcpy(dir[i],&aux2[i*(NAME_MAX+1)]);
	free(aux2);

	files[cntf][0]='\0';
	dir[cntd][0]='\0';
}

void dispcad(char *cad,int wherex)
{
	int len,i;
	char aux[WIDTH_FILE+4];

	len=strlen(cad);
	if(len>=WIDTH_FILE)
	{
		strncpy(aux,cad,(WIDTH_FILE-3));
		aux[WIDTH_FILE-3]='\0';
		strcat(aux,"...");
		xputstrd(aux);
		len=strlen(aux);
	}
	else xputstrd(cad);
	for(i=0;i<wherex-len;i++) xputstrd(" ");
	draw_line(cury);
}

void Showfiles()
{
	int i;
	curx=fx+2;
	cury=fy+2;
	set_att(6);
	for(i=fromf;i<fromf+ly-4 && files[i][0]!='\0';i++)
	{
		gotoxy(curx,cury);
		dispcad(files[i],WIDTH_FILE);
		cury++;
	}
}

void Showdir()
{
	int i;
	curx=fx+23;
	cury=fy+2;
	set_att(6);
	for(i=fromd;i<fromd+ly-4 && dir[i][0]!='\0';i++)
	{
		gotoxy(curx,cury);
		dispcad(dir[i],WIDTH_FILE);
		cury++;
	}
}

void refresh_filebrowser()
{
	extern int cursor_x,cursor_y;
	if(input_handler==4)
	{
		cursor_x=cur_x+win_x;
		cursor_y=cur_y+win_y;
		inactive_cursor();
		cury=fy+1;
		gotoxy(fx+8,fy+1);
		set_att(14);
		dispcad("Files",0);
		gotoxy(fx+27,fy+1);
		dispcad("Directories",0);
		gotoxy(fx+20,fy+1);
		dispcad("/\\",0);
		gotoxy(fx+41,fy+1);
		dispcad("/\\",0);
		cury=ly-2;
		gotoxy(fx+20,ly-2);
		dispcad("\\/",0);
		gotoxy(fx+41,ly-2);
		dispcad("\\/",0);
		Showfiles();
		Showdir();
		Drawitem(1);
	}
}

void Draw_file_browser()
{
	int i;
	cnt_busca=0;
	busca_cad[0]='\0';
	if(direct[0]=='\0') strcpy(direct,cur_dir);
	postab=0;
	positem=0;
	fromf=0;
	fromd=0;
	cntd=0;
	cntf=0;
	fx=(page_width-45)/2;
	if(fx<2) fx=13;
	fy=0;
	lx=45;
	if(XW_ROW-1<6) ly=5;
	else ly=XW_ROW-2;
	box(fx,fy,lx,ly,7,8,"File browser");
	cury=fy+1;
	gotoxy(fx+8,fy+1);
	set_att(14);
	dispcad("Files",0);
	gotoxy(fx+27,fy+1);
	dispcad("Directories",0);
	gotoxy(fx+20,fy+1);
	dispcad("/\\",0);
	gotoxy(fx+41,fy+1);
	dispcad("/\\",0);
	cury=ly-2;
	gotoxy(fx+20,ly-2);
	dispcad("\\/",0);
	gotoxy(fx+41,ly-2);
	dispcad("\\/",0);
	leer_dir();
	Showfiles();
	Showdir();
	if(!cntf) postab=1;
	Drawitem(1);
	return;
}

/* On Solaris, if c> 0xff , isgraph will coredump, silly ? */
misgraph(int c)
{
if (c>0x20 && c< 0x7f) return 0;
else
return 1;
}

void process_file_browser(int skey,int xcur,int ycur)
{
	char cad[NAME_MAX+1];
	int i;
	if(!misgraph(skey)) cnt_busca=0;
	switch(skey)
	{
		case -1:
			if((xcur==fx+20 || xcur==fx+21) && ycur==fy+1)
			{
				if(!cntf) break;
				Drawitem(0);
				postab=0;
				Itemup(ly-4);
				Drawitem(1);
				break;
			}
			if((xcur==fx+41 || xcur==fx+42) && ycur==fy+1)
			{
				Drawitem(0);
				postab=1;
				Itemup(ly-4);
				Drawitem(1);
				break;
			}
			if((xcur==fx+20 || xcur==fx+21) && ycur==ly-2)
			{
				if(!cntf) break;
				Drawitem(0);
				postab=0;
				Itemdown(ly-4);
				Drawitem(1);
				break;
			}
			if((xcur==fx+41 || xcur==fx+42) && ycur==ly-2)
			{
				Drawitem(0);
				postab=1;
				Itemdown(ly-4);
				Drawitem(1);
				break;
			}
			if(xcur<fx+3 || xcur>fx+lx-3) break;
			if(ycur<fy+2 || ycur>fy+ly-3) break;
			if(xcur<fx+22)
			{
				/*i=ycur-fy-2+fromf;*/
				i=ycur-fy-2;
				if(i==positem && postab==0)
				{
				i+=fromf;
				if(i>cntf-1) break;
				if(strcmp(cur_dir,direct))
					sprintf(cad,"%s/%s",cur_dir,files[i]);
				else strcpy(cad,files[i]);
				freefiles();
				input_handler=0;
				xgets_enter=-1;
				close_box();
				redraw_twin();
				open_file(cad);
				K_CD(direct);
				direct[0]='\0';
				break;
				}
				else
				{
				Drawitem(0);
				postab=0;
				positem=i;
				Drawitem(1);
				break;
				}
			}
			if(xcur>fx+22)
			{
				/*i=ycur-fy-2+fromd;*/
				i=ycur-fy-2;
				if(i==positem && postab==1)
				{
				i+=fromd;
				if(i>cntd-1) break;
				strcpy(cad,dir[i]);
				freefiles();
				K_CD(cad);
				Draw_file_browser();
				break;
				}
				else
				{
				Drawitem(0);
				postab=1;
				positem=i;
				Drawitem(1);
				break;
				}
			}
			break;
		case XK_Escape:
			freefiles();
			input_handler=0;
			xgets_enter=-1;
			close_box();
			redraw_twin();
			K_CD(direct);
			direct[0]='\0';
			break;
		case XK_Prior:
			Drawitem(0);
			Itemup(ly-4);
			Drawitem(1);
			break;
		case XK_Next:
			Drawitem(0);
			Itemdown(ly-4);
			Drawitem(1);
			break;
		case XK_Up:
			Drawitem(0);
			Itemup(1);
			Drawitem(1);
			break;
		case XK_Down:
			Drawitem(0);
			Itemdown(1);
			Drawitem(1);
			break;
		case XK_Tab:
			if(!cntf) break;
			Drawitem(0);
			if(!postab) postab=1;
			else postab=0;
			if(positem>cntf-1 && postab==0) positem=cntf-1;
			if(positem>cntd-1 && postab==1) positem=cntd-1;
			Drawitem(1);
			break;
		case XK_Left:
			if(!cntf) break;
			Drawitem(0);
			if(positem>cntf-1) positem=cntf-1;
			postab=0;
			Drawitem(1);
			break;
		case XK_Right:
			Drawitem(0);
			if(positem>cntd-1) positem=cntd-1;
			postab=1;
			Drawitem(1);
			break;
		case XK_Return:
			if(!postab)
			{
				/*strcpy(cad,files[positem+fromf]);*/
				if(strcmp(cur_dir,direct))
					sprintf(cad,"%s/%s",cur_dir,files[positem+fromf]);
				else strcpy(cad,files[positem+fromf]);
				freefiles();
				input_handler=0;
				xgets_enter=-1;
				close_box();
				redraw_twin();
				open_file(cad);
				K_CD(direct);
				direct[0]='\0';
			}
			else
			{
				strcpy(cad,dir[positem+fromd]);
				freefiles();
				K_CD(cad);
				Draw_file_browser();
			}
			break;
		default:
			if(misgraph(skey))
			{
				busca_cad[cnt_busca]=skey;
				cnt_busca++;
				if(postab==0)
				{
					for(i=0;files[i][0]!='\0' &&
i<2000;i++)
					{
						if(!strncmp(files[i],busca_cad,cnt_busca))
						{
							fromf=cntf-(ly-4);
							if(fromf<0)
							{
								fromf=0;
								positem=cntf;
							}
							else positem=ly-4;
							if(cntf-i<=0)
positem=(ly-4)-(cntf-i);
							else Itemup(cntf-i);
							Showfiles();
							Drawitem(1);
							break;
						}
					}
					if(files[i][0]=='\0' || i==2000)
cnt_busca=0;
				}
				else
				{
					for(i=0;dir[i][0]!='\0' && i<2000;i++)
					{
						if(!strncmp(dir[i],busca_cad,cnt_busca))
						{
							fromd=cntd-(ly-4);
							if(fromd<0)
							{
								fromd=0;
								positem=cntd;
							}
							else positem=ly-4;
							if(cntd-i<=0)
positem=(ly-4)-(cntd-i);
							else Itemup(cntd-i);
							Showdir();
							Drawitem(1);
							break;
						}
					}
					if(dir[i][0]=='\0' || i==2000)
cnt_busca=0;
				}
			}
	}
}
