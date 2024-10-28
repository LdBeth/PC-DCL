RC            =	rc
CC            =	cl
LD            =	link
INSTALL       =	copy
RM            = - erase
COPTIMISE     =	-nologo -Gy -W3 -O2
CDEFS         = -D_WIN32 -D_CRT_SECURE_NO_WARNINGS -I.
LDDEBUG       =	/DEBUG
LDOPTIMISE    =	
LDFLAGS       = /NOLOGO /INCREMENTAL:no /PDB:NONE /LIBPATH:lib
LIBS = shell32.lib user32.lib advapi32.lib pcre.lib

.SUFFIXES: .c .o .rc .res

.c.o:
	$(CC) $(COPTIMISE) $(CDEFS) $(MAKECDEFS) -c $< -Fo$@

.rc.res:	
	$(RC) -v -i . -fo $@ $*.rc 

SRC		= allocate.c cmd.c dclfindfile.c delete.c expr.c goto.c keyboard.c open.c rename.c setfile.c symass.c while.c \
append.c copy.c dclini.c dir.c filecopy.c help.c lexical.c platform.c repeat.c show.c symbol.c termio_win32.c write.c \
assign.c create.c dcltime.c else.c findfile.c hlpvms.c logical.c print.c return.c splitpath.c then.c xcvfs.c \
call.c dbglog.c dealloca.c endif.c flexlist.c if.c md5.c read.c run.c step.c termio.c type.c \
close.c dcl.c define.c endsubro.c gosub.c inquire.c on.c recall.c set.c subrouti.c wait.c

HEADERS = dbglog.h dcl.h dclfindfile.h dcltime.h error.h filecopy.h findfile.h flexlist.h md5.h \
pcre.h platform.h psapi.h splitpath.h step.h termio.h termio_win32.h

OBJ_C    = $(SRC:.c=.o)
PLTMRC = dcl.rc
PLTMRES  = $(PLTMRC:.rc=.res)

all: dcl.exe

clean:
    $(RM) *.exe
    $(RM) *.o

install: dcl.exe
    $(INSTALL) dcl.exe ..\dcl2.exe

dcl.exe: $(OBJ_C) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) /SUBSYSTEM:console /out:$@ $(OBJ_C) $(PLTMRES) $(CONSOLE_LIBS) $(LIBS)

$(OBJ_C): $(HEADERS)
