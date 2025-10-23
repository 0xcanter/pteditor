
pteditor: main.c lib/rope.c
	@gcc -Wall -g lib/rope.c main.c -o pteditor -fsanitize=address


run:
	./pteditor

test: test.c lib/rope.c
	@gcc -Wall -g lib/rope.c test.c -o test -fsanitize=address
