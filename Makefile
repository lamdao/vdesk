VERSION  = 0.0.92
CC       = g++
#USE_MB_TEXT = -DUSE_MB_TEXT
XFT_INC  = $(shell pkg-config --cflags xft)
XFT_LIB  = $(shell pkg-config --libs xft)
FLAGS    = $(CXXFLAGS) $(USE_MB_TEXT) -DVERSION=\"$(VERSION)\" $(XFT_INC) 
LIBS	 = `imlib-config --libs` $(XFT_LIB)
VDESK    = vdesk

OBJS     = Color.o BusyCursor.o Database.o Desktop.o Image.o Label.o Link.o Main.o \
			Preferences.o Resource.o Window.o Canvas.o Text.o Device.o Event.o \
			Menu.o Timer.o Trash.o Button.o Dialog.o Editor.o LinkEditor.o Background.o

all: $(VDESK)

$(VDESK): $(OBJS)
	$(CC) $(OBJS) -o $(VDESK) $(LIBS)

%.o: %.cc
	$(CC) $(FLAGS) -c $< -o $@

install: $(VDESK)
	install $(VDESK) /usr/local/bin

clean:
	rm -f $(OBJS) $(VDESK) *~ core

distclean: clean
	rm -f .*.sw*

include Makefile.dep
