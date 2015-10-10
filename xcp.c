/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include "ted.h"

extern Display *display;
extern Window main_win,root;
extern int input_handler;

Atom caret, clipboard, yield, length, lengthc, text, ctext;


void receive_string(u_char *s, int len)
{
if (!len) return;
switch (input_handler) {
	case 0:
		if (len) {
			s_fmod();
      file_modified=-1;
		}
		u_del_str(s,len,1);
		put_str(s,len);
		break;
	case 1:
		put_str_xgets(s,len);
		break;
}
}

void paste_primary(int win,int property,int Delete)
{
Atom actual_type;
int actual_format,i;
long nitem, bytes_after, nread;
unsigned char *data;

if (property == None)
	return;
nread = 0;
do   {
		if (XGetWindowProperty(display,win,property,nread/4,32768,Delete,
			   AnyPropertyType,&actual_type,&actual_format,
			   &nitem,&bytes_after,(unsigned char **)&data)
	!= Success) return;
		receive_string(data,nitem);
		nread += nitem;
		XFree(data);
} while (bytes_after > 0);
disp_fname();
}

void request_selection(Time time)
{
Window w;
Atom property;
if ((w=XGetSelectionOwner(display,XA_PRIMARY)) == None) {
	paste_primary(root,XA_CUT_BUFFER0,False);
	return;
}
property = XInternAtom(display,"VT_SELECTION",False);
XConvertSelection(display,XA_PRIMARY,XA_STRING,property, main_win,time);
}

char *selection_text;
int selection_length;
extern Time eve_time;

void setXselBuf()
{

XSetSelectionOwner(display,XA_PRIMARY,main_win,(Time)eve_time);
if (XGetSelectionOwner(display,XA_PRIMARY) != main_win)
  error("Could not set primary selection");
XChangeProperty(display,root,XA_CUT_BUFFER0,
      XA_STRING,8,PropModeReplace,selection_text,selection_length);
XSetSelectionOwner(display, caret, main_win, eve_time);
XSetSelectionOwner(display, clipboard, main_win, eve_time);
}

void set_selection()
{
	int i,j,k,len=0;

if (cur_file!=mark_holder) return;
if (mline_begin>=0) {
	for(i=mline_begin;i<=mline_end;i++)
		len+=edbf[i].len+1;

	if ((selection_text=(char *)mrealloc(selection_text,len+1,
			"set_selection 1"))==NULL) {
		return;
	}
	j=0;
	for(i=mline_begin;i<=mline_end;i++) {
		memcpy(&selection_text[j],edbf[i].cstr,edbf[i].len);
		j+=edbf[i].len;
		selection_text[j++]='\n';
	}
	selection_length=len;
} else
if (mblk_row0>=0) {
	for(i=mblk_row0;i<=mblk_row1;i++) {
		j=Min(mblk_col1, edbf[i].len-1)-mblk_col0+2;
		if (j>0) len+=j;
		else len++;
	}
	len--;
	if (len<=0) return;
	if ((selection_text=(char *)
	    mrealloc(selection_text,len,"set_selection 2"))==NULL) {
		error("set_selection 2");
		return;
	}
	j=0;
	for(i=mblk_row0;i<=mblk_row1;i++) {
		k=Min(mblk_col1, edbf[i].len-1)-mblk_col0+1;
		if (k > 0) {
			memcpy(&selection_text[j],&edbf[i].cstr[mblk_col0],k);
			j+=k;
		}
		if (i<mblk_row1) selection_text[j++]='\n';
	}
  selection_length=len;
}

setXselBuf();
}


void setSelectionStr(char *s)
{
selection_length=strlen(s);
if ((selection_text=(char *)mrealloc(selection_text,selection_length+1,
		"setSelectionStr"))==NULL) {
	return;
}
strcpy(selection_text,s);
setXselBuf();
}

void K_ExportStatus(char *s)
{
char tt[256],uu[16];
int i=0;

while (*s) {
  if (*s=='%' && *(s+1)) {
    switch (*(s+1)) {
      case 'f':
        memcpy(tt+i,fname,strlen(fname));
        i+=strlen(fname);
        s+=2;
        break;
      case 'y':
        sprintf(uu,"%d",cursor_row+1);
        strcpy(tt+i,uu);
        i+=strlen(uu);
        s+=2;
        break;
      default:
        tt[i++]=*(s++);
    }
  } else
  tt[i++]=*(s++);
}
tt[i]=0;
setSelectionStr(tt);
}

void init_xview()
{
clipboard = XInternAtom(display, "CLIPBOARD", False);
ctext = XInternAtom(display, "COMPOUND_TEXT", False);
length = XInternAtom(display, "LENGTH", False);
lengthc = XInternAtom(display, "LENGTH_CHARS", False);
caret = XInternAtom(display, "_SUN_SELN_CARET", False);
yield = XInternAtom(display, "_SUN_SELN_YIELD", False);
text = XInternAtom(display, "TEXT", False);
}

typedef CARD32  Atom32;
void send_selection(XSelectionRequestEvent *rq)
{
	XSelectionEvent notify;
	u_long data;
  Atom32          target_list[2];
  static Atom     xa_targets = None;

  if (xa_targets == None)
	  xa_targets = XInternAtom(display, "TARGETS", False);

  notify.type = SelectionNotify;
  notify.display = rq->display;
  notify.requestor = rq->requestor;
	notify.selection = rq->selection;
	notify.target = rq->target;
  notify.time = rq->time;

  if (rq->target == xa_targets) {
    target_list[0] = (Atom32) xa_targets;
    target_list[1] = (Atom32) XA_STRING;
    XChangeProperty(display, rq->requestor, rq->property, rq->target,
        (8 * sizeof(target_list[0])), PropModeReplace,
        (char *)target_list,
        (sizeof(target_list) / sizeof(target_list[0])));
    notify.property = rq->property;
  } else
	if (rq->selection == XA_PRIMARY) {
		if (rq->target == yield) {
			data = 1;
			XChangeProperty(display, rq->requestor, rq->property,
					rq->target, 32, PropModeReplace,
					(unsigned char *) &data, 1);
			notify.property = rq->property;
		}
		else {
      XChangeProperty(display, rq->requestor, rq->property,
          XA_STRING, 8, PropModeReplace,
          selection_text,selection_length);
      notify.property = rq->property;
		}
	}
	else if (rq->selection == caret) {
		if (rq->target == yield) {
			XSetSelectionOwner(display, caret, None,rq->time);
			data = 1;
			XChangeProperty(display, rq->requestor, rq->property,
					rq->target, 32, PropModeReplace,
					(unsigned char *) &data, 1);
			notify.property = rq->property;
		}
	}
	else if (rq->selection == clipboard) {
		if (rq->target == lengthc || rq->target == length) {
			data = selection_length;
			XChangeProperty(display, rq->requestor, rq->property,
					rq->target, 32, PropModeReplace,
					(unsigned char *) &data, 1);
			notify.property = rq->property;
		}
		else if (rq->target == XA_STRING ||
			rq->target == text || rq->target == ctext) {
			XChangeProperty(display, rq->requestor, rq->property,
				(rq->target == ctext? ctext: XA_STRING), 8,
				PropModeReplace, selection_text,selection_length );
			notify.property = rq->property;
		}
	}
	XSendEvent(display,rq->requestor,False,0,(XEvent *)&notify);
}
