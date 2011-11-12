/*
This file is part of rss_aggregator.

rss_aggregator is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

rss_aggregator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with rss_aggregator.  If not, see <http://www.gnu.org/licenses/>. */

#include "rss_aggregator.h"
#include <stdio.h>
#include <stdlib.h>

void sample_callback(char *title, char *link, char *description)
{
    fprintf(stderr, "NEW ENTRY\n");
    fprintf(stderr, "title %s\n", title);
    fprintf(stderr, "link %s\n", link);
    fprintf(stderr, "description %s\n", description);
}

int main(int argc, char **argv)
{
    init_rss_aggregator(&sample_callback);

    add_feed("http://feeds.bbci.co.uk/news/rss.xml");
    add_feed("http://feeds.nytimes.com/nyt/rss/Europe");
    add_feed("http://feeds.nytimes.com/nyt/rss/HomePage");
    add_feed("http://feeds.reuters.com/reuters/topNews");
    add_feed("http://feeds.reuters.com/Reuters/worldNews");
    add_feed("http://feeds.wired.com/wired/index");
    add_feed("http://rss.slashdot.org/Slashdot/slashdot");
    add_feed("http://www.nytimes.com/services/xml/rss/nyt/GlobalHome.xml");

    run_rss_aggregator();

    return(0);
}
