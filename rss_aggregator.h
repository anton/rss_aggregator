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

#ifndef hg_RSS_AGGREGATOR_
#define hg_RSS_AGGREGATOR_

void init_rss_aggregator(void (*fn)(char *, char *, char *));
void add_feed(const char *feed_url);
void run_rss_aggregator(void);

#endif
