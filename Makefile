FLAGS = -Wall -g
.PHONY: clean all

all: closest generate_points

closest: closest.o parallel_closest.o serial_closest.o utilities_closest.o
	gcc ${FLAGS} -o $@ $^ -lm

generate_points: generate_points.o
	gcc ${FLAGS} -o $@ $^

closest.o: closest.c point.h serial_closest.h parallel_closest.h utilities_closest.h
	gcc ${FLAGS} -c $<

generate_points.o: generate_points.c point.h
	gcc ${FLAGS} -c $<

parallel_closest.o: parallel_closest.c point.h serial_closest.h parallel_closest.h utilities_closest.h
	gcc ${FLAGS} -c $<

serial_closest.o: serial_closest.c point.h utilities_closest.h
	gcc ${FLAGS} -c $<

utilities_closest.o: utilities_closest.c point.h utilities_closest.h
	gcc ${FLAGS} -c $<

clean:
	rm closest generate_points *.o