/*
	Copyright (C)	1995
	Edward	Der-Hua Liu,	Hsin-Chu, 	Taiwan
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#ifdef	AIX
#include <sys/select.h>
#endif

#include "ted.h"

static char *ted_atom="TedFname";
static char *ted_res="TedRes";

static char *ted_name="Ted";
static int screen_no;
static Colormap colormap;
static XColor mouse_fg_color,mouse_bg_color;
static XFontStruct     *afont, *cfont;
static long foreground, background;
static char *geomstr="80x24";
Display *display;
GC      gc;
#if	CHINESE
GC	cgc;
#endif
int XW_ROW=M_ROW;
int XW_COL=M_COL;
int MROW=M_ROW;
int MCOL=M_COL;
int X_inited=0;
extern int batch;

Window main_win,root;

Atom wm_del_win,atom_fname,atom_res;

#define MARGIN 0

static XSizeHints sizehints = {
	PMinSize | PMaxSize | PResizeInc | PBaseSize | PWinGravity,
	0, 0, M_COL, M_ROW,	/* x, y, width and height */
	10, 5,	/* Min width and height */
	0, 	0,	/* Max width and height */
	1, 	1,	/* Width and height increments */
	{0, 0}, {0, 0}, /* Aspect ratio - not used */
	2 * MARGIN, 2 * MARGIN,	/* base size */
	NorthWestGravity	/* gravity */
};

int fwidth, fheight;

void p_err(char *fmt,...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr,"%s:", ted_name);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr,"\n");
	exit(-1);
}

void error(char *fmt,...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr,"%s:", ted_name);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr,"\n");
}

#if	CHINESE
char *Big5FontName="et24m";
char *AscFontName="vga12x24";
#else
char *AscFontName="10x20";
#endif

int fontidx=-1;
int fontN=5;
char *fontlist[16];

#include "color.h"

ColorDef pc_color[13+MaxLang*(MaxKeyWType+8)]= {
{0,0,"black","pink"},		/* 0 */
{0,0,"yellow","red"},		/* 1 */
{0,0,"yellow","steelblue"},	/* 2 */
{0,0,"yellow","blue"},		/* 3 */
{0,0,"red","pink"},		/* 4 */
{0,0,"yellow","pink"},		/* 5 */
{0,0,"yellow","steelblue"},	/* 6 */
{0,0,"yellow","steelblue"},	/* 7 */
{0,0,"yellow","blue"},		/* 8 */
{0,0,"yellow","pink"},		/* 9 */
{0,0,"yellow","pink"},		/* 10 */
{0,0,"pink","blue"},		/* 11 */
{0,0,"blue","green"},		/* 12 */

};

int ColorCnt=13;

int check_alloc_color(int i, int fb)
{
int j;
XColor color;

char *colorstr=(fb)?pc_color[i].bg_str:pc_color[i].fg_str;

	for(j=0;j<i;j++) {
		if (!strcmp(pc_color[j].fg_str,colorstr)) {
			if (fb) pc_color[i].bg=pc_color[j].fg;
			else
				pc_color[i].fg=pc_color[j].fg;
			return 0;
		}
		if (!strcmp(pc_color[j].bg_str,colorstr)) {
			if (fb) pc_color[i].bg=pc_color[j].bg;
			else
				pc_color[i].fg=pc_color[j].bg;
			return 0;
		}
	}
	if (!XParseColor(display,colormap,colorstr, &color))
		p_err("invalid color %d %s",i, colorstr);
	if (!XAllocColor(display,colormap,&color)) {
		error("can't allocate color %s, will use private colormap.",
		  pc_color[i].fg_str);
		return -1;
	}
	if (fb)
	pc_color[i].bg=color.pixel;
	else
	pc_color[i].fg=color.pixel;
return 0;
}

static Cursor xcursor;
int alloc_pc_colors()
{
XColor color;
int i,j;

for(i=0;i<ColorCnt;i++)
for(j=0;j<2;j++) {
	if (check_alloc_color(i,j)<0) return -1;
}
foreground=pc_color[0].fg;
background=pc_color[0].bg;
if (!XParseColor(display,colormap,pc_color[11].fg_str, &mouse_fg_color))
	p_err("invalid color %s",pc_color[11].fg_str);
if (!XParseColor(display,colormap,pc_color[11].bg_str, &mouse_bg_color))
	p_err("invalid color %s",pc_color[11].bg_str);
if (!X_inited) xcursor = XCreateFontCursor(display,XC_xterm);
XRecolorCursor(display,xcursor,&mouse_fg_color,&mouse_bg_color);
if (X_inited)
	XDefineCursor(display,main_win,xcursor);
return 0;
}


static void load_font(int setsize)
{
int x, y, width, height;
int flags;

	if (afont) XFreeFont(display, afont);
	setFontIndex();
  if (!(afont=XLoadQueryFont(display, AscFontName)))
		p_err("Cannot load ascii font %s", AscFontName);
#if	CHINESE
	if (cfont) XFreeFont(display, cfont);
	if (!(cfont=XLoadQueryFont(display, Big5FontName)))
 	  p_err("Cannot load Chinese font %s", Big5FontName);
#endif
	fwidth=sizehints.width_inc = afont->max_bounds.width;
	fheight=sizehints.height_inc = afont->ascent + afont->descent;
	if (!setsize) return;

	flags = XParseGeometry(geomstr,&x,&y,&width,&height);
	if (flags & WidthValue) {
		sizehints.flags |= USSize;
		XW_COL=MCOL=width;
	} else width=MCOL;
	if (flags & HeightValue) {
		sizehints.flags |= USSize;
		XW_ROW=MROW=height;
	} else height=MROW;
	page_len=MROW-2;
	page_width=MCOL;
	sizehints.width = width * sizehints.width_inc +
	sizehints.base_width;
	sizehints.height = height * sizehints.height_inc +
	sizehints.base_height + 1;
	sizehints.min_width=16* sizehints.width_inc;
	sizehints.min_height=4* sizehints.height_inc;
	sizehints.max_width=160* sizehints.width_inc;
	sizehints.max_height=80* sizehints.height_inc;
	if (flags & XValue) {
	  if (flags & XNegative) {
	  x = DisplayWidth(display,screen_no) + x - sizehints.width - 2;
	  sizehints.win_gravity = NorthEastGravity;
	}
	  sizehints.x = x;
	  sizehints.flags |= USPosition;
	}
	if (flags & YValue) {
	  if (flags & YNegative) {
			y = DisplayHeight(display,screen_no) + y - sizehints.height - 2;
			sizehints.win_gravity = SouthWestGravity;
			if((flags&XValue)&&(flags&XNegative))
				sizehints.win_gravity = SouthEastGravity;
	  }
	  sizehints.y = y;
	  sizehints.flags |= USPosition;
	}

}

void create_gc()
{
XGCValues gcv;
gcv.foreground = foreground;
gcv.background = background;
gcv.graphics_exposures = False;
gcv.font=afont->fid;

if (gc) XFreeGC(display,gc);
gc = XCreateGC(display,main_win,GCForeground|GCBackground|GCFont|
GCGraphicsExposures,&gcv);
#if	CHINESE
if (cgc) XFreeGC(display,gc);
gcv.font=cfont->fid;
cgc = XCreateGC(display,main_win,GCForeground|GCBackground|GCFont,&gcv);
#endif
}

void setWinSize();

void K_SwitchFont()
{
fontidx=(fontidx+1)%fontN;
if (fontidx<0) {
  fontidx=0;
  return;
}
if (!strcmp(AscFontName,fontlist[fontidx]))
  return;
AscFontName=fontlist[fontidx];
load_font(0);
create_gc();
setWinSize(XW_ROW,XW_COL);
}

setFontIndex()
{
int i;
if (fontidx>=0) return;
for(i=0;i<fontN;i++)
  if (fontlist[i] && !strcmp(AscFontName,fontlist[i])) {
    fontidx=i;
    return;
  }
}

void setFontList(char *s)
{
int i;
char *t,*e=s+strlen(s);

if (fontN) {
	for(i=0;i<fontN;i++) free(fontlist[i]);
}
fontN=0;
while (s<e) {
	t=s;
	while (*t!=' ' && *t!='\t' && *t) t++;
	*t=0;
	fontlist[fontN++]=strdup(s);
	s=t+1;
	while (s < e && *s==' ' || *s=='\t') s++;
}
#if	0
printf("fontN:%d\n",fontN);
for(i=0;i<fontN;i++)
	puts(fontlist[i]);
#endif
}

#define MW_EVENTS (	KeyPressMask |\
			FocusChangeMask |\
			StructureNotifyMask |\
			ButtonPressMask |\
			ButtonReleaseMask |\
			ExposureMask    |\
			PropertyChangeMask | \
			Button1MotionMask |\
			VisibilityChangeMask \
		 )

XClassHint class;

static void create_win(int argc, char **argv)
{
	XWMHints wmhints;
	XTextProperty name;
	XSetWindowAttributes watt;
#if	0
	main_win = XCreateSimpleWindow(display,DefaultRootWindow(display),
         sizehints.x,sizehints.y,
				 sizehints.width,sizehints.height,
         1,foreground,background);
#else
  watt.colormap=colormap;
  watt.background_pixel=background;
  watt.cursor=xcursor;
	main_win = XCreateWindow(display,root,
         sizehints.x,sizehints.y, sizehints.width,sizehints.height,
         1,CopyFromParent,InputOutput,CopyFromParent,
         CWBackPixel|CWColormap|CWCursor,&watt);
#endif
	class.res_name = ted_name;
	class.res_class = "Ted";
	wmhints.input = True;
	wmhints.initial_state = NormalState;
	wmhints.flags = InputHint | StateHint;

	if (XStringListToTextProperty(&ted_name,1,&name) == 0) {
	  error("cannot allocate window name");
	  return;
	}
	XSetWMProperties(display,main_win,&name,&name,argv,argc,
		   &sizehints,&wmhints,&class);
	XFree(name.value);

	XSelectInput(display,main_win,MW_EVENTS);
#if	0
	XDefineCursor(display,main_win,xcursor);
#endif
}

static XErrorHandler XTedErrorHandler(Display *dpy, XErrorEvent *event)
{
#if	0
	(void) XmuPrintDefaultErrorMessage (dpy, event, stderr);
exit(-1);
#endif
}

#define MAX_ROW (M_ROW+100)
#define MAX_COL (M_COL+200)
static u_char tch[MAX_ROW][MAX_COL];	/* store text */
static u_char tat[MAX_ROW][MAX_COL];	/* color attribute */
static u_char tchd[MAX_ROW][MAX_COL];	/* store text */
static u_char tatd[MAX_ROW][MAX_COL];	/* color attribute */

static void ostr(int x, int y, char *str, int len, u_char att)
{
int ix,iy;
u_long fg,bg;

if (!len) len=strlen(str);
memcpy(&tch[y][x], str, len);
memcpy(&tchd[y][x], str, len);
memset(&tat[y][x], att, len);
memset(&tatd[y][x], att, len);
ix=x*fwidth;
iy=y*fheight+afont->ascent;
fg=pc_color[att&0x3f].fg;
bg=pc_color[att&0x3f].bg;
XSetForeground(display, gc, fg);
XSetBackground(display, gc, bg);
XDrawImageString(display, main_win, gc, ix,iy, str, len);
}

static void ostrd(int x, int y, char *str, int len, u_char att)
{
if (!len) len=strlen(str);
memcpy(&tch[y][x], str, len);
memset(&tat[y][x], att, len);
}

#if	CHINESE
static void costr(int x, int y, char *str, int len, u_char att)
{
int ix,iy;
int i;
u_char *cp=&tat[y][x];
u_char *cq=&tatd[y][x];
u_long fg,bg;

if (!len) len=strlen(str);
memcpy(&tch[y][x], str, len);
memcpy(&tchd[y][x], str, len);
memset(&tat[y][x], att, len);
memset(&tatd[y][x], att, len);
for(i=0;i<len;i++) {
	 *(cp+i)|= ((i&1)?0x40:0x80);		/* 0:asc  0x80:CH1   0x40:CH2 */
	 *(cq+i)|= ((i&1)?0x40:0x80);		/* 0:asc  0x80:CH1   0x40:CH2 */
}

ix=x*fwidth;
iy=y*fheight+cfont->ascent;
fg=pc_color[att&0x3f].fg;
bg=pc_color[att&0x3f].bg;

XSetForeground(display, cgc, fg);
XSetBackground(display, cgc, bg);
XDrawImageString16(display, main_win, cgc, ix,iy, (XChar2b *)str, len>>1);
}

static void costrd(int x, int y, char *str, int len, u_char att)
{
int i;
u_char *cp=&tat[y][x];
if (!len) len=strlen(str);
memcpy(&tch[y][x], str, len);
memset(&tat[y][x], att, len);
for(i=0;i<len;i++)
	*(cp+i)|= ((i&1)?0x40:0x80);	/* 0:asc  0x40:CH1  0x20:CH2 */

}
#endif

int cursor_x=0, cursor_y=0;


void draw_edit_cursor(u_char att)
{
int x,y;
int len;
GC tgc;
u_long fg,bg;

	y=cursor_y;
	x=cursor_x;
	tatd[y][x]&=0xc0;
	tatd[y][x]|=att;
	if (tat[y][x] & 0x40) {
	if (att!=1) {
		x--;
		goto xxxx;
	}
		XSetForeground(display, gc, pc_color[att].bg);
	XDrawRectangle(display,main_win,gc,
			x*fwidth,y*fheight,fwidth-1,fheight-1);
	return;
	}
xxxx:
#if	CHINESE
	if (tat[y][x]&0x80 && (x<XW_COL-1) && (tat[y][x+1]&0x40)) {
	len=2;
		tatd[y][x+1]&=0xe0;
		tatd[y][x+1]|=att;
	}
	else
#endif
	len=1;
#if	CHINESE
	tgc=(len==2?cgc:gc);
#else
	tgc=gc;
#endif
	XSetForeground(display, tgc, pc_color[att].fg);
	XSetBackground(display, tgc, pc_color[att].bg);
#if	CHINESE
	if (len==2)
	XDrawImageString16(display, main_win, tgc, x*fwidth,
	y*fheight+cfont->ascent,(XChar2b *) &tch[y][x], 1);
	else
#endif
	XDrawImageString(display, main_win, tgc, x*fwidth,
	y*fheight+afont->ascent, &tch[y][x], 1);
}

void hide_cursor()
{
if (in_mlines(cursor_row) || in_mblock(cursor_row,page_col+cur_x)) {
	draw_edit_cursor(3);
}
else {
	draw_edit_cursor(0);
}
}

int usexcopy=1;

void draw_scr()
{
int y;
u_char catt,cbyte;
u_char *a,*b,*c,*d;
int stx,i,j,msti[MAX_ROW*MAX_ROW],mstj[MAX_ROW*MAX_ROW],mlen[MAX_ROW*MAX_ROW];
int len,mcou;
GC tgc;
u_long fg,bg;

#if	0
dump1();
dump2();
#endif

if (batch) return;
mcou=0;
if (!usexcopy) goto nocopy;
for(i=0;i<XW_ROW;i++)
	for(j=0;j<XW_ROW;j++)
		if (i!=j &&
		    !memcmp(&tch[i],&tchd[j],XW_COL) &&
				!memcmp(&tat[i],&tatd[j],XW_COL)) {
		  int k,l;
		  k=i;
		  l=j;
			while (k < XW_ROW && l < XW_ROW &&
			  !memcmp(&tch[k],&tchd[l],XW_COL) &&
				!memcmp(&tat[k],&tatd[l],XW_COL)) {
				k++;
				l++;
			}
			len=k-i;
			if (len) {
			  msti[mcou]=i;
			  mstj[mcou]=j;
				mlen[mcou++]=len;
				if (len > XW_ROW>>1) goto ll1;
			}
		}
ll1:
if (mcou) {
	int cmidx=0,cmlen=0;

	for(i=1;i<mcou;i++)
		if (mlen[i] > mlen[cmidx])
		  cmidx=i;

	i=msti[cmidx];  /* new */
	j=mstj[cmidx];  /* old */
	len=mlen[cmidx];
#if	0
	printf("%d %d %d\n",i,j,len);
#endif
	for(y=i;y<i+len;y++) {
		memcpy(&tchd[y][0],&tch[y][0],XW_COL);
	  memcpy(&tatd[y][0],&tat[y][0],XW_COL);
	}
#if	1
	XCopyArea(display,main_win,main_win,gc, 0,j*fheight,
		XW_COL*fwidth,len*fheight, 0,i*fheight);
#endif
}

nocopy:
for(y=0;y<XW_ROW;y++) {
	a=&tch[y][0];
	b=&tchd[y][0];
	c=&tat[y][0];
	d=&tatd[y][0];
	stx=0;
	while (stx < XW_COL) {
#if	CHINESE
	while (*a==*b && !(*c & 0xc0) && *c==*d && stx<XW_COL) {
#else
	while (*a==*b && *c==*d && stx<XW_COL) {
#endif
	stx++; a++; b++; c++; d++;
	}
	if (stx>=XW_COL) break;
	catt=*c & 0x3f;
#if	0
	if (catt==1) printf("at: y:%d stx:%d\n", y, stx);
#endif
	len=0;
	if (*c & 0xc0) cbyte=0xc0;
	else cbyte=0;
	while (catt==(*c&0x3f) &&
	cbyte==(*c&0xc0?0xc0:0) && (len+stx)<XW_COL &&
(*a!=*b || *c!=*d || *c & 0xc0) ) {
	len++;
	*(b++)=*(a++);
	*(d++)=*(c++);
	}
#if	CHINESE
	tgc=(cbyte&0xc0)?cgc:gc;
#else
	tgc=gc;
#endif
	fg=pc_color[catt&0x3f].fg;
	bg=pc_color[catt&0x3f].bg;
	XSetForeground(display, tgc, fg);
	XSetBackground(display, tgc, bg);
#if	CHINESE
	if (cbyte & 0xc0)
	XDrawImageString16(display, main_win, tgc, stx*fwidth,
	y*fheight+cfont->ascent,
	(XChar2b *) &tch[y][stx], len>>1);
	else
#endif
	XDrawImageString(display, main_win, tgc, stx*fwidth,
	y*fheight+afont->ascent,
	&tch[y][stx], len);
	stx+=len;
	}
}

}


void draw_line(int y)
{
u_char catt,cbyte;
u_char *a,*b,*c,*d;
int stx,i;
int len;
GC tgc;
u_long fg,bg;

	a=&tch[y][0];
	b=&tchd[y][0];
	c=&tat[y][0];
	d=&tatd[y][0];
	stx=0;

	while (stx < XW_COL) {
#if	CHINESE
	while (*a==*b && !(*c & 0xc0) && *c==*d && stx<XW_COL) {
#else
	while (*a==*b && *c==*d && stx<XW_COL) {
#endif
	stx++; a++; b++; c++; d++;
	}
	if (stx>=XW_COL) break;
	catt=*c & 0x3f;
	len=0;
	if (*c & 0xc0) cbyte=0xc0;
	else cbyte=0;
	while (catt==(*c&0x3f) &&
	cbyte==(*c&0xc0?0xc0:0) && (len+stx)<XW_COL &&
(*a!=*b || *c!=*d || *c & 0xc0) ) {
	len++;
	*(b++)=*(a++);
	*(d++)=*(c++);
	}
#if	CHINESE
	tgc=(cbyte&0xc0)?cgc:gc;
#else
	tgc=gc;
#endif
	fg=pc_color[catt&0x3f].fg;
	bg=pc_color[catt&0x3f].bg;
	XSetForeground(display, tgc, fg);
	XSetBackground(display, tgc, bg);
#if	CHINESE
	if (cbyte & 0xc0)
	XDrawImageString16(display, main_win, tgc, stx*fwidth,
	y*fheight+cfont->ascent,
	(XChar2b *) &tch[y][stx], len>>1);
	else
#endif
	XDrawImageString(display, main_win, tgc, stx*fwidth,
	y*fheight+afont->ascent,
	&tch[y][stx], len);
	stx+=len;
	}
}

void draw_line_w(int y)
{
	draw_line(win_y+y);
}

xputs(int x, int y, u_char *s, u_char att)
{
u_char ch;
int len=strlen(s);
int cl=0;
u_char *cp, *endp, *strp;
char ctyp;
u_char hb,lb;

#if	CHINESE
strp=cp=s;
endp=s+len;
cl=0;
ctyp=0;

while (cp<endp) {
	hb=*cp;
	if ((hb>=0x81&&hb<=0xfe) && cp+1<endp) {
	lb=*(cp+1);
	if (lb>=0x40&&lb<=0x7e || lb>=0xa1&&lb<=0xfe) {
		if (ctyp==1) {
			ostr(strp-s+x,y,strp,cl,att);
			cl=0;
			strp=cp;
		}
		cl+=2;
		cp+=2;
		ctyp=2;
		continue;
	}
	}
	if (ctyp==2) {
	costr(strp-s+x,y,strp,cl,att);
	cl=0;
	strp=cp;
	}
	cl++;
	cp++;
	ctyp=1;
}

if (ctyp==1) ostr(strp-s+x,y,strp,cl,att);
else
if (ctyp==2) costr(strp-s+x,y,strp,cl,att);
#else /* else CHINESE */
	ostr(x,y,s,strlen(s),att);
#endif /* CHINESE */
}


static xputsd(int x, int y, u_char *s, u_char att)
{
u_char ch;
int len=strlen(s);
int cl=0;
u_char *cp, *endp, *strp;
char ctyp;
u_char hb,lb;

if (!len) return;
#if	CHINESE
strp=cp=s;
endp=s+len;
cl=0;
ctyp=0;

while (cp<endp) {
	hb=*cp;
	if ((hb>=0x81&&hb<=0xfe) && cp+1<endp) {
	lb=*(cp+1);
	if (lb>=0x40&&lb<=0x7e || lb>=0xa1&&lb<=0xfe) {
		if (ctyp==1) {
			ostrd(strp-s+x,y,strp,cl,att);
			cl=0;
			strp=cp;
		}
		cl+=2;
		cp+=2;
		ctyp=2;
		continue;
	}
	}
	if (ctyp==2) {
	costrd(strp-s+x,y,strp,cl,att);
	cl=0;
	strp=cp;
	}
	cl++;
	cp++;
	ctyp=1;
}

if (ctyp==1) ostrd(strp-s+x,y,strp,cl,att);
else
if (ctyp==2) costrd(strp-s+x,y,strp,cl,att);
#else	/* else CHINESE */
	ostrd(x,y,s,len,att);
#endif
}

u_char cur_att;
void xprintf(char *fmt, ...)
{
	va_list args;
	char tmp[100];

	va_start(args, fmt);
	vsprintf(tmp, fmt, args);
	va_end(args);
	xputs(cursor_x, cursor_y, tmp, cur_att);
	cursor_x+=strlen(tmp);
}

void xputstr(char *str)
{
	xputs(cursor_x, cursor_y, str, cur_att);
	cursor_x+=strlen(str);
}

void xputstrd(char *str)
{
	if (!str[0]) return;
	xputsd(cursor_x, cursor_y, str, cur_att);
	cursor_x+=strlen(str);
}

static u_char out_buf[128];
static int buf_count=0;

void flush_buffer()
{
	if (buf_count) {
		out_buf[buf_count]=0;
		xputstrd(out_buf);
		buf_count=0;
		}
}

void gotoxy(int x, int y)
{
	flush_buffer();
	cursor_x=x;
	cursor_y=y;
}

void gotoxy_w(int x, int y)
{
	flush_buffer();
	cursor_x=x+win_x;
	cursor_y=y+win_y;
}

void set_att(u_char att)
{
if (cur_att!=att) {
	flush_buffer();
	cur_att=att;
}
}

void xputch(u_char ch)
{
out_buf[buf_count++]=ch;
}




void xputch_w(u_char ch)
{
	if (cursor_x+buf_count>=win_x+page_width) return;
	out_buf[buf_count++]=ch;
}

#if	0
void dump1()
{
int y,x;
for(y=0;y<24;y++) {
	for(x=0;x<80;x++)
		putchar(tch[y][x]);
	puts("");
}
puts("------------------------------------");
}

void dump2()
{
int y,x;
for(y=0;y<24;y++) {
	printf("#%d ",y);
	for(x=0;x<80;x++)
		printf("%2x",tatd[y][x]);
	puts("");
}
puts("************************************");
}

void dump3()
{
int y,x;
for(y=0;y<MROW;y++) {
	for(x=0;x<MCOL;x++)
		if ((tat[y][x]&0x3f)==1)
			printf("dump3: %d %d:%d\n",y,x,tat[y][x]);
}
puts("------------------------------------");
}

void dump4()
{
int y,x;
for(y=0;y<MROW;y++) {
	for(x=0;x<MCOL;x++)
		if ((tatd[y][x]&0x3f)==1)
			printf("dump4: %d %d:%d\n",y,x,tatd[y][x]);
}
puts("************************************");
}
#endif

extern int cur_x,cur_y;
extern int page_width,page_len;

void clear_rect(int x,int y,int width,int height)
{
	int i;
	for(i=y;i<y+height;i++) {
		memset(&tch[i][x],' ',width);
		memset(&tat[i][x],0,width);
	}
	cursor_x=cursor_y=0;
}

void clr_line_w(int y)
{
	memset(&tch[win_y+y][win_x],' ',page_width);
	memset(&tat[win_y+y][win_x],0,page_width);
}

void clrscr_all()
{
	memset(tch,' ',sizeof(tch));
	memset(tat,0,sizeof(tat));
	cursor_x=cursor_y=0;
}



void flush_x()
{
	XFlush(display);
}

void scroll_up_w(int top)
{
int i;
for(i=top;i<page_len-1;i++)
{
	memcpy(&tch[win_y+i][win_x],&tch[win_y+i+1][win_x],page_width);
	memcpy(&tat[win_y+i][win_x],&tat[win_y+i+1][win_x],page_width);
}
}

void scroll_down_w(int top)
{
int i;
for(i=page_len-1;i>top;i--)
{
	memcpy(&tch[win_y+i][win_x],&tch[win_y+i-1][win_x],page_width);
	memcpy(&tat[win_y+i][win_x],&tat[win_y+i-1][win_x],page_width);
}
}

static int cur_tatt;
void show_cursor_w()
{
  if (batch) return;
  cursor_x=cur_x+win_x;
  cursor_y=cur_y+win_y;
  if (tat[cursor_y][cursor_x]!=1) cur_tatt=tat[cursor_y][cursor_x];
  if (insert_mode)
    draw_edit_cursor(1);
  else
    draw_edit_cursor(12);
}


void hide_cursor_w()
{
	if (batch) return;
	cursor_x=cur_x+win_x;
	cursor_y=cur_y+win_y;
	if (in_mlines(cursor_row) || in_mblock(cursor_row,page_col+cur_x)) {
		draw_edit_cursor(3);
	}
	else {
		draw_edit_cursor(cur_tatt & 0x3f);
	}
}

int insert_mode_r=1;

void show_cursor_r()
{
if (insert_mode_r)
	draw_edit_cursor(1);
else
	draw_edit_cursor(12);
}

void disp_cursor()
{
if (!input_handler)
 show_cursor_w();
else
if (input_handler<2)
 show_cursor_r();
}

void hide_cursor_r(int color)
{
	draw_edit_cursor(color);
}

void inactive_cursor()
{
	hide_cursor();
	if (input_handler>1) return;
  if (insert_mode && !input_handler || insert_mode_r && input_handler==1)
    XSetForeground(display, gc, pc_color[1].bg);
	else
    XSetForeground(display, gc, pc_color[12].bg);
	XDrawRectangle(display,main_win,gc,
			cursor_x*fwidth,cursor_y*fheight,fwidth-1,fheight-1);
}

void cursor_left(int color,int dist)
{
	hide_cursor_r(color);
	cursor_x-=dist;
	show_cursor_r();
}

void cursor_right(int color,int dist)
{
	hide_cursor_r(color);
	cursor_x+=dist;
	show_cursor_r();
}

void clr_to_eol()
{
char ss[500];
int len;

flush_buffer();
memset(ss,' ',len=page_width-cursor_x);
ss[len]=0;
xputstrd(ss);
}
#if	CHINESE
unsigned B1st(int y, int x)
{
	return (tat[y][x] & 0x80);
}

unsigned B2nd(int y, int x)
{
	return (tat[y][x] & 0x40);
}
#endif

int visual_bell=0;
void bell()
{
  if (visual_bell) {
    XClearWindow(display, main_win);
    XFlush(display);
    restore();
  }
  else
  XBell(display, 100);
}

extern int input_handler;

void resize(int width, int height)
{
  int nXW_COL=width/fwidth;
  int nXW_ROW=height/fheight;
  if (nXW_COL==XW_COL && nXW_ROW==XW_ROW)
    return;
  XW_COL=nXW_COL;
  XW_ROW=nXW_ROW;
  save_win_context(active_win);
  resize_twin();
  edln_restore();
  refresh_filebrowser();
  disp_kwc_pg();
  disp_kb_mode();
  if (!input_handler) {
    show_cursor_w();
  } else
  if (input_handler==1) {
    edln_restore();
    show_cursor_r();
  }
}


restore()
{
int y;
for(y=0;y<XW_ROW;y++)
	bzero(tchd[y],XW_COL);

draw_scr();
disp_cursor();
}

static void scr_refresh(int x,int y, int width, int height)
{
  int i,j;
  int ousexcopy=usexcopy;

  x/=fwidth;
  y/=fheight;
  width=(width+fwidth-1)/fwidth + 1;
  height=(height+fheight-1)/fheight + 1;
  for (j=y;j<y+height;j++) {
    memset(&tchd[j][x],0,width);
  }
  usexcopy=0;
  draw_scr();
  usexcopy=ousexcopy;
  disp_cursor();
}


int auto_save_time=300;


Time eve_time;

static int button0x=-1,button0y=-1,page_row0=-1;
int CursorFollowClick=1;

int xsockfd;

#define REFRESH_PERIOD (2)
#include <sys/time.h>

void X_event()
{
XEvent event;
int x,y;
fd_set chk_fd;
struct timeval refreh_time;
static int save_t_cou;
static button_press_time;
static motion_dire;
extern int win_split;

	XFlush(display);
	FD_ZERO(&chk_fd);
	FD_SET(xsockfd, &chk_fd);
	refreh_time.tv_sec=REFRESH_PERIOD;
	refreh_time.tv_usec=0;
	if (!XPending(display) &&
		!select(xsockfd+1,&chk_fd,NULL,NULL,&refreh_time)) {
    show_time();
    canCloseCwin();
    if (auto_save_time && save_t_cou>=auto_save_time) {
      save_tmp_bak();
      save_t_cou=0;
    } else save_t_cou++;
    return;
	}
	while (XPending(display)) {
	XNextEvent(display,&event);

	switch(event.type) {
	case KeyPress: {
		int count,k_status;
		char astr[10];
		char chstr[256];
		KeySym skey;
		XKeyEvent *keve=(XKeyEvent *)&event;

		eve_time=keve->time;
		astr[0]=0;
		count = XLookupString (keve, astr, sizeof(astr),
			&skey, NULL);
		astr[count]=0;
#if	CHINESE
		k_status=send_key(display, main_win, &event, chstr);
		if (!k_status)
#endif
		handle_key(astr,skey,keve->state);
#if	CHINESE
		else
		if (chstr[0]) {
			switch (input_handler) {
				case 0:
					u_del_str(chstr,strlen(chstr),1);
					put_str(chstr,strlen(chstr));
					break;
				case 1:
					handle_key(chstr,skey,keve->state);
			}
		}
#endif
	}
	break;
	case ClientMessage:
	  if (event.xclient.format == 32 && event.xclient.data.l[0] == wm_del_win)
		  close_quit_file();
	break;
	case SelectionClear:
	break;
	case SelectionRequest:
	send_selection((XSelectionRequestEvent *)&event);
	break;
	case SelectionNotify:
	paste_primary(event.xselection.requestor,
			 event.xselection.property,True);
	break;
	case GraphicsExpose:
	case Expose:
	      scr_refresh(event.xexpose.x,event.xexpose.y,
		      event.xexpose.width,event.xexpose.height);
	break;
#if	1
	case ConfigureNotify:
	resize(event.xconfigure.width,event.xconfigure.height);
	break;
#endif
#if	0
	case ResizeRequest:
	printf("wid:%d height:%d\n",event.xresizerequest.width,
		event.xresizerequest.height);
	resize(event.xresizerequest.width,event.xresizerequest.height);
	break;
#endif
	case ButtonPress:
	if (input_handler) break;
	x=event.xbutton.x;
	y=event.xbutton.y;
	eve_time=event.xbutton.time;
	switch (event.xbutton.button) {
		static first_x=-1,first_y=-1;
		static press_cnt;
		case Button1:
			if (event.xbutton.state & ShiftMask || CursorFollowClick) {
				goto_twin_mv_cur(x/fwidth, y/fheight);
				if (!CursorFollowClick) break;
			}
			if (goto_twin(x/fwidth, y/fheight)) break;
			K_Unmark();
			if (event.xbutton.time-button_press_time < 500)
				press_cnt++;
			else press_cnt=0;

			if (press_cnt==1) {
				mark_word_xy(x/fwidth, y/fheight);
				first_x=(mblk_col0-page_col+win_x)*fwidth;
				if (CursorFollowClick)
					goto_twin_mv_cur(first_x/fwidth, y/fheight);
			}
			else
			if (press_cnt==2) {
				mark_liney(y/fheight);
				press_cnt=0;
			} else {
				first_x=x;
				first_y=y;
			}
			button_press_time=event.xbutton.time;
			hide_cursor_w();
			show_cursor_w();
			break;
		case Button3:
			if (first_x >= 0 && x>first_x &&
					y/fheight==first_y/fheight) {
				mark_blkxy(first_x/fwidth,
				x/fwidth, first_y/fheight);
			} else first_x=-1;
	}
	break;
	case ButtonRelease:
	switch (event.xbutton.button) {
		case Button1:
		case Button3:
			eve_time=event.xbutton.time;
			if (input_handler) break;
			motion_dire=0;
			button0x=button0y=page_row0=-1;
			if (mline_begin>=0 || mblk_row0>=0) {
				set_selection();
			}
			break;
		case Button2:
			request_selection(event.xbutton.time);
	}
	break;
	case FocusIn:
#if	CHINESE
	  send_FocusIn(display, main_win);
#endif
    disp_cursor();
	break;
	case FocusOut:
	  inactive_cursor();
	break;
	case MotionNotify:
	if (input_handler) break;
	x=event.xmotion.x;
	y=event.xmotion.y;
	eve_time=event.xmotion.time;
	if (button0y<0) {
		button0y=y;
		button0x=x;
		page_row0=page_row;
	}
	if (event.xmotion.state & (ControlMask|Mod1Mask|Mod5Mask)) {
		K_MarkBlkxy(x/fwidth,y/fheight);
		break;
	}
	if (abs(x-button0x) > fwidth && y/fheight-button0y/fheight==0 &&
		page_row==page_row0) {
		mark_blkxy(button0x/fwidth,x/fwidth,
			button0y/fheight);
	} else
	if (abs(y-button0y) > 1) {
		int dire;
		if (dire=out_active_win_y(y)) {
			if (dire>0) K_pgbottom();
			else
				K_pgtop();
			motion_dire=dire;
			for (;;) {
				if (dire>0) K_cur_down();
			    	else {
			    		if (!cursor_row) break;
					K_cur_up();
				}
			    	K_MarkLn();
				FD_ZERO(&chk_fd);
				FD_SET(xsockfd, &chk_fd);
					refreh_time.tv_sec=0;
				refreh_time.tv_usec=100;
				select(xsockfd+1,&chk_fd,NULL,NULL,&refreh_time);
				if (XPending(display)) {
					XNextEvent(display,&event);
					if (event.type==ButtonRelease ||
					 event.type==MotionNotify &&
					!out_active_win_y(event.xmotion.y))
						break;
				}
			}
		} else {
			if (motion_dire >= 0)
				mark_liney(y/fheight);
			else {
				mark_liney2(y/fheight);
			}
		}
		break;
	}
	break;
	case PropertyNotify:
	  {
			XPropertyEvent *ev=&event.xproperty;
	    Window srcwin=ev->window;
	    Atom atom,actual_type;
	    int actual_format;
			u_long nitems,bytes_after;
			char *fname,res;

	    if (ev->state==PropertyDelete || srcwin!=root || ev->atom!=atom_fname)
	      break;
			if (XGetWindowProperty(display,root,atom_fname, 0, 64,
				False, AnyPropertyType, &actual_type,&actual_format,
				&nitems,&bytes_after,(u_char **)&fname) != Success)
				puts("err prop");
			if (nitems==0) break;
			res=switch_fullname(fname);
			XFree(fname);
			if (res=='Y') {
				XChangeProperty(display,root, atom_res, XA_STRING, 8,
						PropModeReplace, (u_char *)&res, 1);
				XSync(display,False);
			  XRaiseWindow(display,main_win);
			}
		}
	break;
	case VisibilityNotify:
	  if(event.xvisibility.state == VisibilityUnobscured) {
			usexcopy = 1;
	  }
	  else {
		  usexcopy = 0;
		}
	break;
#if	0
	default:
	printf("Unknow event %d\n",event.type);
#endif
	}
	}  /* while XPend */

}




void run_event()
{

xsockfd=ConnectionNumber(display);

auto_save_time/=REFRESH_PERIOD;
for(;;) {
	X_event();
}

}

extern int cdtedrc,fgps,bgps, readonly;
int autofocusin=1;
char *display_name;
extern char *goto_tagname;
char *ini_fcmd;
int batch;

typedef struct {
	char *argv;
	char *appdef;
	void *res;
	int  type;	/* 1:char*   4:int */
	char *help;
} ARG;
static ARG arg[]={
{"fg",0,	pc_color[0].fg_str,	21,	"foreground color"},
{"bg",0,	pc_color[0].bg_str,	21,	"background color"},
{"geometry",0,  &geomstr,               1,      "80x24+0-0"},
{"g",0,		&geomstr,		1,      "80x24+0-0"},
{"fn",0,  &AscFontName,   1,  "Ascii font name"},
#if	CHINESE
{"fnb5",0,	&Big5FontName, 		1,	"Big5 font name"},
#endif
{"display",0, &display_name, 1,	"display name"},
{"tedrc",0,	&tedrcfname,		1,	"tedrc file name"},
{"cdtedrc",0,   &cdtedrc,	14,     "0 or 1  dis/en current dir tedrc"},
{"bgps",0,   &bgps,	14,     "0 or 1 run as a background process"},
{"fgps",0,   &fgps,	0,     "run as a foreground process"},
{"autofocusin",0,   &autofocusin,	14,     "0 or 1 Move mouse pointer into Ted's win automatically"},
{"t",0,   &goto_tagname,	1,     "goto C tag"},
{"c",0,   &ini_fcmd,	1,     "initial Ted command file"},
{"batch",0,   &batch,	0,     "batch job (no display)"},
{"r",0,   &readonly,	0,     "read only mode"},
{"",0,   &readonly,	0,     "read from stdin"},
};

void get_arg(int argc, char **argv)
{
int num=sizeof(arg)/sizeof(ARG);
char *pp;
int i,j;
FPOS fpos;
extern int fnamesN;
extern char *fnames[];

if (!batch)
for(i=0;i<num;i++)
if  (pp=XGetDefault(display, ted_name,
			arg[i].appdef ? arg[i].appdef:arg[i].argv)) {
	switch (arg[i].type) {
	case 1:
		*(char **)arg[i].res=pp; break;
	case 11:
		*(char *)arg[i].res=atoi(pp); break;
	case 14:
		*(int *)arg[i].res=atoi(pp); break;
	case 21:
		strcpy(arg[i].res,pp);
	}

}

for (i=1;i<argc;i++) {
	if (argv[i][0]=='-') {
		if (!argv[i][1]) {
			put_arg_fname(argv[i]);
			continue;
		}
		for(j=0;j<num;j++)
		if (!strcmp(&argv[i][1],arg[j].argv) && i<argc-1) {
			pp=argv[i+1];
			switch (arg[j].type) {
			case 0:
				*(int *)arg[j].res=1; goto next;
			case 1:
				*(char **)arg[j].res=pp; break;
			case 11:
				*(char *)arg[j].res=atoi(pp); break;
			case 14:
				*(int *)arg[j].res=atoi(pp); break;
			case 21:
				strcpy(arg[j].res,pp);
			}
			i++;
			break;
		}
		if (j==num) {
			char tt[128];
			fprintf(stderr,"+==== Ted editor ver 4.4d ====+\n");
			fprintf(stderr,"| by Edward D. H. Liu, Taiwan |\n");
			fprintf(stderr,"| dhliu@solar.csie.ntu.edu.tw |\n");
			fprintf(stderr,"+-----------------------------+\n");
			error("Unknown arg %s", argv[i]);
			error("Valid args are :");
			for(j=0;j<num;j++)
				fprintf(stderr,"\t-%s  %s\n", arg[j].argv,
						arg[j].help);
			fprintf(stderr,"\n\t+lineno  goto lineno\n");
			error("\tPress F1 in Ted's window for more help");
			exit(-1);
		}
	} else
	if (argv[i][0]=='+') {
		extern int vi_goto_no;
		vi_goto_no=atoi(argv[i]+1)-1;
		if (vi_goto_no<0) vi_goto_no=0;
	}
	else {
		put_arg_fname(argv[i]);
	}
next:
/* HP-UX cc bug */
	1;
}

if (fnamesN && getFileWininfo(fnames[0],&fpos)) {
  char tt[80];
  fontidx=fpos.fntidx;
  AscFontName=fontlist[fontidx];
  sprintf(tt,"%dx%d+%d+%d",fpos.wcol,fpos.wrow,fpos.wx,fpos.wy);
  geomstr=strdup(tt);
}

}



void change_win_name(char *str)
{
	XTextProperty name;

  if (XStringListToTextProperty(&str,1,&name) == 0) {
    error("change_window_name");
    return;
  }
  XSetWMName(display,main_win,&name);
  XStoreName(display,main_win,str);
  XSetIconName(display,main_win,str);
  XFree(name.value);
}

void getWinXY(int *x, int *y)
{
Window chl;
XTranslateCoordinates(display,main_win,DefaultRootWindow(display),0,0,x,y,
&chl);
}

void setWinSize(int row, int col)
{
  if (row>0 && col>0) {
    XResizeWindow(display,main_win,col*fwidth,row*fheight);
  }
}

void K_SetWinSize(row,col)
{
  setWinSize(col,row);
}

static int first_time=1;

void setWinPos(int x, int y)
{
Window ow;
int ox,oy;

if (first_time) { /* somtimes receives X errors */
	int cou=0;
	XWindowAttributes att;
	/* waiting until main_win shows up */
	do {
		XGetWindowAttributes(display, main_win, &att);
		if (att.map_state==IsViewable) break;
		XSync(display,False);
		usleep(50000);
	} while (cou<100);
	if (cou==100) return;
	first_time=0;
}
if (x<=0 && y<=0) return;
XTranslateCoordinates(display,main_win,DefaultRootWindow(display),0,0,&ox,&oy,
&ow);
XMoveWindow(display,ow,x,y);
}

void init_X_arg(int argc, char **argv)
{
int i;
char *display_name;

if (!(display_name=getenv("DISPLAY")))
	display_name=":0";

for(i=1;i<argc;i++)
	 if (!strcmp(argv[i],"-display") && i<argc-1)
	 {   display_name=argv[i+1]; break; }
if (!batch) {
	if (!(display = XOpenDisplay(display_name)))
		p_err("Cannot open display %s", display_name);
	XSetErrorHandler((XErrorHandler)XTedErrorHandler);
}

get_arg(argc,argv);
}

void AutoMouseFocus()
{
if (autofocusin)
XWarpPointer(display,None,main_win,0,0,0,0,
		fwidth*(XW_COL>>1),fheight*(XW_ROW>>1));
}

int owncmap=0;

void init_X_win(int argc, char **argv)
{
int our_depth;
Visual *our_visual;


root=DefaultRootWindow(display);
screen_no = DefaultScreen(display);

load_font(1);
if (owncmap) {
  XVisualInfo *list;
  XVisualInfo template;
  int nitems_return;
  Screen *screen;
  XVisualInfo *best,*list1;

install_cmap:
  best=NULL;
  screen = DefaultScreenOfDisplay(display);
  our_depth = DefaultDepthOfScreen(screen);
  our_visual = DefaultVisualOfScreen(screen);

  template.screen = screen_no;
  template.class = TrueColor;
  colormap = DefaultColormapOfScreen(screen);
  list = XGetVisualInfo(display, VisualScreenMask | VisualClassMask,
    &template, &nitems_return);

  for (list1 = list; list1 < list + nitems_return; ++list1)
    if (list1->depth > our_depth
      && (best == NULL || list1->depth > best->depth))
       best = list1;
    if (list!=NULL) {
      if (best!=NULL) {
        our_depth = best->depth;
        our_visual = best->visual;
        colormap = XCreateColormap(display,root,our_visual,AllocAll);
        colormap = XCopyColormapAndFree(display, colormap);
      }
      XFree(list);
    }
    if (our_visual->class == PseudoColor) {
      XColor tmp_color;
      u_long plane_masks[16],pixel;
      colormap = XCreateColormap(display,root,our_visual,AllocNone);
      alloc_pc_colors();
#if	1
	    colormap = XCopyColormapAndFree(display, colormap);
#endif
      XInstallColormap(display,colormap);
	  } else
    alloc_pc_colors();
} else {
  colormap = DefaultColormap(display,screen_no);
  if (alloc_pc_colors()<0)
    goto install_cmap;
}
create_win(argc,argv);

bzero(tat, sizeof(tat));
XDefineCursor(display,main_win,xcursor);

create_gc();
set_att(0);
wm_del_win = XInternAtom(display,"WM_DELETE_WINDOW",False);
XSetWMProtocols(display,main_win,&wm_del_win,1);
memset(tchd,255,sizeof(tchd));
X_inited=1;
XSelectInput(display, root, PropertyChangeMask);
atom_fname=XInternAtom(display,ted_atom,False);
atom_res=XInternAtom(display,ted_res,False);
init_xview();

XMapWindow(display,main_win);
}


int mult_open,ExitDupOpen;
int test_file(char *fdir,char *fname)
{
Window twin;
Atom atom;
Window win;
char tfname[128];
XClientMessageEvent event;
Atom actual_type;
int actual_format;
u_long nitems,bytes_after;
u_char *res;

if (strcmp(fdir,"/")) {
	strcpy(tfname,fdir);
	strcat(strcat(tfname,"/"),fname);
} else strcpy(tfname,fname);

XChangeProperty(display, root , atom_res, XA_STRING, 8,
		PropModeReplace, "    ",1);
XSync(display,False);

XChangeProperty(display, root , atom_fname, XA_STRING, 8,
		PropModeReplace, (u_char *)tfname, strlen(tfname)+1);
XSync(display,False);

usleep(100000);

if (XGetWindowProperty(display,root,atom_res, 0, 1,
	False, AnyPropertyType, &actual_type,&actual_format,
	&nitems,&bytes_after,&res) != Success)
	puts("err prop");
if (nitems==0) {
	return 0;
}

if (*res=='Y') {
	extern int readonly;
	char *tmp=event.data.b;

	message(4,"Already opened");
	XSync(display,False);
	if (ExitDupOpen && !file_no) {
	  p_err("Dup open %s",tfname);
	}
	return 1;
}

XFree(res);
return 0;
}

