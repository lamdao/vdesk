VERSION  = 0.0.92
CC       = g++
#USE_MB_TEXT = -DUSE_MB_TEXT
FLAGS    = $(CXXFLAGS) $(USE_MB_TEXT) -DVERSION=\"$(VERSION)\" -I/usr/X11R6/include -I/usr/include/freetype2
LIBS	 = `imlib-config --libs` -lXft
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
