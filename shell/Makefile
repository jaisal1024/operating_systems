CC=gcc
src = $(wildcard *.c)
obj = $(src:.c=.o)

dash: $(obj)
	$(CC) -o $@ $^

.PHONY: clean
clean:
	rm -f $(obj) dash
	rm -f .history
	touch .history
	rm -f *.gch
