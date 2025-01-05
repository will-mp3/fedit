CFLAGS= -Wall -Wextra -Werror -ggdb

prog = fedit
objects = fedit.o mu.o
headers = mu.h

$(prog): $(objects)
	$(CC) -o $@ $^

$(objects) : %.o : %.c $(headers)
	$(CC) -o $@ -c $(CFLAGS) $<

clean:
	rm -f $(prog) $(objects)

.PHONY: clean
