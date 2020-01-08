CC = g++
src = *.cpp

all: clean hw3_flat hw3_sphere

hw3_flat:
	$(CC) hw3_flat.cpp helper.h helper.cpp jconfig.h -std=c++11 -O3 -o hw3_flat -lXi -lGLEW -lGLU -lGL -lm -lpthread -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lglfw3 -lrt -ldl -lXrandr -lXinerama -lXxf86vm -lXext -lXcursor -lXrender -lXfixes -lX11 -lpthread -lxcb -lXau -lXdmcp -ljpeg

hw3_sphere:
	$(CC) hw3_sphere.cpp helper.h helper.cpp jconfig.h -std=c++11 -O3 -o hw3_flat -lXi -lGLEW -lGLU -lGL -lm -lpthread -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lglfw3 -lrt -ldl -lXrandr -lXinerama -lXxf86vm -lXext -lXcursor -lXrender -lXfixes -lX11 -lpthread -lxcb -lXau -lXdmcp -ljpeg

debug:
	$(CC) $(src) -std=c++11 -O3 -o test -lXi -lGLEW -lGLU -lGL -lm -lpthread -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lglfw3 -lrt -ldl -lXrandr -lXinerama -lXxf86vm -lXext -lXcursor -lXrender -lXfixes -lX11 -lpthread -lxcb -lXau -lXdmcp -ljpeg -g -Wall -Wextra

clean:
		rm -f hw3_flat