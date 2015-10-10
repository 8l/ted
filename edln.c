/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <dirent.h>
#include <string.h>
#include "ted.h"


#define MAX_ITEM (3)

int xgets_enter;
int edln_esc=0;

static int c_idx,c_len, c_x, c_y, c_width,c_color, c_noitem, c_hino, c_ptr;
static char c_ed[MAX_EDLN_STR+1];
static int active_item;
static int hist_idx;
static char first_t[MAX_ITEM];

static int nogets;
int active_gets;

struct {
	int noitem;
	EDLN edln[MAX_ITEM];
	void (*action)();
} xgets[11];

int xgets_num=0;

static DIR *dir;


int xgets_handle()
{ return xgets_num++;
}

void add_edln(getno, edno, color, x, y, width)
int getno, edno, color, x, y, width;
{
int i,j;
	i=getno;
	j=edno;
	xgets[i].edln[j].color=color;
	xgets[i].edln[j].x=x;
	xgets[i].edln[j].y=y;
	xgets[i].edln[j].width=width;
	if (i>=nogets) nogets=i+1;
	if (j>=xgets[i].noitem) xgets[i].noitem=j+1;
}

void add_action(int getno,void (*action)())
{
	xgets[getno].action=action;
}

static void disp_c_ed()
{
int i,len;
char tt[512];

if (c_ptr+c_width<=c_idx) {
	c_ptr=c_len-c_width+1;
}
gotoxy(c_x,c_y);
set_att(c_color);
c_ed[c_len]=0;
memcpy(tt,c_ed+c_ptr,len=Min(c_width,c_len-c_ptr));
tt[len]=0;
xputstrd(tt);
for(i=c_len-c_ptr;i<c_width;i++)
	xputstrd(" ");
draw_line(c_y);
gotoxy(c_x+c_idx-c_ptr,c_y);
show_cursor_r();
}


static void disp_c_ed0()
{
int i,len;
char tt[512];

gotoxy(c_x,c_y);
set_att(9);
c_ed[c_len]=0;
memcpy(tt,c_ed+c_ptr,len=Min(c_width,c_len-c_ptr));
tt[len]=0;
xputstrd(tt);
set_att(c_color);
for(i=c_len-c_ptr;i<c_width;i++)
	xputstrd(" ");
draw_line(c_y);
}

static void init_c_ed(int hno)
{
int ag=active_gets;
int ai=active_item;
if (hno<0) hno=xgets[ag].edln[ai].hino-1;
if (hno<0) hno=0;
strcpy(c_ed,xgets[ag].edln[ai].hist[hno]);
c_idx=c_len=strlen(c_ed);
c_width=xgets[ag].edln[ai].width;
c_x=xgets[ag].edln[ai].x;
c_y=xgets[ag].edln[ai].y;
c_ptr=c_len-c_width+1;
if (c_ptr<0) c_ptr=0;
c_color=xgets[ag].edln[ai].color;
c_hino=xgets[ag].edln[ai].hino;
}

void restore_item(int ai)
{
int ag=active_gets;
int hno=xgets[ag].edln[ai].hino;
char ttt[100];
int c_len,c_width,c_x,c_y,i;
if (!hno) hno=1;
c_len=strlen(xgets[ag].edln[ai].hist[hno-1]);
c_width=xgets[ag].edln[ai].width;
c_color=xgets[ag].edln[ai].color;
c_ptr=xgets[ag].edln[ai].ptr;
gotoxy(c_x=xgets[ag].edln[ai].x, c_y=xgets[ag].edln[ai].y);
set_att(c_color);
c_len=Min(c_len,c_width);
memcpy(ttt,xgets[ag].edln[ai].hist[hno-1],c_len);
ttt[c_len]=0;
xputstrd(ttt);
for(i=c_len-c_ptr;i<c_width;i++)
	xputstrd(" ");
draw_line(c_y);
}

void init_xgets(ag)
{
active_gets=ag;
c_noitem=xgets[ag].noitem;
xgets_enter=0;
for(active_item=1;active_item<c_noitem;active_item++) {
	init_c_ed(-1);
	disp_c_ed0();
}
active_item=0;
memset(first_t,1,sizeof(first_t));
init_c_ed(-1);
c_idx=c_len;
if (c_idx > c_width) {
	c_ptr=c_len-c_width+1;
}
disp_c_ed0();
gotoxy(c_x+c_idx-c_ptr,c_y);
show_cursor_r();
}

void put_str_xgets(char *s,int slen)
{
int i, len=Min(slen,MAX_EDLN_STR-1-c_len);
if (s[len-1]<' ') len--;
if (len<=0) {
	bell();
	return;
}
if (first_t[active_item]) {
	first_t[active_item]=0;
	c_ed[0]=0;
	c_ptr=c_idx=c_len=0;
}
#if	0
printf("c_width:%d len:%d c_len:%d\n",c_width,len,c_len);
#endif
for(i=c_len-1;i>=c_idx;i--) c_ed[i+len]=c_ed[i];
memcpy(&c_ed[c_idx],s,len);
c_idx+=len;
c_len+=len;
if (c_ptr+c_width<=c_idx) {
	c_ptr=c_idx-c_width+1;
}
disp_c_ed();
}

static void save_ed()
{
int ag=active_gets;
int ai=active_item;
int hno=xgets[ag].edln[ai].hino;
int i;

c_ed[c_len]=0;
if (hno>=HIST_N) {
	for(i=0;i<HIST_N;i++)
	strcpy(xgets[ag].edln[ai].hist[i],xgets[ag].edln[ai].hist[i+1]);
	hno--;
	xgets[ag].edln[ai].hino=hno;
}
strcpy(xgets[ag].edln[ai].hist[hno],c_ed);
xgets[ag].edln[ai].hino++;
}

void edln_restore()
{
int i;
if (input_handler!=1) return;
show_cursor_w();
inactive_cursor();
for(i=0;i<c_noitem;i++)
if (i!=active_item) {
	restore_item(i);
}
disp_c_ed();
}

static void to_bottom()
{
int ag=active_gets;
int ai=active_item;
hist_idx=xgets[ag].edln[ai].hino;
}

static void clr_first_t()
{
	first_t[active_item]=0;
	disp_c_ed();
	show_cursor_r();
}

static int strqcmp(char **a, char **b)
{
return (strcmp(*a,*b));
}


static char *mpat[128];
static int mcount=0, midx=0;


void process_edln(char *astr, int skey, int state)
{
int ag=active_gets;
int ai=active_item;
int i,alen=strlen(astr);
extern int input_handler, cursor_x, cursor_y,insert_mode_r;
int in_file_comp=0;

c_x=xgets[ag].edln[ai].x;
c_y=xgets[ag].edln[ai].y;

switch (skey) {
	case XK_Up:
		clr_first_t();
		if (hist_idx) hist_idx--;
		init_c_ed(hist_idx);
		disp_c_ed();
		show_cursor_r();
		break;
	case XK_Down:
		clr_first_t();
		if (hist_idx< c_hino) hist_idx++;
		init_c_ed(hist_idx);
		disp_c_ed();
		show_cursor_r();
		break;
	case XK_Left:
		clr_first_t();
		if (c_idx) {
#if	CHINESE
			if (c_idx>=2 && B2nd(cursor_y,cursor_x-1)) {
				c_idx-=2;
				if (c_idx<c_ptr) {
					c_ptr=c_idx;
					disp_c_ed();
				}
				else
				cursor_left(c_color,2);
			} else
#endif	CHINESE
			{
				c_idx--;
				if (c_idx<c_ptr) {
					c_ptr=c_idx;
					disp_c_ed();
				}
				else
				cursor_left(c_color,1);
			}
		}
		break;
	case XK_Right:
		clr_first_t();
		if (c_idx<c_len) {
#if	CHINESE
			if (B2nd(cursor_y,cursor_x+1)) {
				c_idx+=2;
				cursor_right(c_color,2);
			} else
#endif	CHINESE
			{
				c_idx++;
				if (c_ptr+c_width<=c_idx) {
					c_ptr++;
					disp_c_ed();
				} else
				cursor_right(c_color,1);
			}
		}
		break;
	case XK_Tab:
		if (c_noitem==1 && c_len) {
			static char dd[128];
			char ee[128];
			char pa[32], *nam;
			int plen;
			struct dirent *dire;

			if (!mcount) {
				for(i=c_len-1;i>=0;i--)
					if (c_ed[i]=='/') break;
				if (i<0) {
					strcpy(dd,".");
					strcpy(pa,c_ed);
					plen=c_len;
				}
				else {
					if (!i) {
						strcpy(dd,"/");
					} else {
						memcpy(dd,c_ed,i);
						dd[i]=0;
					}
					strcpy(pa,&c_ed[i+1]);
					plen=c_len-i-1;
				}
				strcpy(ee,decode_dir(dd));
				if ((dir=opendir(ee))==NULL) {
					bell();
					goto lll1;
				}
				while (dire=readdir(dir)) {
					nam=dire->d_name;
					if (strlen(nam) < plen || !strcmp(nam,".") || !strcmp(nam,".."))
						continue;
					if (!memcmp(nam,pa,plen)) {
						mpat[mcount++]=strdup(nam);
					}
				}
				close(dir);
				if (!mcount) {
					bell();
					goto lll1;
				}
				midx=0;
				qsort(mpat, mcount, sizeof(char *), strqcmp);
			}
			in_file_comp=1;
			if (midx>=mcount) {
				bell();
				goto lll1;
			}
			if (!strcmp(dd,"."))
				strcpy(c_ed,mpat[midx++]);
			else {
				strcpy(c_ed,dd);
				if (strcmp(dd,"/"))
					strcat(strcat(c_ed,"/"),mpat[midx++]);
				else
	  			strcat(c_ed,mpat[midx++]);
			}
			c_idx=c_len=strlen(c_ed);
			disp_c_ed();
			break;
		}
lll1:
		clr_first_t();
		hide_cursor_r(2);
		save_ed();
		to_bottom();
		active_item=(active_item+1)%c_noitem;
		init_c_ed(-1);
		gotoxy(c_x+c_idx-c_ptr,c_y);
		show_cursor_r();
		break;
	case XK_Return:
		save_ed();
		to_bottom();
		input_handler=0;
		xgets_enter=1;
		close_box();
		clr_xystr();
		redraw_twin();
		xgets[ag].action();
		break;
	case XK_BackSpace:
		clr_first_t();
		to_bottom();
		if (c_idx) {
#if	CHINESE
			int ofs=B2nd(cursor_y,cursor_x-1) && c_idx>1 ?  2:1;
#else
			int ofs=1;
#endif
      if (state&ShiftMask) {
        mmemmove(c_ed,c_ed+c_idx,c_len-c_idx);
        c_len-=c_idx;
        c_idx=0;
      } else {
        for(i=c_idx;i<c_len;i++)
          c_ed[i-ofs]=c_ed[i];
        c_len-=ofs;
        c_idx-=ofs;
        if (c_ptr && c_idx<c_ptr+1) {
          c_ptr=c_idx-c_width+1;
          if (c_ptr<0) c_ptr=0;
        }
			}
			disp_c_ed();
			show_cursor_r();
		}
		break;
	case XK_Delete:
		clr_first_t();
		to_bottom();
		if (c_idx<c_len) {
#if	CHINESE
			int ofs=B1st(cursor_y,cursor_x) &&
				c_idx<c_len-1 ?  2:1;
#else
			int ofs=1;
#endif
      if (state&ControlMask) {
        c_len=c_idx;
      } else {
        for(i=c_idx;i<c_len-ofs;i++)
          c_ed[i]=c_ed[i+ofs];
        c_len-=ofs;
      }
			disp_c_ed();
			show_cursor_r();
		}
		break;
	case XK_Home:
		clr_first_t();
		c_idx=0;
		if (c_ptr)
			c_ptr=0;
		disp_c_ed();
		break;
	case XK_End:
		clr_first_t();
		c_idx=c_len;
		disp_c_ed();
		break;
	case XK_Escape:
		to_bottom();
		input_handler=0;
		xgets_enter=-1;
		close_box();
		clr_xystr();
		redraw_twin();
		break;
	case XK_Insert:
		insert_mode_r^=1;
		show_cursor_r();
		break;
	default:
		if (first_t[active_item]) {
			first_t[active_item]=0;
			c_ed[0]=0;
			c_ptr=c_idx=c_len=0;
			disp_c_ed();
		}
		to_bottom();
		if (insert_mode_r && astr[0] && c_len+alen < MAX_EDLN_STR) {
			for(i=c_len-1;i>=c_idx;i--) c_ed[i+alen]=c_ed[i];
			memcpy(&c_ed[c_idx],astr,alen);
			c_idx+=alen;
			c_len+=alen;
			disp_c_ed();
		} else
		if (!insert_mode_r && c_idx+alen < MAX_EDLN_STR) {
			memcpy(&c_ed[c_idx],astr,alen);
			c_idx+=alen;
			c_len=Max(c_len,c_idx);
			disp_c_ed();
		}
		else if (astr[0]) bell();
		show_cursor_r();
}
if (!in_file_comp) {
	if (mcount) {
		for(i=0;i<mcount;i++)
			if (mpat[i]) {
				free(mpat[i]);
				mpat[i]=0;
			}
		mcount=0;
	}
}
}

char *return_item(int i)
{
int ag=active_gets;
int ai=i;
int hno=xgets[ag].edln[ai].hino-1;
if (hno < 0) hno=0;
return xgets[ag].edln[ai].hist[hno];
}
