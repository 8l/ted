/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include <stdio.h>
#include "ted.h"

#define INS_CHR         (1)
#define REP_CHR		(2)
#define DEL_STRING	(3)
#define INS_STR		(4)
#define REP_STR		(5)
#define DEL_STR		(6)
#define INS_LINE	(7)
#define DEL_LINE	(8)
#define INS_MARK_LINES	(9)
#define DEL_LINES	(10)
#define REP_LINES	(11)
#define SPLIT_LINE	(12)
#define JOIN_LINE	(13)
#define REP_MARK_LINES	(14)

#define MAX_UN (100)
static int in_file_cou=0;
static int file_ofs=0,bf_ofs=0;
#define MAX_BF (1024)
static char ubf[MAX_BF];
static int bf_un_count,file_un_count;
#define M_file_un_count (200)
int undo_idx=-1,idx_base=-1;
static int undo_owner;

struct {
int u_ofs,u_len,crow,ccol,crow1,ccol1,cmd;
} sdo[MAX_UN];

static FILE *ufw,*tfw;

static char *unfname[2];
static int a_fname;

static int buf_type=0;
static int buflen,un_col,un_row;
static char strbuf[512];

void init_undo_fname()
{
	unfname[0]=tempnam("/tmp","Ted");
	unfname[1]=tempnam("/tmp","Ted");
}

void del_undo_file()
{
	unlink(unfname[0]);
	unlink(unfname[1]);
}

static void init_ufw()
{
if (!ufw)
	if ((ufw=fopen(unfname[a_fname],"w+"))==NULL)
	p_err("init_ufw:err create file %s %d",unfname[a_fname],a_fname);
}

void clear_undo()
{
	if (ufw) {
		close(ufw);
		ufw=0;
		unlink(unfname[a_fname]);
	}
	in_file_cou=0;
	file_ofs=0; bf_ofs=0;
	bf_un_count=file_un_count=0;
	undo_idx=idx_base=-1;
	buf_type=0;
}

/* only undo in u_ofs is saved */
static void pack_file()
{
int i;
char *tbf=0;

	init_ufw();
	if (file_un_count > M_file_un_count) {
		if ((tfw=fopen(unfname[a_fname^1],"w"))==NULL) {
			error("cannot create file",unfname[a_fname^1]);
			return;
		}
		for(i=0;i<in_file_cou;i++) {
			int len=sdo[i].u_len;
			if (!len) continue;
			if ((tbf=(char *)mrealloc(tbf,len,"pack_file"))==NULL) {
				error("pack_file:malloc:%d\n",len);
				return;
			}
			fseek(ufw,sdo[i].u_ofs,SEEK_SET);
			fread(tbf,1,len,ufw);
			sdo[i].u_ofs=ftell(tfw);
			fwrite(tbf,1,len,tfw);
		}
		fclose(ufw);
		ufw=tfw;
		a_fname^=1;
		file_un_count=in_file_cou;
		if (tbf) free(tbf);
	}
}

static void cp_head(int idx,int crow, int ccol, int crow1,int ccol1,char cmd)
{
sdo[idx].crow=crow;
sdo[idx].ccol=ccol;
sdo[idx].crow1=crow1;
sdo[idx].ccol1=ccol1;
sdo[idx].cmd=cmd;
}

static void make_bf_room()
{
int i;
if (bf_un_count<MAX_UN) return;
for(i=1;i<MAX_UN;i++)
	sdo[i-1]=sdo[i];
bf_un_count--;
if (undo_idx >=0) undo_idx--;
if (idx_base >=0) idx_base--;
if (in_file_cou>0) in_file_cou--;
}

void chk_undo_owner()
{
if (undo_owner!=cur_file) {
	clear_undo();
	undo_owner=cur_file;
}
}

void adj_undo_owner(int f)
{
if (f==undo_owner)
	undo_owner=-1;
else
if (f < undo_owner)
	undo_owner--;
}

static void save_cmd(int cmd, char *str, int len, int crow, int ccol,
int crow1,int ccol1)
{
int datalen=len;
int i,ofs;
#if     0
printf("bf_ofs:%d in_file_cou:%d bf_un_count:%d\n",
bf_ofs,in_file_cou,bf_un_count);
#endif

chk_undo_owner();
if (bf_ofs+datalen >= MAX_BF) {
	pack_file();
	fseek(ufw,0,SEEK_END);
	for(i=in_file_cou;i<bf_un_count;i++) {  /* flush buffer */
		ofs=ftell(ufw);
		if (sdo[i].u_len)
			fwrite(&ubf[sdo[i].u_ofs],1,sdo[i].u_len,ufw);
		sdo[i].u_ofs=ofs;
		file_un_count++;
	}
	make_bf_room();
	if (datalen <= MAX_BF) {
		sdo[bf_un_count].u_ofs=0;
		sdo[bf_un_count].u_len=datalen;
		if (datalen) memcpy(ubf,str,datalen);
		bf_ofs=datalen;
		in_file_cou=bf_un_count;
	} else {
		sdo[bf_un_count].u_len=datalen;
		sdo[bf_un_count].u_ofs=ftell(ufw);
		fwrite(str,1,datalen,ufw);
		in_file_cou=bf_un_count+1;
	}
#if     0
	fflush(ufw);
#endif
	cp_head(bf_un_count++,crow,ccol,crow1,ccol1,cmd);
#if     0
	printf("#bf_ofs:%d in_file_cou:%d bf_un_count:%d\n",
	bf_ofs,in_file_cou,bf_un_count);
#endif
	return;
}
make_bf_room();
if (datalen) {
	memcpy(&ubf[bf_ofs],str,datalen);
	sdo[bf_un_count].u_ofs=bf_ofs;
	bf_ofs+=datalen;
}
sdo[bf_un_count].u_len=datalen;
cp_head(bf_un_count++,crow,ccol,crow1,ccol1,cmd);
#if     0
printf("@bf_ofs:%d in_file_cou:%d bf_un_count:%d\n",
	bf_ofs,in_file_cou,bf_un_count);
#endif
}


static void flush_undo_str(int rst)
{
if (buf_type==1) {
	save_cmd(DEL_STR,(char *)&buflen,sizeof(int),un_row,un_col,0,0);
	if (rst) idx_base=undo_idx=bf_un_count-1;
} else
if (buf_type==2) {
	save_cmd(INS_STR,strbuf,buflen,un_row,un_col,0,0);
	if (rst) idx_base=undo_idx=bf_un_count-1;
}
buf_type=0;
}

void u_del_str(char *s, int len, int rst)
{
chk_undo_owner();
if (!rst) {
	save_cmd(DEL_STR,(char *)&len,sizeof(int),cursor_row,cursor_col,0,0);
	return;
}
if (s && strchr(s,'\n')) {
	flush_undo_str(rst);
	save_cmd(DEL_STR,(char *)&len,sizeof(int),cursor_row,cursor_col,0,0);
	idx_base=undo_idx=bf_un_count-1;
	return;
}
if (buf_type==1 && cursor_row==un_row && un_col+buflen==cursor_col) {
	buflen+=len;
	if (rst) idx_base=undo_idx=bf_un_count-1;
	return;
}
flush_undo_str(rst);
un_row=cursor_row;
un_col=cursor_col;
buflen=len;
buf_type=1;
if (rst) idx_base=undo_idx=bf_un_count-1;
}


void u_ins_str(int len, int rst)
{
char *ss=&edbf[cursor_row].cstr[cursor_col];

chk_undo_owner();
if (!rst) {
	save_cmd(INS_STR,ss,len,cursor_row,cursor_col,0,0);
	return;
}
if (buf_type==2 && buflen+len<sizeof(strbuf)) {
	if (un_row==cursor_row) {
		if (un_col==cursor_col) {
			memcpy(strbuf+buflen,ss,len);
			buflen+=len;
			return;
		} else
		if (cursor_col+len==un_col) {
			mmemmove(strbuf+len,strbuf,buflen);
			memcpy(strbuf,ss,len);
			buflen+=len;
			un_col=cursor_col;
			return;
		}
	}
}
flush_undo_str(rst);
un_row=cursor_row;
un_col=cursor_col;
buflen=len;
buf_type=2;
memcpy(strbuf,ss,len);
}

void u_Ins_str(int ilen)
{
char *uu,*tt;
int i,row,col,len;

chk_undo_owner();
if ((uu=mmalloc(ilen+1,"u_Ins_str"))==NULL) {
	printf("ilen:%d\n",ilen);
	return;
}
row=cursor_row;
col=cursor_col;
tt=edbf[row].cstr;
len=edbf[row].len;
row++;
i=0;
while(i<ilen) {
	if (col>=len) {
		uu[i++]='\n';
		col=0;
		len=edbf[row].len;
		tt=edbf[row++].cstr;
	}
	else
	uu[i++]=tt[col++];
}
uu[ilen]=0;
save_cmd(INS_STR,uu,ilen,cursor_row,cursor_col,0,0);
free(uu);
return;
}

void u_ins_line(int rst)
{
int len=edbf[cursor_row].len;
flush_undo_str(rst);
save_cmd(INS_LINE,edbf[cursor_row].cstr,len,cursor_row,cursor_col,0,0);
if (rst) idx_base=undo_idx=bf_un_count-1;
}

void u_del_line(int rst)
{
save_cmd(DEL_LINE,0,0,cursor_row,cursor_col,0,0);
flush_undo_str(rst);
if (rst) idx_base=undo_idx=bf_un_count-1;
}

void u_ins_mark_lines(int rst)
{
char *tb,*tt;
int i,len=0;
flush_undo_str(rst);
for(i=mline_begin;i<=mline_end;i++) len+=edbf[i].len+1;
if (len<=0) return;
if ((tb=mmalloc(len,"u_ins_mark_lines"))==NULL) return;
tt=tb;
for(i=mline_begin;i<=mline_end;i++) {
	if (edbf[i].cstr) {
		memcpy(tb,edbf[i].cstr,edbf[i].len);
		tb+=edbf[i].len;
	}
	*(tb++)='\n';
}
save_cmd(INS_MARK_LINES,tt,len,mline_begin,mline_end,0,0);
if (rst) idx_base=undo_idx=bf_un_count-1;
free(tt);
}

void u_rep_mark_lines(int row0,int col0,int row1,int col1,int rst)
{
char *tb,*tt;
int i,len=0;

flush_undo_str(rst);
for(i=row0;i<=row1;i++) len+=edbf[i].len+1;
if (len<=0) return;
if ((tb=mmalloc(len,"u_ins_mark_block"))==NULL) return;
tt=tb;
for(i=row0;i<=row1;i++) {
	if (edbf[i].cstr) {
		memcpy(tb,edbf[i].cstr,edbf[i].len);
		tb+=edbf[i].len;
	}
	*(tb++)='\n';
}
save_cmd(REP_MARK_LINES,tt,len,row0,col0,row1,col1);
if (rst) idx_base=undo_idx=bf_un_count-1;
free(tt);
}

void u_rep_lines(int row0,int row1,int rst)
{
char *tb,*tt;
int i,len=0;

flush_undo_str(rst);
for(i=row0;i<=row1;i++) len+=edbf[i].len+1;
if (len<=0) return;
if ((tb=mmalloc(len,"u_ins_mark_block"))==NULL) return;
tt=tb;
for(i=row0;i<=row1;i++) {
	if (edbf[i].cstr) {
		memcpy(tb,edbf[i].cstr,edbf[i].len);
		tb+=edbf[i].len;
	}
	*(tb++)='\n';
}
save_cmd(REP_LINES,tt,len,row0,0,row1,0);
if (rst) idx_base=undo_idx=bf_un_count-1;
free(tt);
}

void u_del_lines(int frm, int to,int rst)
{
flush_undo_str(rst);
save_cmd(DEL_LINES,0,0,frm,0,to,0);
if (rst) idx_base=undo_idx=bf_un_count-1;
}

void u_split_line(int col,int rst)
{
flush_undo_str(rst);
save_cmd(SPLIT_LINE,0,0,cursor_row,col,0,0);
if (rst) idx_base=undo_idx=bf_un_count-1;
}

void u_join_line(int rst)
{
flush_undo_str(rst);
save_cmd(JOIN_LINE,0,0,cursor_row,cursor_col,0,0);
if (rst) idx_base=undo_idx=bf_un_count-1;
}

void exe_un_re(int idx, int unre)
{
char *ss=0;
int crow,ccol,len;
int slen;

if (idx < 0) return;
len=sdo[idx].u_len;
if (len) {
	if ((ss=mmalloc(len,"undo"))==NULL) return;
	if (idx>=in_file_cou) {
		memcpy(ss,ubf+sdo[idx].u_ofs,len);
	} else {
		fseek(ufw,sdo[idx].u_ofs,SEEK_SET);
		fread(ss,1,len,ufw);
	}
}
switch (sdo[idx].cmd) {
	case DEL_STR:
		hide_cursor_w();
		cursor_row=sdo[idx].crow;
		cursor_col=sdo[idx].ccol;
		memcpy(&slen,ss,sizeof(int));
		if (unre) u_Ins_str(slen);
		del_Str(slen);
		break;
	case INS_STR:
		hide_cursor_w();
		cursor_row=sdo[idx].crow;
		cursor_col=sdo[idx].ccol;
		if (unre) u_del_str(0,len,0);
		put_str(ss,len);
		break;
	case INS_LINE:
		hide_cursor_w();
		cursor_row=sdo[idx].crow;
		cursor_col=sdo[idx].ccol;
		if (unre) u_del_line(0);
		ins_line(ss,len);
		show_cursor_w();
		break;
	case DEL_LINE:
		hide_cursor_w();
		cursor_row=sdo[idx].crow;
		cursor_col=sdo[idx].ccol;
		if (unre) u_ins_line(0);
		del_line();
		break;
	case DEL_LINES:
		hide_cursor_w();
		cursor_row=mline_begin=sdo[idx].crow;
		mline_end=sdo[idx].crow1;
		if (unre) u_ins_mark_lines(0);
		del_mlines();
		break;
	case INS_MARK_LINES:
		mline_begin=mline_end=-1;
		hide_cursor_w();
		cursor_row=sdo[idx].crow;
		cursor_col=0;
		put_str(ss,len);
		mline_begin=sdo[idx].crow;
		cursor_row=mline_end=sdo[idx].ccol;
		if (unre) u_del_lines(mline_begin,mline_end,0);
		pos_cur();
		disp_page();
		show_cursor_w();
		break;
	case REP_MARK_LINES:
		hide_cursor_w();
		mblk_row0=sdo[idx].crow;
		mblk_row1=sdo[idx].crow1;
		mblk_col0=sdo[idx].ccol;
		mblk_col1=sdo[idx].ccol1;
		if (unre) u_rep_mark_lines(mblk_row0,-1,mblk_row1,-1,0);
		rep_lines(mblk_row0,ss,len);
		if (mblk_col0<0) mblk_row0=mblk_row1=-1;
		disp_page();
		show_cursor_w();
		break;
	case REP_LINES:
		hide_cursor_w();
		if (unre) u_rep_lines(sdo[idx].crow,sdo[idx].crow1,0);
		rep_lines(sdo[idx].crow,ss,len);
		disp_page();
		show_cursor_w();
		break;
	case SPLIT_LINE:
		cursor_row=sdo[idx].crow;
		cursor_col=sdo[idx].ccol;
		if (unre) u_join_line(0);
		split_line();
		break;
	case JOIN_LINE:
		hide_cursor_w();
		cursor_row=sdo[idx].crow;
		cursor_col=sdo[idx].ccol;
		if (unre) u_split_line(cursor_col,0);
		join_line();
		show_cursor_w();
		break;
}
if (ss) free(ss);
}

K_Undo()
{
	if (undo_owner!=cur_file) return;
	s_fmod();
	flush_undo_str(1);
	exe_un_re(undo_idx,1);
	if (undo_idx>=0) undo_idx--;
	return 0;
}

K_Redo()
{
	if (undo_owner!=cur_file)
	  return 0;
	if (bf_un_count-1<=idx_base)
	  return 0;
	exe_un_re(bf_un_count-1,0);
	if (bf_un_count-1>=in_file_cou) bf_ofs=sdo[bf_un_count-1].u_ofs;
	bf_un_count--;
	if (undo_idx<idx_base) undo_idx++;
	return 0;
}
