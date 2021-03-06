#Standard variables
CC=gcc $(CFLAGS)
CXX=g++ $(CFLAGS)
CFLAGS=-Wall
CMATH=-lm

#My variables
ARCH=-march=native
OPT=_opt
M=_arch
SIN=size_test_input
F="%es\n"

#Goals
all: clean size_test fib fmath unroll tests

size_test: size_test.cpp
	$(CXX) $< -o $@.out
	$(CXX) $< -o $@$M.out $(ARCH)
	$(CXX) $< -o $@$(OPT).out -Os
	$(CXX) $< -o $@$(OPT)$M.out -Os $(ARCH)

fib: fib.c
	$(CC) $< -o $@.out
	$(CC) $< -o $@$M.out $(ARCH)
	$(CC) $< -o $@$(OPT).out -O3
	$(CC) $< -o $@$(OPT)$M.out -O3 $(ARCH)

fmath: fmath.c
	$(CC) $(CMATH) $< -o $@.out -O1
	$(CC) $(CMATH) $< -o $@$M.out -O1 $(ARCH)
	$(CC) $(CMATH) $< -o $@$(OPT).out -O1 -ffast-math
	$(CC) $(CMATH) $< -o $@$(OPT)$M.out -O1 -ffast-math $(ARCH)

unroll: unroll.c
	$(CC) $< -o $@.out -O1
	$(CC) $< -o $@$M.out -O1 $(ARCH)
	$(CC) $< -o $@$(OPT).out -O1 -funroll-loops
	$(CC) $< -o $@$(OPT)$M.out -O1 -funroll-loops $(ARCH)

tests:
	@echo "Test flagi -Os: size_test.cpp"
	@echo "***************************************"
	@echo "Bez opcji:"
	time ./size_test.out < $(SIN)
	wc -c ./size_test.out
	@echo "Bez opcji, pod architekturę:"
	time ./size_test$M.out < $(SIN)
	wc -c ./size_test$M.out
	@echo "Z opcją -Os:"
	time ./size_test$(OPT).out < $(SIN)
	wc -c ./size_test$(OPT).out
	@echo "Z opcją -Os, pod architekturę:"
	time ./size_test$(OPT)$M.out < $(SIN)
	wc -c ./size_test$(OPT)$M.out
	@echo "***************************************"
	@echo "Test flagi -03: fib.c"
	@echo "***************************************"
	@echo "Bez opcji:"
	time -f $F ./fib.out 45
	@echo "Bez opcji, pod architekturę:"
	time -f $F ./fib$M.out 45
	@echo "Z opcją -O3:"
	time -f $F ./fib$(OPT).out 45
	@echo "Z opcją -O3, pod architekturę:"
	time -f $F ./fib$(OPT)$M.out 45
	@echo "***************************************"
	@echo "Test flagi -ffast-math: fmath.c"
	@echo "***************************************"
	@echo "Bez opcji:"
	time -f $F ./fmath.out
	@echo "Bez opcji, pod architekturę:"
	time -f $F ./fmath$M.out
	@echo "Z opcją -ffast-math:"
	time -f $F ./fmath$(OPT).out
	@echo "Z opcją -ffast-math, pod architekturę:"
	time -f $F ./fmath$(OPT)$M.out
	@echo "***************************************"
	@echo "Test flagi -funroll-loops: unroll.c"
	@echo "***************************************"
	@echo "Bez opcji:"
	time -f $F ./unroll.out
	@echo "Bez opcji, pod architekturę:"
	time -f $F ./unroll$M.out
	@echo "Z opcją -funroll-loops:"
	time -f $F ./unroll$(OPT).out
	@echo "Z opcją -funroll-loops, pod architekturę:"
	time -f $F ./unroll$(OPT)$M.out
	@echo "***************************************"

clean:
	rm -rfv *.out

.PHONY : all clean tests
