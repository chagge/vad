all:
	gcc -o vad test.c wb_vad.c -lm
clean:
	rm -f vad
