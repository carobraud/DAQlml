PROGRAMS = parameters.nc DAQlml
DOCUMENTATIONS = doc

#OPT = -DLOOP_USLEEP_TIME=500 -Wall -Wextra -ansi -pedantic -O0 -g -fno-tree-pre -Dcimg_use_vt100 -DDEMIPERIOD=10000000
OPT = -DLOOP_USLEEP_TIME=500 -Wall -Wextra -ansi -pedantic -O0 -g -fno-tree-pre -Dcimg_use_vt100
LibRT = -DREAL_TIME -lrt
LIBcomedi = -lcomedi -lm
LIBCImg = -I/usr/X11R6/include -Dcimg_use_xshm -Dcimg_use_xrandr -L/usr/X11R6/lib -lpthread -lX11 -lXext -lXrandr
LIBNetCDF = -I../../NetCDF/include -lnetcdf_c++ -lnetcdf -L../../NetCDF/lib/
CC = gcc
CPP = g++

all: $(PROGRAMS) $(DOCUMENTATIONS)

parameters.nc: parameters.cdl
	ncgen -b parameters.cdl -o parameters.nc
	ncgen -b parameters.test.cdl -o parameters.test.nc
DAQlml: main.cpp acquisition.h control.h DAQloop.h DAQdata.h DAQcomedi.h DAQtest.h ../RealTime/RT_PREEMPT.h
	cd ../CImg.Tool; rm CImg_NetCDF.h;  ln -s CImg_NetCDF.v0.4.DAQlml.h CImg_NetCDF.h; cd ../DAQlml
##RT or standard
	$(CPP) $(OPT) main.cpp $(LibRT) $(LIBcomedi) $(LIBCImg) $(LIBNetCDF) -o $@
#	$(CPP) $(OPT) main.cpp          $(LIBcomedi) $(LIBCImg) $(LIBNetCDF) -o $@
doc: DAQlml.Doxygen main.cpp acquisition.h control.h DAQloop.h DAQdata.h DAQcomedi.h DAQtest.h
	./doxIt.sh

clean:
	rm -rf $(DOCUMENTATIONS)
	rm -f *.o
	rm -rf .libs
	@list='$(PROGRAMS)'; for p in $$list; do \
	  rm -f $$p ; \
	done


