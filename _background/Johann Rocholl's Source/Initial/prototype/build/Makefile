# PKG_CONFIG_PATH=/opt/local/lib/pkgconfig:${PKG_CONFIG_PATH}

OBJECTS=minmax.o points.o dir8.o dir16.o blur.o gradients.o roi.o \
scanlines.o checksums.o sections.o changes.o illumination.o \
perspective.o encoder.o simulator.o gnuplot.o threshold.o peaks.o \
trace.o quietzones.o corners.o thirty.o decoder.o
CCFLAGS=`pkg-config --cflags opencv` -I../decoder -std=c99
LDFLAGS=`pkg-config --libs opencv`

all : picture camera

picture : picture.o $(OBJECTS)
	gcc $(LDFLAGS) -o $@ $^

camera : camera.o $(OBJECTS)
	gcc $(LDFLAGS) -o $@ $^

%.o : ../decoder/%.c ../decoder/*.h
	gcc $(CCFLAGS) -c $< -o $@

%.o : ../%.c
	gcc $(CCFLAGS) -c $< -o $@
