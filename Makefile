CFLAGS := -W -Wall \
	  -Wformat=2 \
	  -Wunused \
	  -Werror \
	  -Wcast-align \
	  -pedantic-errors \
	  -fstack-protector-all \
	  -Wwrite-strings \
	  -Wcast-align \
	  -Wno-format -Wno-long-long \
	  -std=gnu99 \
	  -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations \
	  -Wold-style-definition \
	  -Wbad-function-cast \
	  -Wnested-externs \
	  -Wfloat-equal \
	  -Wpointer-arith \
#	  -Wdeclaration-after-statement \
#	  -pedantic \

CC = gcc $(CFLAGS)


bins := edgelist2mtx edgelist2mtx-debug

all: edgelist2mtx

debug: edgelist2mtx-debug


edgelist2mtx: src/edgelist2mtx.c
	$(CC) -O3 -o $@ $<

edgelist2mtx-debug: src/edgelist2mtx.c
	$(CC) -g -o $@ $<

clean:
	rm -f $(bins)
	find * -name \*~ -delete

