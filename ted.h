/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#define M_ROW (24)
#define M_COL (80)

#define MaxLang (8)
#define MaxKeyWType (6)

extern int page_width,page_len,page_row,page_col;
extern int MROW,MCOL,XW_ROW,XW_COL;
extern int cur_y,cur_x,win_x,win_y;
extern int cursor_row,cursor_col;
extern int cur_tab;
extern int Lines,Alloc_Lines;
extern int mline_begin,mline_end,mesg;
extern int mblk_row0,mblk_col0,mblk_row1,mblk_col1,f_modified,file_modified;
extern int file_no,xgets_enter,insert_mode,batch;
extern char fname[];

typedef struct {
char *cstr;
int len,flag;
} LIN;


#define HIST_N (10)
#define MAX_EDLN_STR (100)
typedef struct {
int color,ptr;
int x,y,width;
int hidx,hino;
char hist[HIST_N+1][MAX_EDLN_STR+1];
} EDLN;

typedef struct {
int Lines,Alloc_Lines;
int f_modified, cmapno;
LIN *edbf;
char fname[128],fdir[128];
int nosave,readonly;
} EDSTATE;

extern EDSTATE *edstate;
extern int cur_file, mark_holder,cmapno, readonly;

typedef struct {
int cur_y,cur_x;
int cursor_row,cursor_col;
int page_row,page_col;
} PAGE_POS;

extern LIN  *edbf;

typedef struct {
int x,y,MROW,MCOL;
int cur_file, mesg;
PAGE_POS pg_pos[256];
int insert_mode, Indent;
} EDWIN;

typedef struct {
int page_row,page_col,cursor_row,cursor_col;
int fno;
} TAG;

#define MAXTAG (32)


typedef struct {
int x,y;
char *str;
} XYstr;

extern EDWIN edwin[];
extern int active_win,cwin_len;

#define Max(a,b) (a>b?a:b)
#define Min(a,b) (a<b?a:b)

extern void *mmalloc(int, char *);
extern void *mrealloc(void *, int, char *);
extern int input_handler;
extern int hot_file;
extern char sep_chars[],*tedrcfname;
extern char *return_item();
extern char *decode_dir();
extern int tab_width,Indent,nosave;
#ifndef NULL
#define NULL (0)
#endif
#ifndef SEEK_SET
#define SEEK_SET (0)
#define SEEK_END (2)
#endif

typedef struct {
	char fname[128];
	int page_row,cursor_row,cursor_col;
	int wx,wy,wrow,wcol;
	int fntidx;
} FPOS;
