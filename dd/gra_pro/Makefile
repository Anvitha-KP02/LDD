# OpenGL 3.3 Core + GLFW (Mesa + libglfw on Ubuntu/Debian)
CFLAGS = -Wall -Wextra -O2
LIBS = -lglfw -lGL -lm

gra_demo: main.c
	$(CC) $(CFLAGS) -o gra_demo main.c $(LIBS)

clean:
	rm -f gra_demo

.PHONY: clean
