/*
	Copyright (C)   1995
	Edward  Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include <sys/types.h>
#include <stdio.h>
#include "ted.h"
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#ifndef	SEEK_CUR
#define SEEK_CUR (2)
#endif

int file_no,nosave;
extern int MAX_FILE;
char tedfilepos[]=".tedfilepos";

char fname[256]="#noname";
char fdir[128];

mfgets(char *s,int len,FILE *fp)
{
int n,i;
char *t=s;
for(i=0;i<len;i++) {
	n=fread(s,1,1,fp);
	if (!n) break;
	if (*s=='\n') {
		s++;
		break;
	}
	s++;
}
return s-t;
}

int need_autosave=0;
void s_fmod()
{
f_modified=1;
need_autosave=1;
}

static char fdbf[2048];
static int bfsize,bfofs,breadsize;

void init_fdbuf(int bsize)
{
bfsize=bfofs=0;
breadsize=bsize;
}

fdgets(char **s,int fd)
{
int n,i;
char *t;
int allen=40,len;

if ((t=mmalloc(allen,"fdgets"))==NULL) return;
len=0;

for(;;) {
	if (bfofs>=bfsize) {
		if ((bfsize=read(fd,fdbf,breadsize)) <= 0) break;
		bfofs=0;
	}
	if (len>=allen) {
		allen+=80;
		if ((t=mrealloc(t,allen,"fdgets2"))==NULL) return;
	}
	if ((t[len++]=fdbf[bfofs++])=='\n')
		break;
}
if (!len) {
	free(t);
	*s=0;
} else
*s=mrealloc(t,len,"fdgets2");
return len;
}

static char hostname[40];
static char user_name[12];

void disp_win_title()
{
char ttt[256];
sprintf(ttt,"%s %s@%s",fname, user_name, hostname);
change_win_name(ttt);
}

void save_edstate()
{
int f_no=cur_file;

if (extend_edstate()) return;

edwin[active_win].pg_pos[f_no].cur_y=cur_y;
edwin[active_win].pg_pos[f_no].cur_x=cur_x;
edwin[active_win].pg_pos[f_no].page_row=page_row;
edwin[active_win].pg_pos[f_no].page_col=page_col;
edwin[active_win].pg_pos[f_no].cursor_row=cursor_row;
edwin[active_win].pg_pos[f_no].cursor_col=cursor_col;
edwin[active_win].cur_file=f_no;
edstate[f_no].f_modified=f_modified;
edstate[f_no].nosave=nosave;
edstate[f_no].readonly=readonly;
edstate[f_no].edbf=edbf;
edstate[f_no].Alloc_Lines=Alloc_Lines;
edstate[f_no].Lines=Lines;
strcpy(edstate[f_no].fname,fname);
strcpy(edstate[f_no].fdir,fdir);
edstate[f_no].cmapno=cmapno;
}


static int fds[2];
int StripCR=1;

read_file(char *fn, int pope)
{
char *bf,*str;
int len;
int pid=0,status;
int fd;

if (pope) {
	if (pipe(fds)<0) {
		error("Cannot create pipe");
		return -1;
	}
	if (!(pid=fork())) {
		dup2(fds[1],1);
		dup2(fds[1],2);
		close(fds[0]);
		if (strchr(fn,' '))
			execl("/bin/sh","sh","-c",fn,NULL);
		else    execlp(fn,fn,NULL);
	} else
	if (pid<0) {
		error("Cannot fork process");
		return -1;
	}
	close(fds[1]);
	fd=fds[0];
}
else
if (!strcmp(fn,"-")) fd=0;
else
if ((fd=open(fn,O_RDONLY))<0)
		bell();

if (fn[0]!='/') {
	getcwd(fdir,sizeof(fdir));
} else {
	strcpy(fdir,"/");
}

if (!pope && file_no<20) {
	if (test_file(fdir,fn)) {
    if (cur_file>=file_no)
	    cur_file=file_no-1;
	  return -1;
	}
}
strcpy(fname,fn);

init_edbf();
edbf[0].cstr=0;
edbf[0].len=0;
if(fd<0) {
	Lines=1;
	disp_win_title();
	return 0;
}
Lines=0;
init_fdbuf(sizeof(fdbf));
for(;;) {
	len=fdgets(&bf,fd);
	if (!len) {
		edbf[Lines].len=0;
		edbf[Lines].cstr=0;
		break;
	}
	if (bf[len-1]=='\n') bf[--len]=0;
	   	if (StripCR && len && bf[len-1]=='\r') bf[--len]=0;
	edbf[Lines].len=len;
	if (!len)
		edbf[Lines].cstr=0;
	else
	  edbf[Lines].cstr=bf;
	incr_line();
}
close(fd);
if (pid) wait(&status);
if (Lines==0) {
	edbf[0].cstr=0;
	edbf[0].len=0;
	Lines=1;
}
nosave=pope;
disp_win_title();
K_SwitchKeyDef(0);
return 0;
}

int BackupSave;
void save_filename(char *ffname, int bak)
{
FILE *fw;
int i,wn;

if (BackupSave && bak) {
	char tt[256];
	if ((fw=fopen(ffname,"r+"))==NULL) {
/*		message(4,"Error creating %s", ffname); */
		goto lll1;
	}
	fclose(fw);
	sprintf(tt,"cp -p '%s' '%s~'",ffname,ffname);
	if (system(tt)) {
		message(4,"Cannot copy file for backup");
	}
}
lll1:
if ((fw=fopen(ffname,"w"))==NULL) {
	message(4,"Error creating %s", ffname);
	return;
}
for(i=0;i<Lines;i++) {
	if (i) {
		wn=fwrite("\n",1,1,fw);
		if (wn<=0) {
diskfull:
			message(4,"write error, disk full ??");
			return;
		}
	}
	if (edbf[i].len) {
		wn=fwrite(edbf[i].cstr,1,edbf[i].len,fw);
		if (wn<=0)
			goto diskfull;
	}
}
fwrite("\n",1,1,fw);
fsync(fileno(fw));
message(5,"%s  %d lines saved",ffname,Lines);
fclose(fw);
}

int LeadingSpaceTab=1;
int TrimBlankTail=1;
K_SaveFile()
{
int i;

if (LeadingSpaceTab) leading_space_tab();
if (TrimBlankTail) trim_blank_tail();
save_filename(fname,1);
need_autosave=f_modified=0;
return 0;
}



void restore_edstate()
{
int f_no=cur_file;

cur_y=edwin[active_win].pg_pos[f_no].cur_y;
cur_x=edwin[active_win].pg_pos[f_no].cur_x;
page_row=edwin[active_win].pg_pos[f_no].page_row;
page_col=edwin[active_win].pg_pos[f_no].page_col;
cursor_row=edwin[active_win].pg_pos[f_no].cursor_row;
cursor_col=edwin[active_win].pg_pos[f_no].cursor_col;

f_modified=edstate[f_no].f_modified;
nosave=edstate[f_no].nosave;
readonly=edstate[f_no].readonly;
edbf=edstate[f_no].edbf;
Lines=edstate[f_no].Lines;
Alloc_Lines=edstate[f_no].Alloc_Lines;
strcpy(fname,edstate[f_no].fname);
strcpy(fdir,edstate[f_no].fdir);
set_lang_type(fname);
K_SwitchKeyDef(edstate[f_no].cmapno);
}

void disp_edstate()
{
file_modified=-1;
disp_page();
disp_fname();
disp_win_title();
disp_cur_pos();
pos_cur();
show_cursor_w();
}

int endian_test()
{
int tt;
memcpy(&tt,"\x11\x22\x33\x44",4);
if (tt==0x11223344) return 1;
return 0;
}

int endian_conv(char *s)
{
char t;
if (endian_test()) {
	t=*s;
	*s=*(s+3);
	*(s+3)=t;
	t=*(s+1);
	*(s+1)=*(s+2);
	*(s+2)=t;
}
}


extend_edstate()
{
if (file_no>=MAX_FILE) {
	MAX_FILE+=10;
	if ((edstate=mrealloc(edstate,(MAX_FILE+1)*sizeof(EDSTATE),"extend_edstate"))
		==NULL) {
		message(4," Open too many file ");
		return 1;
	}
}
return 0;
}

int getFileWininfo(char *fname, FPOS *fposp)
{
FILE *fp;
int match=0;
struct stat st;
static char tedfpos[]=".tedfilepos";

stat(tedfpos,&st);
if (st.st_size%sizeof(FPOS)) {
  unlink(tedfpos);
  return 0;
}
if ((fp=fopen(tedfpos,"r"))==NULL)
  return 0;

bzero(fposp,sizeof(FPOS));
while (!feof(fp)) {
  fread(fposp,sizeof(FPOS),1,fp);
  if (!strcmp(fposp->fname,fname)) {
    match=1;
    break;
  }
}
fclose(fp);
if (match) {
  endian_conv((char *)&fposp->page_row);
  endian_conv((char *)&fposp->cursor_row);
  endian_conv((char *)&fposp->cursor_col);
  endian_conv((char *)&fposp->wrow);
  endian_conv((char *)&fposp->wcol);
  endian_conv((char *)&fposp->wx);
  endian_conv((char *)&fposp->wy);
  endian_conv((char *)&fposp->fntidx);
  return 1;
}

return 0;
}

char *goto_tagname=NULL;

int vi_goto_no=-1;

int RestCursor,RestWinPos,RestWinSize;

void close_file_id();

edit_file(char *fn)
{
FILE *fp;
extern int c_comment, mult_open;
struct stat sta;

if (extend_edstate()) return -1;
save_win_context(active_win);
cur_file=file_no;
/* insert_mode=1; */
if (read_file(fn,0)) return -1;
file_no++;
bzero(&edstate[cur_file],sizeof(edstate[0]));
readonly=page_row=page_col=cursor_row=cursor_col=cur_y=cur_x=f_modified=0;

if (!stat(fn,&sta)) {
  if (fp=fopen(fn,"a"))
    fclose(fp);
  else
    readonly=1;
}

if (vi_goto_no >=0) {
	goto_line(vi_goto_no);
	save_edstate();
	vi_goto_no=-1;
} else {
  FPOS fpos;
  extern int fontidx;

  if (!getFileWininfo(fn,&fpos))
    goto ll1;

  if (RestCursor) {
    page_row=fpos.page_row;
    cursor_row=fpos.cursor_row;
    cursor_col=fpos.cursor_col;
    if (page_row>=Lines||cursor_row>=Lines)
       page_row=cursor_row=0;
    if (page_row==Lines-1) {
      /* so that it will not be thinked as a empty file */
      page_row=Lines-page_len+1;
      if (page_row<0) page_row=0;
    }
  }
#if	0
  if (file_no==1 && !batch && !mult_open) {
    if (RestWinSize) setWinSize(fpos.wrow,fpos.wcol);
    if (RestWinPos) setWinPos(fpos.wx,fpos.wy);
    fontidx=fpos.fntidx-1;
    K_SwitchFont();
  }
#endif
  save_win_context(active_win);
}
ll1:
set_lang_type(fname);
save_edstate();
#if	0
if (c_comment)
	scan_comment(0,Lines);
#endif
redraw_twin();

if (file_no > 1 && !strcmp(edstate[0].fname,"#noname") && edstate[0].Lines==1 &&
!edstate[0].edbf[0].len) {
	close_file_id(0);
}
return 0;
}

void goto_file(int f_no)
{
if (file_no<2) return;
save_edstate();
cur_file=f_no;
restore_edstate();
disp_edstate();
}

static void edit_new_file()
{
int i;
char *s;

if (extend_edstate()) return;

hide_cursor_w();
s=return_item(0);
if(!s[0]) {
	K_FileBrowser();
	return;
}
save_edstate();
s=decode_dir(s);

if (strchr(s,'?')||strchr(s,'*')) {
  char tt[256],ee[256];
  int ll=0;
  FILE *fp;

  strcat(strcpy(tt,"/bin/ls -1 "),s);
  if ((fp=popen(tt,"r"))==NULL) {
    error("cannot execute /bin/ls -1");
    return;
  }
  while (!feof(fp)) {
    int l;
    fgets(ee,sizeof(ee),fp);
    l=strlen(ee);
    if (l && ee[l-1]=='\n') ee[--l]=0;
    if (!l || ee[l-1]=='~' || l>2 && ee[l-1]=='o' && ee[l-2]=='.')
      continue;
    for(i=0;i<file_no;i++)
      if (!strcmp(ee,edstate[i].fname)) break;
    if (i<file_no) continue;
    edit_file(ee);
  }
  pclose(fp);
  return;
}

for(i=0;i<file_no;i++)
if (!strcmp(s,edstate[i].fname)) {
	goto_file(i);
	return;
}
if (edit_file(s)) {
	return;
}
disp_win_title();
}


K_ReadPipe(char *cmd)
{
if (extend_edstate()) return 1;

save_edstate();
bzero(&edstate[file_no],sizeof(edstate[0]));
page_row=page_col=cursor_row=cursor_col=cur_y=cur_x=f_modified=0;
/* insert_mode=1; */
cur_file=file_no;
#if	0
strcpy(fname,cmd);
#endif
read_file(cmd,1);
save_win_context(active_win);
file_no++;
redraw_twin();
disp_win_title();
return 0;
}

static int f_xgets_handle;

void read_file_init()
{
f_xgets_handle=xgets_handle();
add_edln(f_xgets_handle,0,6,20,10,26);
add_action(f_xgets_handle,edit_new_file);
}

void enable_read_file()
{
input_handler=1;
save_win_context(active_win);
inactive_cursor();
box(18,8,30,5,7,8,"Edit file");
init_xgets(f_xgets_handle);
}


void del_tmp_bak(char *ffname)
{
char tt[128];
strcat(strcpy(tt,ffname),"#auto");
unlink(tt);
}


void close_file_id(int c_file, int exit_ifempty)
{
int row,i,Lines0=edstate[c_file].Lines;
LIN *edbf0=edstate[c_file].edbf;
char ttt[256];
FILE *fp;
int newposf=0;
char *fn=edstate[c_file].fname;

if (!nosave && Lines) {
	if ((fp=fopen(tedfilepos,"r+"))==NULL) {
new_one:
		if ((fp=fopen(tedfilepos,"w"))==NULL) {
			error("Check file permission of .tedfilepos");
		}
		newposf=1;
	} else {
		struct stat st;
		fstat(fileno(fp),&st);
		if (st.st_size%sizeof(FPOS))
			goto new_one;
	}
	if (fp) {
		FPOS fpos;
		int idx=0,match=0;
		extern int fontidx;

		bzero(&fpos,sizeof(FPOS));
		if (!newposf)
		while (!feof(fp)) {
			fread(&fpos,1,sizeof(fpos),fp);
			if (!strcmp(fpos.fname,fn)) {
				match=1;
				break;
			}
		}
		if (match) fseek(fp,- sizeof(FPOS),SEEK_CUR);
		strcpy(fpos.fname,fn);
		save_win_context(active_win);
		fpos.page_row=edwin[active_win].pg_pos[c_file].page_row;
		fpos.cursor_row=edwin[active_win].pg_pos[c_file].cursor_row;
		fpos.cursor_col=edwin[active_win].pg_pos[c_file].cursor_col;
		fpos.wrow=XW_ROW;
		fpos.wcol=XW_COL;
		fpos.fntidx=fontidx;
		getWinXY(&fpos.wx,&fpos.wy);
		endian_conv((char *)&fpos.page_row);
		endian_conv((char *)&fpos.cursor_row);
		endian_conv((char *)&fpos.cursor_col);
		endian_conv((char *)&fpos.wrow);
		endian_conv((char *)&fpos.wcol);
		endian_conv((char *)&fpos.wx);
		endian_conv((char *)&fpos.wy);
		endian_conv((char *)&fpos.fntidx);
		fwrite(&fpos,sizeof(FPOS),1,fp);
		fclose(fp);
	}
}

adj_tag_fno(c_file);
adj_undo_owner(c_file);
del_tmp_bak(fn);
if (c_file==mark_holder) K_Unmark();
else
if (c_file<mark_holder) mark_holder--;
for(row=0;row<Lines0;row++)
	if (edbf0[row].cstr) {
		free(edbf0[row].cstr);
	}
free(edbf0);
for(row=c_file;row<file_no-1;row++)
	edstate[row]=edstate[row+1];
file_no--;
if (!file_no) {
	del_undo_file();
	if (exit_ifempty) exit(0);
	else return;
}
for(i=0;i<4;i++) {
	if (edwin[i].cur_file>=c_file) edwin[i].cur_file--;
	if (edwin[i].cur_file>=file_no) edwin[i].cur_file=file_no-1;
	if (edwin[i].cur_file<0) edwin[i].cur_file=0;
	for(row=c_file;row<file_no;row++)
		edwin[i].pg_pos[row]=edwin[i].pg_pos[row+1];
}
if (c_file < hot_file) hot_file--;
if (c_file==hot_file) hot_file=-1;
if (cur_file>=file_no) cur_file=file_no-1;
edwin[active_win].cur_file=cur_file;
restore_edstate();
#if	0
strcpy(fname,edstate[cur_file].fname);
#endif
set_lang_type(fname);
redraw_twin();
disp_win_title();
}

void close_file()
{
save_edstate();
close_file_id(cur_file,1);
}

void close_file_noexit()
{
close_file_id(cur_file,0);
}

typedef struct {
	char fname[128];
	time_t atime,mtime;
	int row,size;
}  FA;


qcmpfa(FA *a, FA *b)
{
if (a->atime > b->atime) return -1;
if (a->atime < b->atime) return 1;
return 0;
}

K_VisitedFile(char *ofile)
{
FILE *fp;
FPOS fpos;
struct stat st;
int cou,i;
FA fa[1024];

if ((fp=fopen(tedfilepos,"r"))==NULL) {
	return;
}

cou=0;
while (!feof(fp))	{
	if (fread(&fpos,sizeof(FPOS),1,fp)<=0) break;
	endian_conv((char *)&fpos.cursor_row);
	if (stat(fpos.fname,&st)) continue;
	strcpy(fa[cou].fname, fpos.fname);
	fa[cou].size=st.st_size;
	fa[cou].atime=st.st_atime;
	fa[cou].mtime=st.st_mtime;
	fa[cou].row=fpos.cursor_row;
	cou++;
}
close(fp);
qsort(fa,cou,sizeof(FA),qcmpfa);
if ((fp=fopen(ofile,"w"))==NULL) {
	message(4,"cannot create %s",ofile);
	return;
}
for(i=0;i<cou;i++) {
	struct tm atm,mtm;
	atm=*(localtime(&fa[i].atime));
	mtm=*(localtime(&fa[i].mtime));

	fprintf(fp,"%-20s:%4d: SZ:%-6d Ref:%02d:%02d %02d/%02d/%d  Mod:%02d:%02d %02d/%02d/%d\n",
	fa[i].fname,fa[i].row+1,fa[i].size,
	atm.tm_hour,atm.tm_min,atm.tm_mon+1,atm.tm_mday,atm.tm_year+1900,
	mtm.tm_hour,mtm.tm_min,mtm.tm_mon+1,mtm.tm_mday,mtm.tm_year+1900);
}
fflush(fp);
close(fp);
}

K_PrevFile()
{
int c_file;
c_file=cur_file-1;
if (c_file<0) c_file=file_no-1;
goto_file(c_file);
return 0;
}

K_NextFile()
{
goto_file((cur_file+1)%file_no);
return 0;
}

static char ttfname[80];

static void confirm_write_file(char yn)
{
if (yn=='Y') {
	strcpy(fname,ttfname);
	save_edstate();
	disp_edstate();
	K_SaveFile();
}
redraw_twin();
}

static void write_to_file()
{
FILE *fp;
char *s;
s=decode_dir(return_item(0));
strcpy(ttfname,s);
redraw_twin();
if (fp=fopen(ttfname,"r")) {
	fclose(fp);
	message(4,"File Exists. Overwrite (Y/N):   ");
	setup_getch(30,MROW-1,"YNyn",'N', confirm_write_file);
	return;
}
if (!(fp=fopen(ttfname,"w"))) {
	message(4,"Cannot write to file %s",ttfname);
	bell();
	return;
}
fclose(fp);
strcpy(fname,ttfname);
save_edstate();
disp_edstate();
K_SaveFile();
}
static int wf_xgets_handle;

void write_file_init()
{
wf_xgets_handle=xgets_handle();
add_edln(wf_xgets_handle,0,6,20,10,20);
add_action(wf_xgets_handle,write_to_file);
}

void enable_write_file()
{
	input_handler=1;
	save_win_context(active_win);
	inactive_cursor();
	box(18,8,24,5,7,8,"Write to file");
	init_xgets(wf_xgets_handle);
}

void write_line_blk_file()
{
FILE *fp;
int i;

if (!(fp=fopen(ttfname,"w"))) {
	message(4,"Cannot write to file %s",ttfname);
	bell();
	return;
}

for(i=mline_begin;i<=mline_end;i++) {
	if (edbf[i].len) {
		fwrite(edbf[i].cstr,1,edbf[i].len,fp);
	}
	if (i!=mline_end) fwrite("\n",1,1,fp);
}

fclose(fp);
save_edstate();
disp_edstate();
}

void confirm_write_file_blk(char yn)
{
if (mtoupper(yn)=='Y') {
	write_line_blk_file();
	return;
}
save_edstate();
disp_edstate();
}

static void write_blk_to_file()
{
FILE *fp;
char *s;

s=decode_dir(return_item(0));
strcpy(ttfname,s);
redraw_twin();
if (fp=fopen(ttfname,"r")) {
	fclose(fp);
	message(4,"File Exists. Overwrite (Y/N):   ");
	setup_getch(30,MROW-1,"YNyn",'N', confirm_write_file_blk);
	return;
}
write_line_blk_file();
}


static int wr_blk_xgets_handle;

void write_blk_init()
{
wr_blk_xgets_handle=xgets_handle();
add_edln(wr_blk_xgets_handle,0,6,20,10,20);
add_action(wr_blk_xgets_handle,write_blk_to_file);
}

void K_WriteBlk()
{
if (mline_begin<0) {
	message(4,"Block is undefined");
	return;
}
input_handler=1;
save_win_context(active_win);
inactive_cursor();
box(18,8,24,5,7,8,"Write block to file");
init_xgets(wr_blk_xgets_handle);
}


K_SaveIfModified()
{
if (f_modified) K_SaveFile();
return 0;
}

K_InsFile(char *ins_file)
{
FILE *fp;
char bf[1024],*str;
int len,lc;
if ((fp=fopen(ins_file,"r"))==NULL) {
	message(4,"Cannot open include file %s", ins_file);
	return;
}
lc=0;
while(!feof(fp)) {
	len=mfgets(bf,sizeof(bf),fp);
	if (!len) break;
	lc++;
}
hide_cursor_w();
ins_lines(cursor_row,mline_end=lc);
fseek(fp,0,0);
mline_begin=lc=cursor_row;
while(!feof(fp)) {
	len=mfgets(bf,sizeof(bf),fp);
	if (!len) break;
	if (bf[len-1]=='\n') bf[--len]=0;
	edbf[lc].len=len;
	if (!len) {
		edbf[lc].cstr=0;
		lc++;
		continue;
	}
	if ((str=mmalloc(len,"InsFile"))==NULL) return;
	memcpy(str,bf,len);
	edbf[lc].cstr=str;
	lc++;
}
fclose(fp);
s_fmod();
mline_end+=mline_begin-1;
mblk_row0=-1;
disp_page();
show_cursor_w();
}

K_InsPipe(char *pipecmd)
{
int fd,pid;
char *bf,*str;
int len,olines,lc;
LIN *lin;

if (pipe(fds)<0) {
	error("Cannot create pipe");
	return;
}
if (!(pid=fork())) {
	dup2(fds[1],1);
	dup2(fds[1],2);
	close(fds[0]);
	len=strlen(pipecmd);
	if (strchr(pipecmd,' '))
		execl("/bin/sh","sh","-c",pipecmd,NULL);
	else    execlp(pipecmd,pipecmd,NULL);
} else
if (pid<0) {
	error("Cannot fork process");
	return;
}
close(fds[1]);
fd=fds[0];
init_fdbuf(sizeof(fdbf));
hide_cursor_w();
olines=Lines;
for(;;) {
	len=fdgets(&bf,fd);
	if (!len) break;
	if (bf[len-1]=='\n') bf[--len]=0;
	edbf[Lines].len=len;
	if (!len) {
		edbf[Lines].cstr=0;
		incr_line();
		continue;
	}
	edbf[Lines].cstr=bf;
	incr_line();
}
close(fd);
lc=Lines-olines;
if (!lc) return 0;
if ((lin=mmalloc(lc*sizeof(LIN),"K_InsPipe"))==NULL) return;
memcpy(lin,&edbf[olines],lc*sizeof(LIN));
mmemmove(&edbf[cursor_row+lc],&edbf[cursor_row],
	(olines-cursor_row)*sizeof(LIN));
memcpy(&edbf[cursor_row],lin,lc*sizeof(LIN));
free(lin);
mline_begin=cursor_row;
mline_end=mline_begin+lc-1;
mblk_row0=-1;
disp_page();
s_fmod();
show_cursor_w();
return 0;
}

static void start_inc_file()
{
K_InsFile(decode_dir(return_item(0)));
}

static int incf_xgets_handle;

void include_file_init()
{
incf_xgets_handle=xgets_handle();
add_edln(incf_xgets_handle,0,6,20,10,20);
add_action(incf_xgets_handle,start_inc_file);
}

void K_IncFile()
{
input_handler=1;
save_win_context(active_win);
inactive_cursor();
box(18,8,24,5,7,8,"Insert File");
init_xgets(incf_xgets_handle);
}

lookup_tag(char *tag)
{
FILE *fp;
char bf[1024],*s, tt[80];
int len,row,i;

strcat(strcpy(tt,"ref "),tag);
if ((fp=popen(tt,"r"))==NULL) {
	error("Cannot execute ref");
	return -1;
}
if (!fgets(bf,sizeof(bf),fp)) return -1;
len=strlen(bf);
pclose(fp);

if (!len) return -1;
if (bf[len-1]=='\n') bf[--len]=0;
if (!len) return -1;
bf[--len]=0;
s=bf+len-1;
while (*s!=' ' && s> bf) s--;
s++;
row=atoi(s)-1;
s=bf;
while (*s!=',') s++;
*s=0;
for(i=0;i<file_no;i++) {
	if (!strcmp(edstate[i].fname,bf)) break;
}
hide_cursor_w();
if (i<file_no) {
	save_edstate();
	cur_file=i;
	restore_edstate();
#if	0
	page_row=row-(page_len>>1);
#else
	page_row=row;
#endif
	page_col=0;
	cursor_row=row;
	cursor_col=0;
	if (page_row<0) page_row=0;
	disp_page();
	disp_edstate();
	show_cursor_w();
	disp_win_title();
	return 0;
}
save_edstate();
#if	0
strcpy(fname,bf);
#endif
edit_file(bf);
#if	0
page_row=row-(page_len>>1);
#else
page_row=row;
#endif
if (page_row<0) page_row=0;
cursor_row=row;
cursor_col=page_col=0;
set_lang_type(fname);
disp_page();
disp_edstate();
show_cursor_w();
disp_win_title();
return 0;
}

static void start_LookupTag()
{
lookup_tag(return_item(0));
}

static int lookuptag_xgets_handle;

void lookup_tag_init()
{
lookuptag_xgets_handle=xgets_handle();
add_edln(lookuptag_xgets_handle,0,6,20,10,30);
add_action(lookuptag_xgets_handle,start_LookupTag);
}

int K_LookupTag()
{
input_handler=1;
save_win_context(active_win);
inactive_cursor();
box(18,8,34,5,7,8,"Look up C Tag");
init_xgets(lookuptag_xgets_handle);
return 0;
}

K_LookMarkTag()
{
char tt[128];
if (!GetMarkWord(tt)) return;
lookup_tag(tt);
}

static void start_ExecCmd()
{
K_ReadPipe(return_item(0));
}

static int ExecCmd_xgets_handle;

void ExecCMd_init()
{
ExecCmd_xgets_handle=xgets_handle();
add_edln(ExecCmd_xgets_handle,0,6,20,10,36);
add_action(ExecCmd_xgets_handle,start_ExecCmd);
}

K_ExecCmd()
{
input_handler=1;
save_win_context(active_win);
inactive_cursor();
box(18,8,40,5,7,8,"Execute Command");
init_xgets(ExecCmd_xgets_handle);
}

K_ExeCmdFnameExt(char *cmd, char*ext)
{
char tt[128],uu[128],*vv;
strcpy(uu,fname);
if (!(vv=strchr(uu,'.'))) return;
*vv=0;
strcat(strcat(strcat(strcpy(tt,cmd)," "),uu),ext);
K_ReadPipe(tt);
}

void save_tmp_bak()
{
char tt[128];
if (need_autosave && f_modified) {
	strcat(strcpy(tt,fname),"#auto");
	save_filename(tt,0);
	message(5,"backup %s %d line ",tt,Lines);
  disp_cursor();
	need_autosave=0;
}
}

K_RaiseOrLoad(char *fn)
{
int i;
for(i=0;i<file_no;i++)
	if (!strcmp(edstate[i].fname,fn)) {
		goto_file(i);
		return 0;
	}
K_EditFName(fn);
return 0;
}

K_Reload()
{
int row;
for(row=0;row<Lines;row++)
	if (edbf[row].cstr) {
		free(edbf[row].cstr);
	}
free(edbf);
hide_cursor_w();
read_file(fname,0);
if (cursor_row>=Lines) cursor_row=Lines-1;
if (cursor_col>edbf[cursor_row].len) cursor_col=edbf[cursor_row].len;
disp_page();
show_cursor_w();
}

#include <pwd.h>
void get_hostname()
{
gethostname(hostname,sizeof(hostname));
strcpy(user_name, (getpwuid(getuid()))->pw_name);
}

switch_fullname(char *s)
{
int i;
char ff[128];

for(i=0;i<file_no;i++) {
	strcat(strcat(strcpy(ff,edstate[i].fdir),"/"),edstate[i].fname);
	if (!strcmp(s,ff)) {
		goto_file(i);
		return 'Y';
	}
}
return 0;
}
