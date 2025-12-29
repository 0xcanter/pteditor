
pteditor: main.c lib/rope.c
	@gcc -Wall -g lib/rope.c main.c -O2 -o pteditor


run:
	./pteditor tender.txt

test: main.c lib/rope.c
	@gcc -Wall -g lib/rope.c main.c  -o pteditor -fsanitize=address

rope: test.c lib/rope.c
	@gcc -Wall -g lib/rope.c test.c -O2 -o rope

start:
	./rope	
