CHINESE=0
TEDDIR=/usr/local/lib/Ted
########## GCC/Linux ......  ############
CC=gcc
XINC=-I/usr/X11R6/include
LDFLAGS=-L/usr/X11R6/lib
###########################
###### HP-UX's ansi cc ####
#CC=cc -Ae
#LDFLAGS=-L/usr/lib/X11R5
#LDFLAGS=-L/usr/lib/X11R6
#XINC=
###########################
########## IRIX ############
#CC=cc -cckr
#LDFLAGS=
#XINC=-I/usr/include/X11
#LDFLAGS= -L/usr/lib/X11
###########################
########## CC/DEC Alpha /OSF ......  ############
#CC=cc
#XINC=-I/usr/include/X11
#LDFLAGS=-L/usr/lib/X11
###########################
########## CC/IBM AIX......  ############
#CC=cc -DAIX
#XINC=
#LDFLAGS=
###########################
########## Solaris and SunOS 4.x ############
CC=cc
#XINC=-I/usr/openwin/include
#LDFLAGS= -L/usr/openwin/lib -lsocket
#CC=cc
#XINC=-I/usr/X11R6.3/include
#LDFLAGS= -L/usr/X11R6.3/lib -lsocket
#SUNKBD=-DSUNKBD
###########################

CFLAGS = -O -DCHINESE=$(CHINESE) $(XINC) $(SUNKBD) -DTEDDIR=\"$(TEDDIR)\"
#CFLAGS = -g -DCHINESE=$(CHINESE) $(XINC) -DDEBUG $(SUNKBD) -DTEDDIR=\"$(TEDDIR)\"
#CFLAGS = -g -DCHINESE=$(CHINESE) $(XINC) $(SUNKBD) -DTEDDIR=\"$(TEDDIR)\"

OBJS =	e.o x.o m.o f.o b.o edln.o xcp.o s.o u.o misc.o twin.o dir.o xkey.o \
	cwin.o filebrowse.o kwcomp.o

all:	ted ref ctags tedrc
ted:	$(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) -lX11
pur:	$(OBJS)
	purify $(CC) $(OBJS) -o $@ $(LDFLAGS) -lXmu -lX11
#strip $@
	echo '*** Finish ***'

# tedrc.tmpl is no longer needed.
# tedrc:	 tedrc.tmpl
# cat tedrc.tmpl | sed "s~TEDDIR~$(TEDDIR)~g" > tedrc

ref:
	$(CC) $(CFLAGS) ref.c -o $@
	strip $@
ctags:
	$(CC) $(CFLAGS) ctags.c -o $@
	strip $@
install:
	cp ted ref ctags /usr/local/bin
	if [ ! -d $(TEDDIR) ]; then mkdir $(TEDDIR); fi
	cp tedrc tedrc.vi tedrc.grep ted.help tedrc.filevisit $(TEDDIR)
cli_xcin.o:
	ln -s ../big5-pack/xcin/cli_xcin.o .
clean:
	rm -f $(OBJS) ted core *~ *_
clb5:
	rm -f x.o e.o m.o edln.o

#include .depend
