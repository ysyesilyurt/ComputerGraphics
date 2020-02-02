CC = g++
src = *.cpp

all: clean hw3_flat hw3_sphere

hw3_flat: clean1 part1

hw3_sphere: clean2 part2

part1:
	$(CC) flat/$(src) -std=c++11 -O3 -o hw3_flat -lXi -lGLEW -lGLU -lGL -lm -lpthread -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lglfw3 -lrt -ldl -lXrandr -lXinerama -lXxf86vm -lXext -lXcursor -lXrender -lXfixes -lX11 -lpthread -lxcb -lXau -lXdmcp -ljpeg

part2:
	$(CC) sphere/$(src) -std=c++11 -O3 -o hw3_sphere -lXi -lGLEW -lGLU -lGL -lm -lpthread -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lglfw3 -lrt -ldl -lXrandr -lXinerama -lXxf86vm -lXext -lXcursor -lXrender -lXfixes -lX11 -lpthread -lxcb -lXau -lXdmcp -ljpeg

clean:
	rm -f hw3_flat hw3_sphere

clean1:
		rm -f hw3_flat

clean2:
		rm -f hw3_sphere

