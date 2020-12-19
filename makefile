PHG_HOME  = /home/tong/Documents/Apps/phg-0.8.5
PHG_LIB   =${PHG_HOME}/src/libphg

FILES = Makefile functions.h poisson.c simplest.c .cvsignore *-plot.sh jobs.sh \
	maxwell*.c heat.c non-smooth.c eigen.c elastic.c navier-stokes.[ch] \
	navier-stokes-unsteady.c

#default: test-grid
default: ice-flow
#default: test-solverT

all: lib poisson simplest maxwell maxwell-complex heat non-smooth eigen \
	elastic maxwell-eigen navier-stokes

examples.zip: ${FILES}
	@zip -9 -u -y $@ $^

clean:
	-/bin/rm -f *.o

distclean: clean
	-/bin/rm -f *.o ice-flow

matclean:
	-/bin/rm -f *.m *.m.dat

lib:
	@(cd ../src; $(MAKE))


include ${PHG_HOME}/Makefile.inc
VPATH=./src
vpath %.c ./src

#################
#   test grid   #
#################

test-grid.o: test-grid.c
test-grid: ${PHG_LIB}${LIB_SUFFIX} test-grid.o ice-grid.o
	@echo "Linking test-grid"
	${LINKER} ${LDFLAGS} -o $@ $^ ${LIBS}    

test-gridv.o: test-gridv.c
test-gridv: ${PHG_LIB}${LIB_SUFFIX} test-gridv.o ice-grid.o slip-bdry.o
	@echo "Linking test-gridv"
	${LINKER} ${LDFLAGS} -o $@ $^ ${LIBS}    

test-solver.o: test-solver.c
test-solver: ${PHG_LIB}${LIB_SUFFIX} test-solver.o
	@echo "Linking test-grid"
	@${LINKER} ${LDFLAGS} -o $@ $^ ${LIBS}    

###########
#   INS   #
###########
ins-main.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ins-main.c
contact.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  contact.c
ins-solver.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ins-solver.c
ins-system.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ins-system.c
ins-utils.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ins-utils.c
ins-bc.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ins-testcase.c ins-bc.c TEST.c
temp-solver.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  temp-solver.c TEST.c
ins-pcd.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ins-pcd.c
save-load.o: ${PHG_LIB}${LIB_SUFFIX} save-load.c
slip-bdry.o: ${PHG_LIB}${LIB_SUFFIX} slip-bdry.c
ice-grid.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ice-grid.c
read_txt_data.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  read_txt_data.c
netcdf-utils.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  netcdf-utils.c netcdf-utils.h
#greenland.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  greenland.c
#ins-mg.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h ins-mg.c
#multi-grid.o: ${PHG_LIB}${LIB_SUFFIX} multi-grid.h multi-grid.c
#mg-line-smoother.o: ${PHG_LIB}${LIB_SUFFIX} multi-grid.h mg-line-smoother.c
#multi-grid-smoother.o: ${PHG_LIB}${LIB_SUFFIX} multi-grid.h multi-grid-smoother.c
moving-mesh.o: ${PHG_LIB}${LIB_SUFFIX} ins.h layers.h moving-mesh.c
struct-mesh.o: ${PHG_LIB}${LIB_SUFFIX} ins.h layers.h struct-mesh.c
layers.o: ${PHG_LIB}${LIB_SUFFIX} ins.h layers.h layers.c
#line-smoother.o: ${PHG_LIB}${LIB_SUFFIX} ins.h layers.h line-smoother.c
ice-flow: ${PHG_LIB}${LIB_SUFFIX} ins-main.o contact.o ins-solver.o ins-bc.o ins-utils.o save-load.o ins-system.o temp-solver.o ice-grid.o slip-bdry.o ins-pcd.o read_txt_data.o moving-mesh.o struct-mesh.o layers.o fv_vert.o netcdf-utils.o -lnetcdf
	@echo "Linking ice-flow"
	@${LINKER} ${LDFLAGS} -g -o $@ $^ ${LIBS}
#ins-mg.o multi-grid.o multi-grid-smoother.o mg-line-smoother.o



#################
# Test solver T #
#################
# test-solverT.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ins-main.c
# temp-solver.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  temp-solver.c 
# ins-utils.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ins-utils.c
# ins-bc.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ins-testcase.c ins-bc.c ISMIP-HOM.c ESIMINT_TEST.c HEINO_TEST.c GREENLAND.c test-solverT-bc.c
# ice-grid.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  ice-grid.c
# #greenland.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h  greenland.c
# #ins-mg.o: ${PHG_LIB}${LIB_SUFFIX} ins.h parameters.h ins-mg.c
# #multi-grid.o: ${PHG_LIB}${LIB_SUFFIX} multi-grid.h multi-grid.c
# #mg-line-smoother.o: ${PHG_LIB}${LIB_SUFFIX} multi-grid.h mg-line-smoother.c
# #multi-grid-smoother.o: ${PHG_LIB}${LIB_SUFFIX} multi-grid.h multi-grid-smoother.c
# moving-mesh.o: ${PHG_LIB}${LIB_SUFFIX} ins.h layers.h moving-mesh.c
# layers.o: ${PHG_LIB}${LIB_SUFFIX} ins.h layers.h layers.c
# line-smoother.o: ${PHG_LIB}${LIB_SUFFIX} ins.h layers.h line-smoother.c
# test-solverT: ${PHG_LIB}${LIB_SUFFIX} test-solverT.o ins-solver.o ins-bc.o ins-utils.o temp-solver.o ice-grid.o layers.o moving-mesh.o line-smoother.o
# 	@echo "Linking test-solverT"
# 	@${LINKER} ${LDFLAGS} -o $@ $^ ${LIBS}
# #ins-mg.o multi-grid.o multi-grid-smoother.o mg-line-smoother.o



fv_vert.o: fv_vert.c
	@echo "compiling fv_vert.o ..."
	${CC} ${CFLAGS} ${CPPFLAGS} -O0 -c src/fv_vert.c 2>&1 | sed -e '/unused/d; /defined but not used/d; '
