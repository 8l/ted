/*
	Copyright (C)   1995
	Edward  Der-Hua Liu,    Hsin-Chu,       Taiwan
*/

#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ted.h"
#include "xkey.h"
#include "color.h"

KEYMAP kmap[]={
{"space",XK_space},{"!",XK_exclam},{"\"",XK_quotedbl},{"#",XK_numbersign},
{"$",XK_dollar},{"%",XK_percent},{"&",XK_ampersand},{"'",XK_quoteright},
{"(",XK_parenleft},{")",XK_parenright},{"*",XK_asterisk},{"+",XK_plus},
{",",XK_comma},{"minus",XK_minus},{".",XK_period},{"/",XK_slash},
{"0",XK_0},{"1",XK_1},{"2",XK_2},{"3",XK_3},{"4",XK_4},{"5",XK_5},{"6",XK_6},
{"7",XK_7},{"8",XK_8},{"9",XK_9},
{":",XK_colon},{";",XK_semicolon},{"<",XK_less},{"=",XK_equal},{">",XK_greater},
{"?",XK_question},{"@",XK_at},
{"A",XK_A},{"B",XK_B},{"C",XK_C},{"D",XK_D},{"E",XK_E},{"F",XK_F},{"G",XK_G},
{"H",XK_H},{"I",XK_I},{"J",XK_J},{"K",XK_K},{"L",XK_L},{"M",XK_M},{"N",XK_N},
{"O",XK_O},{"P",XK_P},{"Q",XK_Q},{"R",XK_R},{"S",XK_S},{"T",XK_T},{"U",XK_U},
{"V",XK_V},{"W",XK_W},{"X",XK_X},{"Y",XK_Y},{"Z",XK_Z},
{"brl",XK_bracketleft},{"\\",XK_backslash},{"brr",XK_bracketright},
{"^",XK_asciicircum},{"_",XK_underscore},{"`",XK_grave},
{"a",XK_a},{"b",XK_b},{"c",XK_c},{"d",XK_d},{"e",XK_e},{"f",XK_f},{"g",XK_g},
{"h",XK_h},{"i",XK_i},{"j",XK_j},{"k",XK_k},{"l",XK_l},{"m",XK_m},{"n",XK_n},
{"o",XK_o},{"p",XK_p},{"q",XK_q},{"r",XK_r},{"s",XK_s},{"t",XK_t},{"u",XK_u},
{"v",XK_v},{"w",XK_w},{"x",XK_x},{"y",XK_y},{"z",XK_z},
{"{",XK_braceleft}, {"|",XK_bar}, {"}",XK_braceright},{"~",XK_asciitilde},

{"adia",XK_adiaeresis}, {"Adia",XK_Adiaeresis},
{"odia",XK_odiaeresis}, {"Odia",XK_Odiaeresis},
{"udia",XK_udiaeresis}, {"Udia",XK_Udiaeresis},
{"eszet",XK_ssharp},

{"bs",XK_BackSpace},{"enter",XK_Return},{"tab",XK_Tab},

{"del",XK_Delete},{"home",XK_Home},{"end",XK_End},{"ins",XK_Insert},
{"pgup",XK_Prior},{"pgdn",XK_Next},
{"left",XK_Left},{"right",XK_Right},{"up",XK_Up},{"down",XK_Down},

{"f1",XK_F1},{"f2",XK_F2},{"f3",XK_F3},{"f4",XK_F4},{"f5",XK_F5},{"f6",XK_F6},
{"f7",XK_F7},{"f8",XK_F8},{"f9",XK_F9},{"f10",XK_F10},
{"f11",XK_F11},{"f12",XK_F12},{"f13",XK_F13},{"f14",XK_F14},{"f15",XK_F15},
{"f16",XK_F16},{"f17",XK_F17},{"f18",XK_F18},{"f19",XK_F19},{"f20",XK_F20},

{"kp0",XK_KP_0},{"kp1",XK_KP_1},{"kp2",XK_KP_2},{"kp3",XK_KP_3},{"kp4",XK_KP_4},
{"kp5",XK_KP_5},{"kp6",XK_KP_6},{"kp7",XK_KP_7},{"kp8",XK_KP_8},{"kp9",XK_KP_9},
{"kpdivide",XK_KP_Divide},{"kpmult",XK_KP_Multiply},{"kpminus",XK_KP_Subtract},
{"kpplus",XK_KP_Add},{"kpenter",XK_KP_Enter},{"kpdot",XK_KP_Decimal},
{"escape",XK_Escape}, {"pause",XK_Pause}, {"print",XK_Print},
#ifdef SUNKBD
{"cancel",XK_Cancel},{"sunprops",0x1005ff70},{"sunfront",0x1005ff71},
{"suncopy",0x1005ff72},
{"sunopen",0x1005ff73},{"sunpaste",0x1005ff74},{"suncut",0x1005ff75},
{"find",XK_Find},
{"undo",XK_Undo},{"redo",XK_Redo},
#endif
};
int kname_no=sizeof(kmap)/sizeof(kmap[0]);

extern K_return(),K_cur_left(),K_cur_right(),K_cur_up(),K_cur_down();
extern K_pgup(),K_pgdn(),K_home(),K_end(),K_quit(),K_del(),K_DelLn();
extern K_pgtop(),K_pgbottom(),K_beginfile(),K_endfile(),K_BS();
extern K_DelEol(),K_DelWord(),K_CpArea(),K_SaveFile(),K_SwWin();
extern K_PrevFile(),K_NextFile(),K_MarkLn(),K_MarkBlk(),K_Unmark(),K_DelArea();
extern K_OwCp(),K_MvArea(),K_EdFile(),K_TogIns(),K_SplitWin();
extern K_FindStr(),K_RepStr(),K_WrFile(),K_TogIns(),K_FindStr();
extern K_RepStr(),K_WrFile(),K_Undo(),K_Redo(),K_system(),K_SetTag();
extern K_GotoTag(),K_JoinLine(),K_ExtMarkUp(),K_ExtMarkDown();
extern K_ExtMarkLeft(),K_ExtMarkRight(),K_MvArea(),K_RepFind();
extern K_GotoRow(),K_ReadPipe(),K_MarkWord(),K_ExecMarkWord(),K_PutStr();
extern K_EditMarkWord(),K_EditFName(),K_LoadTedrc(),K_SaveIfModified();
extern K_CD(),K_ChDir(),K_InsFile(),K_IncFile(),K_SetHotFile(),K_TgHotFile();
extern K_InsPipe(),K_LookupTag(),K_ExecCmd(),K_CurLeftWord(),K_CurRightWord();
extern K_UpLowBlk(),K_Compile(),K_TogCpEdit(),K_CloseCpWin(),K_SystemFname();
extern K_SetWordWrap(),K_RaiseOrLoad(),K_Reload(),K_ReturnIndent();
extern K_TogIndent(),K_ExeCmdFnameExt(),K_DelBol(),K_BShiftUpDn(),K_BShiftLR();
extern K_spgup(),K_spgdn(),K_MergeTedrc(),K_LookMarkTag(),K_FindMarkStr();
extern K_Dup1stToEndBlk(),K_MatchBracket(),K_RecMacro(),K_PlayMacro();
extern K_FileBrowser(),K_PrevFind(),K_KeywordComp(),K_SwitchKeyDef();
extern K_PutStrSave(),K_ToPasteBuff(),K_PasteBuffPut(),K_DelLnN();
extern K_cur_upN(),K_cur_downN(),K_SaveNLtoPaste(),K_PasteBuffPutAfter();
extern K_delvi(),K_SaveEoltoPaste(),K_PasteBuffer(),K_SaveWordtoPaste();
extern K_WriteBlk(),K_GotoGrep(),K_Tab2Space(),K_TogVertical();
extern K_LocateStr(),K_ReplaceStr(),K_ExportMacro(),K_TogReadOnly();
extern K_VisitedFile(),K_quit2(),K_SwitchFont(),K_TogDispTab(),K_SetWinSize();
extern K_ExportStatus(),K_SumRectBlock();
/*
 field 1 :
 field 2 :
 field 3 : arg type
           0 : no arg
           1 : char     (integer)
           2 : string
           3 : string string
           7 : string string string
           8 : int int
 field 4 : If eq 1, this func will modify text
*/

FUNCMAP func[]={
{"PutStr",K_PutStr,2,       1},     /* PutStr must be the 1st one */
{"RecMacro",K_RecMacro,0},      /* fixed to 1 */
{"PlayMacro",K_PlayMacro,0},    /* fixed to 2 */
{"SaveCurFuncs",K_PlayMacro,5}, /*          3 */
{"PlaySaveFuncs",K_PlayMacro,6},/*          4 */
{"enter",K_ReturnIndent,0,  1},
{"up",K_cur_up,0},
{"down",K_cur_down,0},
{"left",K_cur_left,0},
{"right",K_cur_right,0},
{"pgup",K_pgup,0},
{"pgdn",K_pgdn,0},
{"spgup",K_spgup,0},
{"spgdn",K_spgdn,0},
{"pgtop",K_pgtop,0},
{"pgbottom",K_pgbottom,0},
{"beginfile",K_beginfile,0},
{"endfile",K_endfile,0},
{"home",K_home,0},
{"end",K_end,0},
{"quit",K_quit,0},
{"quit2",K_quit2,0},
{"bs",K_BS,0,1},
{"del",K_del,0,1},
{"DelLn",K_DelLn,0,       1},
{"DelEol",K_DelEol,0,     1},
{"DelWord",K_DelWord,0,   1},
{"CpArea",K_CpArea,0,     1},
{"SaveFile",K_SaveFile,0, 1},
{"SwWin", K_SwWin,0},
{"PrevFile",K_PrevFile,0},
{"NextFile",K_NextFile,0},
{"MarkLn",K_MarkLn,0},
{"MarkBlk",K_MarkBlk,0},
{"Unmark",K_Unmark,0},
{"DelArea",K_DelArea,0,   1},
{"OwCp",K_OwCp,0,         1},
{"MvArea",K_MvArea,0,     1},
{"EdFile",K_EdFile,0},
{"TogIns",K_TogIns,0,     1},
{"SplitWin",K_SplitWin,0},
{"FindStr",K_FindStr,0},
{"RepStr",K_RepStr,0},
{"WrFile",K_WrFile,0},
{"Undo",K_Undo,0,         1},
{"Redo",K_Redo,0,         1},
{"system",K_system,2},
{"SetTag",K_SetTag,1},
{"GotoTag",K_GotoTag,1},
{"JoinLine",K_JoinLine,0,   1},
{"ExtMarkUp",K_ExtMarkUp,0},
{"ExtMarkDown",K_ExtMarkDown,0},
{"ExtMarkLeft",K_ExtMarkLeft,0},
{"ExtMarkRight",K_ExtMarkRight,0},
{"MvArea",K_MvArea,0,1},
{"RepFind",K_RepFind,0},
{"GotoRow",K_GotoRow,0},
{"ReadPipe",K_ReadPipe,2},
{"MarkWord",K_MarkWord,2},
{"ExecMarkWord",K_ExecMarkWord,3},
{"EditMarkWord",K_EditMarkWord,0},
{"EditFName",K_EditFName,2},
{"LoadTedrc",K_LoadTedrc,2},
{"SaveIfModified",K_SaveIfModified,0},
{"CD",K_CD,2},
{"ChDir",K_ChDir,0},
{"InsFile",K_InsFile,2},
{"IncFile",K_IncFile,0},
{"SetHotFile",K_SetHotFile,0},
{"TgHotFile",K_TgHotFile,0},
{"InsPipe",K_InsPipe,2,1},
{"LookupTag",K_LookupTag,0},
{"ExecCmd",K_ExecCmd,0,0},
{"CurLeftWord",K_CurLeftWord,0},
{"CurRightWord",K_CurRightWord,0},
{"UpLowBlk",K_UpLowBlk,1},
{"Compile",K_Compile,2},
{"TogCpEdit",K_TogCpEdit,0},
{"CloseCpWin",K_CloseCpWin,0},
{"SystemFname",K_SystemFname,2},
{"SetWordWrap",K_SetWordWrap,0},
{"RaiseOrLoad",K_RaiseOrLoad,2},
{"Reload",K_Reload,0},
{"TogIndent",K_TogIndent,0},
{"ExeCmdFnameExt",K_ExeCmdFnameExt,3},
{"DelBol",K_DelBol,0,1},
{"BShiftUpDn",K_BShiftUpDn,1,   1},
{"BShiftLR",K_BShiftLR,1,       1},
{"MergeTedrc",K_MergeTedrc,2},
{"LookMarkTag",K_LookMarkTag,0},
{"FindMarkStr",K_FindMarkStr,0},
{"Dup1stToEndBlk",K_Dup1stToEndBlk,0,   1},
{"MatchBracket", K_MatchBracket,0},
{"FileBrowser", K_FileBrowser,0},
{"PrevFind", K_PrevFind,0},
{"KeywordComp", K_KeywordComp,0},
{"SwitchKeyDef", K_SwitchKeyDef,1},
{"PutStrSave", K_PutStrSave,6},
{"ToPasteBuff",K_ToPasteBuff,0},
{"PasteBuffPut",K_PasteBuffPut,0},
{"PasteBuffPutAfter",K_PasteBuffPutAfter,0},
{"DelLnN",K_DelLnN,1,                   1},
{"upN",K_cur_upN,1},
{"downN",K_cur_downN,1},
{"SaveNLtoPaste",K_SaveNLtoPaste,1},
{"delvi",K_delvi,0,                     1},
{"SaveEoltoPaste",K_SaveEoltoPaste,0},
{"PasteBuffer",K_PasteBuffer,0},
{"SaveWordtoPaste",K_SaveWordtoPaste,0},
{"WriteBlk",K_WriteBlk,0},
{"GotoGrep",K_GotoGrep,1},
{"Tab2Space",K_Tab2Space,0},
{"TogVertical",K_TogVertical,0},
{"LocateStr",K_LocateStr,3},
{"ReplaceStr",K_ReplaceStr,7,           1},
{"ExportMacro",K_ExportMacro,2},
{"TogReadOnly",K_TogReadOnly,0},
{"VisitedFile",K_VisitedFile,2},
{"SwitchFont",K_SwitchFont,0},
{"TogDispTab",K_TogDispTab,0},
{"SetWinSize",K_SetWinSize,8},
{"ExportStatus",K_ExportStatus,2},
{"SumRectBlock",K_SumRectBlock,0},
};

int funcno=sizeof(func)/sizeof(func[0]);

int MAX_KEY_DEF=280;
KEYDEF *kdefmap[4];
KEYDEF *ckdef=NULL;
int kdefhash[4][128];
int *ckdefhash;
int ckdefno, cmapno;
int kdefno_m[4];
char *mode_str_m[4];
char *mode_str;

extern ColorDef pc_color[];

struct {
char *cname;
char *caddr;
} locolor[]={
{"TextFg",      pc_color[0].fg_str},
{"TextBg",      pc_color[0].bg_str},
{"CursorFg",    pc_color[1].fg_str},
{"CursorBg",    pc_color[1].bg_str},
{"SepLineFg",   pc_color[2].fg_str},
{"SepLineBg",   pc_color[2].bg_str},
{"MarkFg",      pc_color[3].bg_str},
{"MarkBg",      pc_color[3].bg_str},
{"EnterStrFg",  pc_color[6].fg_str},
{"EnterStrBg",  pc_color[6].bg_str},
{"EnterBorderFg", pc_color[7].fg_str},
{"EnterBorderBg", pc_color[7].bg_str},
{"EnterRectBg",  pc_color[8].bg_str},
{"EnterFirstFg",  pc_color[9].fg_str},
{"EnterFirstBg",  pc_color[9].bg_str},
{"StatusWarnFg", pc_color[4].fg_str},
{"StatusWarnBg", pc_color[4].bg_str},
{"StatusFSavedFg", pc_color[5].fg_str},
{"StatusFSavedBg", pc_color[5].bg_str},
{"BracketFg", pc_color[10].fg_str},
{"BracketBg", pc_color[10].bg_str},
{"MouseFg", pc_color[11].fg_str},
{"MouseBg", pc_color[11].bg_str},
{"OvwCursorFg", pc_color[12].fg_str},
{"OvwCursorBg", pc_color[12].bg_str}
};

extern int auto_save_time;
extern int wrap_width, LeadingSpaceTab,Indent,BackupSave,TrimBlankTail;
extern int CurAutoUpDown,StripCR,CComp,WinSplitSeq,CursorFollowClick;
extern int AutoBlockRepDel,RestCursor,RestWinPos,RestWinSize,ExitDupOpen;
extern int FModifiedQuitYN,DispTab,TabChar;
struct {
char *sname;
int *saddr;
} sets[]=
{
{"autosave",&auto_save_time},
{"WrapWidth",&wrap_width},
{"TabWidth",&tab_width},
{"LeadSpaceTab",&LeadingSpaceTab},
{"Indent",&Indent},
{"Insert",&insert_mode},
{"Backup",&BackupSave},
{"TrimBlankTail",&TrimBlankTail},
{"CurAutoUpDownAtEdge",&CurAutoUpDown},
{"StripCR",&StripCR},
{"Compiler",&CComp},
{"WinSplitSeq",&WinSplitSeq},
{"CursorFollowClick",&CursorFollowClick},
{"AutoBlockRepDel",&AutoBlockRepDel},
{"RestCursor",&RestCursor},
{"RestWinSize",&RestWinSize},
{"RestWinPos",&RestWinPos},
{"ExitDupOpen",&ExitDupOpen},
{"FModifiedQuitYN",&FModifiedQuitYN},
{"DispTab",&DispTab},
{"TabChar",&TabChar},
};

char *tedrcfname="tedrc";
void lper(char *tedname, int lno, char *fmt,...)
{
va_list args;

va_start(args, fmt);
fprintf(stderr,"'%s' line %d:", tedname, lno);
vfprintf(stderr, fmt, args);
va_end(args);
fprintf(stderr,"\n");
exit(-1);
}

void lper2(char *tedname, int lno, char *fmt,...)
{
va_list args;

va_start(args, fmt);
fprintf(stderr,"'%s' line %d:", tedname, lno);
vfprintf(stderr, fmt, args);
va_end(args);
fprintf(stderr,"\n");
}

char *skip_spc(char *s)
{
while (*s==' '||*s==9) s++;
return s;
}

char *to_spc(char *s)
{
while (*s!=' ' && *s!=9 && *s) s++;
return s;
}

int hex2b(char h)
{
return (h>='a'?h-'a'+10:h-'0');
}

void mkstr(char *aa,char *bb,char *tt)
{
  int len;
  memcpy(tt,aa,len=bb-aa);
  tt[len]=0;
}

char *varname(char *s,char *fn)
{
  char *t;
  int len;
  t=s=skip_spc(s);
  if (!isvarchr(*s)) return 0;
  while (isvarchr(*t)||isdig(*t)) t++;
  mkstr(s,t,fn);
  return t;
}


char *varname_sh(char *s,char *fn)
{
  char *t;
  int len;
  char uu[256];
  char *env;

  t=s+1;
  if (!isvarchr(*t)) {
skip:
    strcpy(fn,"$");
    return s+1;
  }
  while (isvarchr(*t)||isdig(*t))
    t++;
  mkstr(s+1,t,uu);
  if (env=getenv(uu)) {
    strcpy(fn,env);
    return t;
  } else {
    if (!strcmp(uu,"TEDDIR")) {
      strcpy(fn,TEDDIR);
      return t;
    } else
    if (!strcmp(uu,"DBGSTOP")) {
#ifdef	__GNUC__
      strcpy(fn,"b");
#else
      strcpy(fn,"stop at");
#endif
      return t;
    }
    else
      goto skip;
  }
}


char *parse_Cstr(char *tedname, int lno, char *ss, char **end)
{
  static char tt[1024];
  int sl=0;
  if (*ss!='"') lper(tedname,lno,"Bad string %s",ss);
  ss++;
  while (*ss && *ss!='"') {
    if (*ss=='\\') {
      switch (*(ss+1)) {
        case 'n':
          tt[sl++]='\n';
          ss+=2;
          continue;
        case 'r':
          tt[sl++]='\r';
          ss+=2;
          continue;
        case '\\':
          tt[sl++]='\\';
          ss+=2;
          continue;
        case 't':
          tt[sl++]='\t';
          ss+=2;
          continue;
        case '"':
          tt[sl++]='"';
          ss+=2;
          continue;
        case 'v':
          tt[sl++]='\v';
          ss+=2;
          continue;
        case 'f':
          tt[sl++]='\f';
          ss+=2;
          continue;
        case 'a':
          tt[sl++]='\a';
          ss+=2;
          continue;
        case 'e':
          tt[sl++]=27;
          ss+=2;
          continue;
        case 'x':
          ss+=2;
          tt[sl++]=(hex2b(*ss)<<4)+hex2b(*(ss+1));
          ss+=2;
          continue;
        case '$':
          tt[sl++]='$';
          ss+=2;
          continue;
      }
    } else
    if (*ss=='$') {
      char uu[1024];
      int l;
      ss=varname_sh(ss,uu);
      l=strlen(uu);
      memcpy(&tt[sl],uu,l);
      sl+=l;
      continue;
    }
    tt[sl++]=*(ss++);
  }
  if (!(*ss)) lper(tedname,lno,"\" expected");
  tt[sl]=0;
  *end=ss+1;
  return tt;
}

static parse_key(char *tedname, int lno, char *uu,char *vv,u_short *oo)
{
int i,tk=0;
char *ww,*xx,*yy;
u_short kmod,kname;

while (*uu && uu<vv) {
  if (*uu!='[') lper(tedname,lno,"cannot find the 1st [");
  xx=++uu;
  kmod=0;
  yy=xx;
  while (*yy && *yy!='-' && *yy!=']') yy++;
  while (*xx!='-' && *xx!=']' && xx<vv) {
    if (*yy=='-')
    switch (*xx) {
    case 'c':
      kmod|=CTRL;
      break;
    case 'a':
      kmod|=ALT;
      break;
    case 's':
      kmod|=SHFT;
      break;
    }
    xx++;
  }
  if (!kmod) {
    if (*xx==']') {
      ww=xx;
      *xx=0;
    }
    else lper(tedname,lno,"cannot find ]");
    xx=uu;
  }
  else
  if (*xx=='-') {
    ww=++xx;
    while (*ww!=']' && ww<vv) ww++;
    if (*ww!=']') lper(tedname,lno,"cannot find ]");
    else *ww=0;
  } else lper(tedname,lno,"error");
  for(i=0;i<kname_no;i++)
  if (!strcmp(xx,kmap[i].keyname)){
    kname=i;
    break;
  }
  if (i==kname_no) lper(tedname,lno,"unknown key %s",xx);
  kname|=kmod;
  oo[tk++]=kname;
  uu=ww+1;
}
return tk;
}

isvarchr(int ch)
{
ch=toupper(ch);
return (ch>='A' && ch<='Z' || ch=='_');
}

isdig(int ch)
{ return (ch>='0' && ch<='9');
}

isvardig(int ch)
{
return (isvarchr(ch)||isdig(ch));
}

ispc(char c)
{
return (c==' ' || c==9);
}


u_char takearg(char *tedname, int lno, char *ss, char **ostr, char **next)
{
  static char ooo[64];
  char *tt, *env;
  char *aa;
  ss=skip_spc(ss);
  *next=tt=ss;
  if (!*ss) return 0;
  if (isdig(*tt)) {
    while (isdig(*tt)) tt++;
    mkstr(ss,*next=tt,*ostr=ooo);
    return 1;
  }
  else
  if (*tt=='"') {
    *ostr=parse_Cstr(tedname, lno, tt,next);
    return 2;
  }
  return 0;
}

char *getstr(char *s, char **t)
{
  static char tt[80];
  char *u;
  while (ispc(*s) || *s==',') s++;
  u=s;
  while (isvarchr(*u) || u>s && isdigit(*u)) u++;
  mkstr(s,u,tt);
  *t=u;
  return tt;
}

parse_stmt(char *tedname, int lno,  char *ss,char **cmd, u_char *count)
{
char fn[32], *arg,*tt;
char tbuf[1024];
u_short funidx[64],fuid;
int typ,cmlen=0,fncnt=0,argtyp,ii;
while (*ss) {
  if (!(ss=varname(ss,fn))) lper(tedname,lno,"func name expected");
  for(fuid=0;fuid<funcno;fuid++)
    if (!strcmp(fn,func[fuid].funname))
      break;
  if (fuid==funcno) {
    lper2(tedname,lno,"func %s is not defined",fn);
    return 0;
  }
  ss=skip_spc(ss);
  if (*ss!='(') lper(tedname,lno,"( expected");
  ss++;
  funidx[fncnt++]=cmlen;
  argtyp=func[fuid].argtype;
  tbuf[cmlen++]=fuid;
  switch (argtyp) {
  case 0:
  case 4:
  case 5:
  case 6:
    typ=takearg(tedname, lno, ss,&arg,&tt);
    if (typ!=0)
      lper(tedname,lno,"No arg is expected");
    break;
  case 1:
    typ=takearg(tedname, lno, ss,&arg,&tt);
    if (typ!=1)
      lper(tedname, lno, tedname,lno,"integer is expected");
    tbuf[cmlen++]=atoi(arg);
    break;
  case 2:
    typ=takearg(tedname, lno, ss,&arg,&tt);
    if (typ!=2)
      lper(tedname,lno,"String is expected");
    strcpy(&tbuf[cmlen],arg);
    cmlen+=strlen(arg)+1;
    break;
  case 3:  /* (string,string) */
    typ=takearg(tedname, lno,ss,&arg,&tt);
    if (typ!=2)
      lper(tedname,lno,"String is expected");
    strcpy(&tbuf[cmlen],arg);
    cmlen+=strlen(arg)+1;
    tt=skip_spc(tt);
    if (*tt!=',') lper(tedname,lno,", is expected");
    tt++;
    typ=takearg(tedname, lno,tt,&arg,&tt);
    if (typ!=2)
      lper(tedname,lno,"String is expected");
    strcpy(&tbuf[cmlen],arg);
    cmlen+=strlen(arg)+1;
    break;
  case 7:  /* (string,string,string) */
    typ=takearg(tedname, lno,ss,&arg,&tt);
    if (typ!=2)
      lper(tedname,lno,"String is expected");
    strcpy(&tbuf[cmlen],arg);
    cmlen+=strlen(arg)+1;
    tt=skip_spc(tt);
    if (*tt!=',') lper(tedname,lno,", is expected");
    tt++;
    typ=takearg(tedname, lno,tt,&arg,&tt);
    if (typ!=2)
      lper(tedname,lno,"String is expected");
    strcpy(&tbuf[cmlen],arg);
    cmlen+=strlen(arg)+1;
    tt=skip_spc(tt);
    if (*tt!=',') lper(tedname,lno,", is expected");
    tt++;
    typ=takearg(tedname, lno,tt,&arg,&tt);
    if (typ!=2)
      lper(tedname,lno,"String is expected");
    strcpy(&tbuf[cmlen],arg);
    cmlen+=strlen(arg)+1;
    break;
  case 8: /* int int */
    typ=takearg(tedname, lno, ss,&arg,&tt);
    if (typ!=1)
      lper(tedname, lno, tedname,lno,"integer is expected");
    ii=atoi(arg);
    memcpy(&tbuf[cmlen],&ii,sizeof(int));
		cmlen+=sizeof(int);
		tt=skip_spc(tt);
		if (*tt!=',') lper(tedname,lno,", is expected");
		tt++;
		ss=skip_spc(tt);
		typ=takearg(tedname, lno, ss,&arg,&tt);
		if (typ!=1)
			lper(tedname, lno, tedname,lno,"integer is expected");
    ii=atoi(arg);
    memcpy(&tbuf[cmlen],&ii,sizeof(int));
		cmlen+=sizeof(int);
	}

	ss=skip_spc(tt);
	if (*ss!=')')
		lper(tedname,lno," ) expected");
	ss++;
	ss=skip_spc(ss);
	if (*ss==';') ss++;
}

if ((*cmd=(char *)malloc(cmlen))==NULL)
	lper(tedname,lno,"malloc err");
*count=fncnt;
memcpy(*cmd,tbuf,cmlen);
return 1;
}

static qcmp(const void *aa, const void *bb)
{
KEYDEF *a=(KEYDEF *)aa;
KEYDEF *b=(KEYDEF *)bb;
int i,mlen=Min(a->klen,b->klen);
int res;
for(i=0;i<mlen;i++)
	if (res=a->kcode[i]-b->kcode[i]) break;
if (i==mlen) return(a->klen - b->klen);
return res;
}

static qcmpxk(const void *a, const void *b)
{
return(((KEYMAP *)a)->xkey - ((KEYMAP *)b)->xkey);
}

int file_exist(char *s)
{
FILE *fp;
if (fp=fopen(s,"r")) {
	fclose(fp);
	return 1;
}
return 0;
}

int cdtedrc=1;
FILE *init_tedrc_fname(char *ftedrc, int depth)
{
FILE *fp;
char tt[256],uu[256];
#if	0
printf(" %d %s\n",depth, ftedrc);
#endif
if (cdtedrc && (fp=fopen(ftedrc,"r"))) {
	return fp;
}


if (fp=fopen(
	strcat(strcat(strcpy(tt,(char *)getenv("HOME")),"/.Ted/"),ftedrc), "r"
)) 	return fp;
if (fp=fopen(strcat(strcat(strcpy(uu, TEDDIR),"/"),ftedrc),"r"))
	return fp;
if (!depth) {
  FILE *tfp;
  extern char *exec_name;
  static char exepath[128];
  int l;

  if (exepath[0]) {
    strcpy(tt,exepath);
  } else {
    sprintf(tt,"which %s",exec_name);
    if ((tfp=popen(tt,"r"))==NULL)
      p_err("cannot locate exec %s path",exec_name);
    fgets(tt,sizeof(tt),tfp);
    if ((l=strlen(tt))>0) {
      int i=l-1;
      while (i>=0 && tt[i]!='/') i--;
      if (i>=0) tt[i]=0;
      strcpy(exepath,tt);
    }
  }
  if (fp=fopen(strcat(strcat(strcpy(uu, tt),"/"),ftedrc),"r"))
    return fp;
  if ((l=strlen(tt))>0) {
    int i=l-1;
    while (i>=0 && tt[i]!='/') i--;
    if (i>=0) tt[i]=0;
    strcat(tt,"/Ted");
    if (fp=fopen(strcat(strcat(strcpy(uu, tt),"/"),ftedrc),"r"))
      return fp;
  }
	if (cdtedrc)
		p_err("Cannot find ./%s or %s or %s",ftedrc,tt,uu);
	else
		p_err("Cannot find %s or %s",ftedrc,tt,uu);
}
return fp;
}

extern int LangN;
extern char *LangExt[];
extern int ColorCnt;

extern KW *LangKw[];
extern int LangKwN[], LangMaxKeywordLen[];


void free_lang()
{
int i,j,k;
for (i=0;i<LangN;i++) {
	free(LangExt[i]);
	for(j=3;j<LangKwN[i];j++) {
		free(LangKw[i][j].keyword);
		if (LangKw[i][j].arg) free(LangKw[i][j].arg);
	}
	free(LangKw[i]);
}
ColorCnt=13;
LangN=0;
}

void incr_kdefno(KEYDEF **kd, int kdefmapno, int *kdno)
{
KEYDEF *kdef=*kd;
int kdefno=*kdno;
kdefno++;
if (kdefno >= MAX_KEY_DEF) {
	MAX_KEY_DEF+=50;
	if ((kdef=mrealloc(kdef,MAX_KEY_DEF*sizeof(KEYDEF),"incr_kdefno"))==NULL)
		exit(1);
	kdefmap[kdefmapno]=kdef;
}
*kd=kdef;
*kdno=kdefno;
kdefno_m[kdefmapno]=kdefno;
}

int kcode_exist(int x, KEYDEF *kdef, int kdefno)
{
int i;
for(i=0;i<kdefno;i++) {
	if (kdef[x].klen==kdef->klen &&
			!memcmp(kdef[i].kcode,kdef[x].kcode,kdef->klen*sizeof(u_short)))
		break;
}
return i;
}

static int kwcmp(const void *a, const void *b)
{
  return strcmp(((KW *)a)->keyword,((KW *)b)->keyword);
}

static int TedStart=1;
int ReLoadTed=0;
LoadTedrc(char *tedname, int depth, int kdefmapno)
{
FILE *fp;
char ss[8192],ibuf[2048],*tt,*uu,*vv,*ww,*xx;
int len,i,j,lno;
int hdef[128];
extern int X_inited;
KEYDEF *kdef=kdefmap[kdefmapno];
int kdefno=kdefno_m[kdefmapno];

if ((fp=init_tedrc_fname(tedname, depth))==NULL)
	return 1;

if (kdefno && !depth) {
	for(j=0;j<kdefno;j++) free(kdef[j].cmd);
	if (!kdefmapno) free_lang();
	free(kdef);
	kdefmap[kdefmapno]=NULL;
	if (kdefmapno) {
		free(mode_str_m[kdefmapno]);
		mode_str_m[kdefmapno]=NULL;
	}
	kdef=NULL;
}
#if	0
printf("-- %d %d %s\n",depth, kdefmapno, ftedrc);
#endif
if (kdef==NULL) {
	MAX_KEY_DEF=280;
	if ((kdef=mmalloc(sizeof(KEYDEF)*MAX_KEY_DEF,"LoadTedrc"))==NULL)
		exit(1);
	kdefmap[kdefmapno]=kdef;
	kdefno_m[kdefmapno]=kdefno=0;
}


if (!depth) qsort(kmap,kname_no,sizeof(KEYMAP),qcmpxk);
tedrcfname=tedname;
lno=0;
while (!feof(fp)) {
	int next_l;

	ss[0]=0;
	next_l=0;
	for(;;) {
		lno++;
		len=mfgets(ibuf,sizeof(ibuf),fp);
		if (!len) {
			next_l=1;
			break;
		}
		if (ibuf[len-1]=='\n') ibuf[--len]=0;
		if (!len) {
			next_l=1;
			break;
		}
		while (len && (ibuf[len-1]==' '||ibuf[len-1]==9)) len--;
		if (!len) {
			next_l=1;
			break;
		}
		if (ibuf[len-1]=='\\') {
			ibuf[len-1]=0;
			strcat(ss,ibuf);
		} else {
			strcat(ss,ibuf);
			break;
		}
	}
	if (next_l) continue;

	tt=skip_spc(ss);
	uu=to_spc(tt);
	vv=skip_spc(uu);
	*uu=0;
	if (tt[0]=='#')	continue;
	if (!strcmp(tt,"d")) {
		ww=to_spc(vv);
		if (!(*vv)) lper(tedname,lno,"argument expected");
		kdef[kdefno].klen=parse_key(tedname, lno,vv,ww,kdef[kdefno].kcode);
		if ((i=kcode_exist(kdefno,kdef,kdefno))<kdefno)
			free(kdef[i].cmd);
		xx=skip_spc(ww);
		kdef[i].cmd=strdup(parse_Cstr(tedname, lno,xx,&uu));
		kdef[i].type=0;
		if (i==kdefno) incr_kdefno(&kdef,kdefmapno,&kdefno);
		continue;
	} else
	if (!strcmp(tt,"f")) {
		ww=to_spc(vv);
		if (!(*vv)) lper(tedname,lno,"argument expected");
		kdef[kdefno].klen=parse_key(tedname, lno,vv,ww,kdef[kdefno].kcode);
		if ((i=kcode_exist(kdefno,kdef,kdefno))<kdefno)
			free(kdef[i].cmd);
		xx=skip_spc(ww);
		if (parse_stmt(tedname, lno,xx,&kdef[i].cmd,&kdef[i].type)) {
			if (i==kdefno) incr_kdefno(&kdef,kdefmapno,&kdefno);
		}
		continue;
	}
	if (kdefmapno)
		lper(tedname,lno,"%s for LoadKeyDefInto can accept 'f' and 'd' command only", tedname);
	if (!strcmp(tt,"s")) {
		ww=to_spc(vv);
		xx=skip_spc(ww);
		*ww=0;
		for(i=0;i<sizeof(locolor)/sizeof(locolor[0]);i++)
			if (!strcmp(vv,locolor[i].cname)) break;
		if (i==sizeof(locolor)/sizeof(locolor[0]))
			lper(tedname,lno,"Unknown Def color %s",vv);
		ww=to_spc(xx);
		*ww=0;
		strcpy(locolor[i].caddr,xx);
	} else
	if (!strcmp(tt,"set")) {
		ww=to_spc(vv);
		xx=skip_spc(ww);
		tt=to_spc(xx);
		if (!(*ww)) lper(tedname,lno,"argument expected");
		*ww=0;
		for(i=0;i<sizeof(sets)/sizeof(sets[0]);i++)
		if (!strcmp(sets[i].sname,vv)) break;
		if (i==sizeof(sets)/sizeof(sets[0]))
			lper2(tedname,lno,"Unknown set %s",vv);
		else {
		  *tt=0;
		  *(sets[i].saddr)=atoi(xx);
		}
	} else
	if (!strcmp(tt,"lang") && !depth) {
		extern char DirectCommentChars[];
		int keywcnt=3,LangIdx,ColIdx,AllocN=32,kwidx, MaxKeywordLen=0;
		char *ff;
		KW *kw;

		if (depth)
			p_err("cannot define 'lang' in include %s", tedname);


		tt=parse_Cstr(tedname, lno,vv,&uu);
		i=strlen(tt)+2;
		if ((vv=mmalloc(i,"LoadTedrc"))==NULL) exit(1);
		if ((kw=mmalloc(AllocN*sizeof(KW),"LoadTedrc 2"))==NULL) exit(1);

		LangExt[LangN]=strcat(strcpy(vv,tt)," ");
		kw[0].coloridx=ColorCnt;
		strcpy(pc_color[ColorCnt].fg_str,getstr(uu,&vv));
		strcpy(pc_color[ColorCnt++].bg_str,getstr(vv,&uu));
		uu=skip_spc(uu);
		DirectCommentChars[LangN]=*uu;
		kw[1].coloridx=ColorCnt;
		strcpy(pc_color[ColorCnt].fg_str,getstr(uu+1,&vv));
		strcpy(pc_color[ColorCnt++].bg_str,getstr(vv,&uu));
		kw[2].coloridx=ColorCnt;
		strcpy(pc_color[ColorCnt].fg_str,getstr(uu+1,&vv));
		strcpy(pc_color[ColorCnt++].bg_str,getstr(vv,&uu));
		for(;;) {
			if (!*uu) break;
			strcpy(pc_color[ColorCnt].fg_str,getstr(uu+1,&vv));
			strcpy(pc_color[ColorCnt].bg_str,getstr(vv,&uu));
			uu=skip_spc(uu);
			ww=tt=parse_Cstr(tedname, lno,uu,&vv);
			uu=vv;
			xx=tt+strlen(tt);
			for(;;) {
				char *pstr;

				tt=to_spc(ww);
				vv=skip_spc(tt);
				*(tt)=0;
				if (pstr=strchr(ww,'(')) {
					kw[keywcnt].arg=strdup(pstr);
					*pstr=0;
				} else kw[keywcnt].arg=0;
				kw[keywcnt].keyword=strdup(ww);
				if (strlen(ww) > MaxKeywordLen) MaxKeywordLen=strlen(ww);
				kw[keywcnt].coloridx=ColorCnt;
#if	0
				printf("%d %d   %d %s\n",LangN,keywcnt, ColorCnt,
				kw[keywcnt].keyword);
#endif
				keywcnt++;
				if (keywcnt+3>=AllocN) {
					AllocN+=32;
					if ((kw=mrealloc(kw,AllocN*sizeof(KW),"LoadTedrc 3"))==NULL) exit(1);
				}
				ww=vv;
				if (ww==xx) break;
			}
			ColorCnt++;
		}
		qsort(kw+3,keywcnt-3,sizeof(KW), kwcmp);
		LangKw[LangN]=kw;
		LangMaxKeywordLen[LangN]=MaxKeywordLen;
		LangKwN[LangN++]=keywcnt;
		if (LangN>=MaxLang) {
			error("Too many languages defined");
			error("Remove unused language in tedrc or modify ted.h and recompile");
			exit(1);
		}
	} else
	if (!strcmp(tt,"include")) {
		if (depth > 10) {
			error("include too many level ??");
			continue;
		}
		ww=to_spc(vv);
		xx=skip_spc(ww);
		if (!(*vv)) lper(tedname,lno,"argument expected");
		*ww=0;
		LoadTedrc(vv,depth+1,kdefmapno);
		tedrcfname=tedname;
	} else
	if (!strcmp(tt,"filebrowser")) {
		ww=to_spc(vv);
		xx=skip_spc(ww);
		if (!(*vv)) lper(tedname,lno,"argument expected");
		init_reg_exp(vv);
	} else
	if (!strcmp(tt,"fontlist")) {
		ww=to_spc(vv);
		xx=skip_spc(ww);
		if (!(*vv)) lper(tedname,lno,"argument expected");
		setFontList(vv);
	} else
	if (!strcmp(tt,"LoadKeyDefInto")) {
		int no;
		char mstr[16], *p;
		tt=parse_Cstr(tedname, lno,vv,&uu);
		if (!(*tt)) lper(tedname,lno,"file name expected");
		ww=skip_spc(uu);
		if (!*ww) lper(tedname,lno,"Bind number expected");
		no=*ww-'0';
		if (no<=0 || no>=4)
			lper(tedname,lno,"Bind number must be less than 4 and greater than 0");
		if ((p=strchr(tt,'.'))==NULL) {
			mode_str_m[no]="      ";
		} else {
			p++;
			if (! *p)
				mode_str_m[no]=strdup("      ");
			else
				mode_str_m[no]=strdup(p);
		}
		LoadTedrc(tt,0,no);
	}
}
fclose(fp);

if (!depth) {
kdefno=kdefno_m[kdefmapno];
qsort(kdef,kdefno,sizeof(kdef[0]),qcmp);
bzero(hdef,sizeof(hdef));
bzero(kdefhash[kdefmapno],128*sizeof(int));
for(i=0;i<kdefno;i++) {
	j=KEYHASH(kdef[i].kcode[0]);
	if (hdef[j]) continue;
	hdef[j]=1;
	kdefhash[kdefmapno][j]=i;
}
kdefhash[kdefmapno][127]=kdefno;
for(i=126;i>=0;i--)
if (!hdef[i]) kdefhash[kdefmapno][i]=kdefhash[kdefmapno][i+1];
ReLoadTed=1;
if (X_inited) alloc_pc_colors();
}
return 0;
}

void set_kbd_mode(int no)
{
if (no>=4 || !kdefmap[no]) return;
if (ckdef==kdefmap[no]) return;
ckdef=kdefmap[no];
ckdefno=kdefno_m[no];
ckdefhash=kdefhash[no];
mode_str=mode_str_m[no];
cmapno=no;
}

void set_kdef();

K_LoadTedrc(char *ftedrc)
{
LoadTedrc(ftedrc,0,0);
set_kbd_mode(0);
set_lang_type(fname);
set_kdef();
}

K_MergeTedrc(char *ftedrc)
{
LoadTedrc(ftedrc,1,0);
set_kbd_mode(0);
set_lang_type(fname);
set_kdef();
}

void disp_kb_mode()
{
if (batch) return;
#if	0
xputs(edwin[active_win].MCOL-20,
edwin[active_win].MROW-2+edwin[active_win].y,mode_str,2);
#else
xputs(XW_COL-20,
	edwin[0].MROW-2+edwin[0].y,mode_str,2);
#endif
}

K_SwitchKeyDef(int no)
{
set_kbd_mode(no);
disp_kb_mode();
}

static int kst;
char shift_chr[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZ~ÄÖÜ!@#$%^&*()_+|{\"}:?><";

u_short map_xkey(int skey,int state,int chr)
{
int km,top,bottom;

if (skey>0x7f&&skey<=0xff) skey=chr;
if (kst || (state&(ControlMask|Mod1Mask)))
	skey=mtolower(skey);
top=0; bottom=kname_no-1;
while (top<=bottom) {
	km=(top+bottom)>>1;
	if (skey > kmap[km].xkey) top=km+1;
	else
	if (skey < kmap[km].xkey) bottom=km-1;
	else break;
}
if (top>bottom || kmap[km].xkey!=skey) return 0xffff;
if (state&(Mod1Mask|Mod5Mask)) km|=ALT;
if (state&ControlMask) km|=CTRL;
if (state&ShiftMask && (skey>255 || (strchr(shift_chr,skey) &&
km&(ALT|CTRL)))  ) km|=SHFT;
return km;
}
#define MAX_MACRO_BUF (1024)
static char *mac_buf, *mac_ptr;
static int mac_cnt, rec_mac;

K_RecMacro()
{
int i;
if (rec_mac) {
	message(4,"Macro recording stopped..");
	rec_mac=0;
	return 1;
}
message(4,"Macro recording started..");
if (mac_buf) free(mac_buf);
if (!(mac_buf=mmalloc(MAX_MACRO_BUF+16,"K_RecMacro 2"))) {
	return 0;
}
mac_cnt=0;
mac_ptr=mac_buf;
rec_mac=1;
}

K_PlayMacro()
{
  if (rec_mac) {
    message(4,"You are still recording marcro");
    return 0;
  }
  if (mac_buf && mac_cnt)
    ExecCommand(mac_buf,mac_cnt);
}

K_ExportMacro(char *fname)
{
int i,count;
char *cmd,*ttt,*uuu;
FILE *fw;

if ((fw=fopen(fname,"a"))==NULL) {
	message(4,"Cannot append %s",fname);
	return;
}

if (!mac_buf) return;
if (rec_mac) {
	message(4,"You are still recording macro");
	return;
}
cmd=mac_buf;
for(i=0;i<mac_cnt;i++) {
	u_char funo=*(cmd++);
	switch (func[funo].argtype) {
	case 0: /* void */
		fprintf(fw,"%s();",func[funo].funname);
		break;
	case 1: /* int */
		fprintf(fw,"%s(%d);",func[funo].funname,*(cmd++));
		break;
	case 2: /* string */
		fprintf(fw,"%s(\"%s\");",func[funo].funname,cmd);
		cmd+=strlen(cmd)+1;
		break;
	case 3: /* string,string */
		ttt=cmd+strlen(cmd)+1;
		fprintf(fw,"%s(\"%s\",\"%s\");",func[funo].funname,cmd,ttt);
		cmd+=strlen(cmd)+1+strlen(ttt)+1;
		break;
	case 7: /* string,string,string */
		ttt=cmd+strlen(cmd)+1;
		uuu=ttt+strlen(ttt)+1;
		fprintf(fw,"%s(\"%s\",\"%s\",\"%s\");",func[funo].funname,cmd,ttt,uuu);
		cmd+=strlen(cmd)+1+strlen(ttt)+1+strlen(uuu)+1;
		break;
	}
}
fprintf(fw,"\n");
fclose(fw);
}

void ck_mac_size()
{
  if (mac_ptr>=mac_buf+MAX_MACRO_BUF) {
    message(4,"You forgot to stop macro recording");
    rec_mac=0;
  }
}

cmdlen(char *cmd, int count)
{
int i;
char *ttt,*uuu;
char *qqq=cmd;

for(i=0;i<count;i++) {
	 u_char funo=*(cmd++);
	 switch (func[funo].argtype) {
	 case 0:
		 break;
	 case 1:
		 cmd++;
		 break;
	 case 2:
		 cmd+=strlen(cmd)+1;
		 break;
	 case 3:
		 ttt=cmd+strlen(cmd)+1;
		 cmd+=strlen(cmd)+1+strlen(ttt)+1;
		 break;
	 case 7:
		 ttt=cmd+strlen(cmd)+1;
		 uuu=ttt+strlen(ttt)+1;
		 cmd+=strlen(cmd)+1+strlen(ttt)+1+strlen(uuu)+1;
	case 8:
	   cmd+=sizeof(int)*2;
		 break;
	}
}

return cmd-qqq;
}

int saveBufType;

K_PutStrSave()
{
  puts("should not enter here");
}


int readonly=0;

K_TogReadOnly()
{
  readonly^=1;
  if (readonly)
    message(4,"Read-only mode");
  else
    message(4,"Read-Write mode");
}

static char *last_str;

void X_loop()
{
while (input_handler) {
	X_event();
}
}

ExecCommand(char *cmd, int count)
{
	int i,usta,j;
	char *ttt,*uuu;
	char *pcmd;

	pcmd=cmd;
	last_str=0;
	for(i=0;i<count;i++) {
		u_char funo=*(cmd++);
		if (func[funo].modfunc && readonly) {
			message(4,"Read only mode");
			break;
		}
		switch (func[funo].argtype) {
		case 0:
			if (rec_mac && funo!=1) {
				*(mac_ptr++)=funo;
				mac_cnt++;
				ck_mac_size();
			}
			usta=func[funo].action();
			X_loop();
			break;
		case 1:
			if (rec_mac) {
				*(mac_ptr++)=funo;
				*(mac_ptr++)=*cmd;
				mac_cnt++;
				ck_mac_size();
			}
			usta=func[funo].action(*(cmd++));
			break;
		case 2:
			if (rec_mac) {
				*(mac_ptr++)=funo;
				strcpy(mac_ptr, cmd);
				mac_ptr+=strlen(cmd)+1;
				ck_mac_size();
			}
			usta=func[funo].action(cmd);
			if (ReLoadTed) {
				ReLoadTed=0;
				return 0;
			}
			cmd+=strlen(cmd)+1;
			break;
		case 3:
			usta=func[funo].action(cmd,ttt=cmd+strlen(cmd)+1);
			if (rec_mac) {
				*(mac_ptr++)=funo;
				strcpy(mac_ptr, cmd);
				mac_ptr+=strlen(cmd)+1;
				strcpy(mac_ptr,ttt);
				mac_ptr+=strlen(ttt)+1;
				ck_mac_size();
			}
			cmd+=strlen(cmd)+1+strlen(ttt)+1;
			break;
		case 5:
		  /* for binding that needs vi keystrokes saving (dd, dw ...) */
			/* look for PlaySaveFunc */
			for (j=0;j<ckdefno;j++) {
				char *p=ckdef[j].cmd;
				if (*p==4) {
					static char *qq;
					int cmlen=cmdlen(pcmd,count);

					if ((qq=mrealloc(qq,cmlen,"feek_key"))==NULL)
						return 0;
					memcpy(qq,pcmd,cmlen);
					ckdef[j].type=count;
					*qq=4;
					ckdef[j].cmd=qq;
					saveBufType=1;
					break;
				}
			}
			usta=1;
			break;
		case 6:
			if (saveBufType) usta=1;
			else {
				putStrSave();
				return 0;
			}
			break;
		case 7:
			ttt=cmd+strlen(cmd)+1;
			uuu=ttt+strlen(ttt)+1;
      usta=func[funo].action(cmd,ttt,uuu);
			if (rec_mac) {
				*(mac_ptr++)=funo;
				strcpy(mac_ptr, cmd);
				mac_ptr+=strlen(cmd)+1;
				strcpy(mac_ptr,ttt);
				mac_ptr+=strlen(ttt)+1;
				strcpy(mac_ptr,uuu);
				mac_ptr+=strlen(uuu)+1;
				ck_mac_size();
			}
			cmd+=strlen(cmd)+1+strlen(ttt)+1+strlen(uuu)+1;
			break;
		case 8:
		{
		  int aa,bb;
		  memcpy(&aa,cmd,sizeof(int));
		  memcpy(&bb,cmd+4,sizeof(int));
      usta=func[funo].action(aa,bb);
      cmd+=sizeof(int)*2;
    }
		  break;
		}
    if (xgets_enter==-1) {
		  xgets_enter=0;
		  break;
		}
		if (usta) continue;
		disp_fname();
		disp_cur_pos();
		disp_ins_mod();
		show_cursor_w();
	}
return 1;
}


void feed_key(int skey,int state,int chr)
{
int kk;
u_short km=map_xkey(skey,state,chr);
static u_short lastofs, endofs;
u_short hh=KEYHASH(km);

if (km==0xffff) return;
if (!kst) {
state0:
	kst=0;
	lastofs=ckdefhash[hh];
	endofs=ckdefhash[hh+1];
	while (lastofs<endofs && ckdef[lastofs].kcode[0]!=km) lastofs++;
	if (ckdef[lastofs].kcode[0]!=km) return;
	else  kst=1;
	if (ckdef[lastofs].klen==1) goto match;
	return;
} else {
	while (lastofs<endofs &&
	(ckdef[lastofs].kcode[kst]!=km)||ckdef[lastofs].klen<kst) lastofs++;
	if (lastofs==endofs)
		goto state0;
	kst++;
	if (kst==ckdef[lastofs].klen)
		goto match;
	else return;
}
match:
kst=0;
kk=lastofs;

if (!ckdef[kk].type) {
	if (readonly) {
		message(4,"Readonly mode");
		return;
	}
	if (rec_mac) {
		if (last_str) { /* append it */
			strcat(last_str, ckdef[kk].cmd);
			mac_ptr+=strlen(ckdef[kk].cmd);
		} else {
			*(mac_ptr++)=0;
			strcpy(last_str=mac_ptr, ckdef[kk].cmd);
			mac_ptr+=strlen(ckdef[kk].cmd)+1;
			mac_cnt++;
		}
		ck_mac_size();
	}
	K_PutStr(ckdef[kk].cmd);
	disp_fname();
	disp_cur_pos();
	show_cursor_w();
	return;
} else {
	ExecCommand(ckdef[kk].cmd,ckdef[kk].type);
}
}

void set_kdef()
{
ckdef=kdefmap[0];
ckdefno=kdefno_m[0];
mode_str=mode_str_m[0]="      ";
ckdefhash=kdefhash[0];
}
