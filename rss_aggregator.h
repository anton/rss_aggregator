#ifndef hg_RSS_AGGREGATOR_
#define hg_RSS_AGGREGATOR_

void init_rss_aggregator(void (*fn)(char *, char *, char *, char *));
void add_feed(const char *tag, const char *feed_url);
void run_rss_aggregator(void);

#endif
