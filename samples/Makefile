CPP=g++
CPPFLAGS=-std=c++0x -I../src/
EXT=.out

ifdef windir
  EXT=.exe
endif

.PHONY: clean

all: $(addsuffix $(EXT),$(basename $(shell ls *.cpp)))

%$(EXT): %.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

clean:
	$(RM) *$(EXT)
