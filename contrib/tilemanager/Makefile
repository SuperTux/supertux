GOAL=tilemanager.exe
SOURCES=$(wildcard *.cs)
CSFLAGS=-debug -pkg:gtk-sharp -pkg:glade-sharp -pkg:gnome-sharp -r:System.Drawing

$(GOAL): $(SOURCES)
	mcs $(CSFLAGS) $(SOURCES) -out:$(GOAL)

clean:
	rm -f $(GOAL)

.PHONY: clean
