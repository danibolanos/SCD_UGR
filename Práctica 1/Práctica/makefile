#Para compilar hacer make fumadores2

.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe

compilador:=g++

fumadores2: fumadores2.cpp
		$(compilador) -std=c++11 -I. -o $@ $< Semaphore.cpp -lpthread

clean:
	rm -f fumadores2
