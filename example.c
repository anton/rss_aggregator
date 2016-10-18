#include "rss_aggregator.h"
#include <stdio.h>
#include <stdlib.h>

void sample_callback(char *title, char *link, char *pubdate, char *description)
{
	fprintf(stderr, "%s [%s]\n", pubdate, title);
	fprintf(stderr, "%s\n", description);
	fprintf(stderr, "%s\n\n", link);
}

int main(int argc, char **argv)
{
	init_rss_aggregator(&sample_callback);

	add_feed("BBC", "http://feeds.bbci.co.uk/news/rss.xml");
	add_feed("NTE", "http://feeds.nytimes.com/nyt/rss/Europe");
	add_feed("NYT", "http://feeds.nytimes.com/nyt/rss/HomePage");
	add_feed("RET", "http://feeds.reuters.com/reuters/topNews");
	add_feed("REW", "http://feeds.reuters.com/Reuters/worldNews");
	add_feed("/. ", "http://rss.slashdot.org/Slashdot/slashdot");
	add_feed("NTG", "http://www.nytimes.com/services/xml/rss/nyt/GlobalHome.xml");

	run_rss_aggregator();

	return(0);
}
