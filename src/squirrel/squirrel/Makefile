SQUIRREL= ..


OUT= $(SQUIRREL)/lib/libsquirrel.a
INCZ= -I$(SQUIRREL)/include -I. -Iinclude
DEFS= 
LIB=	stdc++

OBJS= \
	sqapi.o \
	sqbaselib.o \
	sqcompiler.o \
	sqdebug.o \
	sqlexer.o \
	sqobject.o \
	sqparser.o \
	sqstate.o \
	sqtable.o \
	sqvm.o \
	sqmem.o \
	sqclass.o
	
SRCS= \
	sqapi.cpp \
	sqbaselib.cpp \
	sqfuncstate.cpp \
	sqdebug.cpp \
	sqlexer.cpp \
	sqobject.cpp \
	sqcompiler.cpp \
	sqstate.cpp \
	sqtable.cpp \
	sqmem.cpp \
	sqvm.cpp \
	sqclass.cpp

	
	
squirrellib:
	gcc -O3 -Os -c $(SRCS) $(INCZ) $(DEFS)
	ar rc $(OUT) *.o
	rm *.o