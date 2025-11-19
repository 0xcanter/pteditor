
pteditor: main.c lib/rope.c
	# cd lib && git pull origin master && cd ..
	@gcc -Wall -g lib/rope.c main.c -O2 -o pteditor


run:
	./pteditor

test: test.c lib/rope.c
	# cd lib && git pull origin master && cd ..
	@gcc -Wall -g lib/rope.c test.c -O2 -o pteditor -fsanitize=address
