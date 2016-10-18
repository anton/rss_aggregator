all: librss_aggregator.so example

librss_aggregator.so: rss_aggregator.h rss_aggregator.c
	gcc -c -fpic `xml2-config --cflags` rss_aggregator.c
	gcc -shared -lc -o librss_aggregator.so rss_aggregator.o

example: librss_aggregator.so example.c
	gcc example.c -o example -L. -lrss_aggregator `xml2-config --libs`

run: example
	LD_LIBRARY_PATH=./ ./example

clean:
	@touch librss_aggregator.so example a.o
	@rm -f librss_aggregator.so example *.o
