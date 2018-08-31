CC = g++
CFLAGS = -c -std=c++11
LDFLAGS = -lSDL2 -lglut -lGL -lGLU -lglfw -lGLEW -lavformat -lavcodec -lavutil -lswscale -lSDL2_image
SOURCES = glmodelwin.cpp vertex.cpp vec3.cpp triangle.cpp pmesh.cpp mesh.cpp conf.cpp
OBJECTS1 = server.o $(SOURCES:.cpp=.o)
OBJECTS2 = client.o $(SOURCES:.cpp=.o)

EXECUTABLE1 = server
EXECUTABLE2 = client

all: $(SOURCES) $(EXECUTABLE1) $(EXECUTABLE2)
	
$(EXECUTABLE1): $(OBJECTS1)
	$(CC) $(OBJECTS1) -o $@ $(LDFLAGS)

$(EXECUTABLE2): $(OBJECTS2)
	$(CC) $(OBJECTS2) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o $(EXECUTABLE)
