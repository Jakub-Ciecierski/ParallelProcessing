N=$(MAKECMDGOALS)

ifeq ($(N),run)
	N=10
else
	N=$(filter-out $@,$(MAKECMDGOALS))
endif

all: compile

compile:
	cc -pthread main.c -o main
	
run: 
	./main $(N)