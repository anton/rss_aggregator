#include "utlist.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <libxml/xmlreader.h>
#ifdef LIBXML_READER_ENABLED

enum xmltype { UNDEF, RDF, RSS, FEED };

typedef struct post_element
{
	char *name;
	char *tag;
	time_t pub_epoch;
	char *url;
	char *summary;
	char *pubdate;
	struct post_element *next;
} post_element;

typedef struct feed_element
{
	char *url;
	char *tag;
	enum xmltype type;
	struct feed_element *next;
} feed_element;

post_element *posts_head = NULL;
feed_element *feeds_head = NULL;

void (*new_entry_callback)(char *, char *, char *, char *) = NULL;

int cmp_posts(const post_element *p1, const post_element *p2)
{
	if (p1 == NULL || p2 == NULL)
		return 1;
	return strcmp(p1->name, p2->name);
}

int cmp_epochs(const post_element *p1, const post_element *p2)
{
	if (p1 == NULL || p2 == NULL)
		return 1;
	return p1->pub_epoch < p2->pub_epoch;
}

int add_entry(const post_element *input, const char *timestamp)
{
	//fprintf(stderr, "%s:%d:%s() timestamp %s\n", __FILE__, __LINE__, __func__, timestamp);
	post_element *out = NULL;
	post_element *tmp = (post_element *) malloc(sizeof(post_element));
	tmp->name = strdup(input->name);

	struct tm tm;
	memset(&tm, 0, sizeof(struct tm));

	if (strptime(timestamp, "%a, %d %b %Y %H:%M:%S", &tm) != 0)
	{
		tmp->pub_epoch = mktime(&tm);
		//fprintf(stderr, "%s:%d:%s() pub_epoch %ld\n", __FILE__, __LINE__, __func__, tmp->pub_epoch);
	}

	LL_SEARCH(posts_head, out, tmp, cmp_posts);

	if (out != NULL)
	{
		free(tmp->name);
		free(tmp);
		return 1;
	}

	tmp->url = strdup(input->url);
	tmp->summary = strdup(input->summary);
	tmp->pubdate = strdup(input->pubdate);
	tmp->tag = strdup(input->tag);

	LL_APPEND(posts_head, tmp);

	return 0;
}

void add_feed(const char *tag, const char *feed_url)
{
	feed_element *new_entry = malloc(sizeof(feed_element));
	new_entry->url = strdup(feed_url);
	new_entry->tag = strdup(tag);
	new_entry->type = UNDEF;
	LL_APPEND(feeds_head, new_entry);
}

void init_rss_aggregator(void (*callback)(char *, char *, char *, char *))
{
	new_entry_callback = callback;
}

#define NAME_IS(a)                                                             \
	((0 == xmlStrcmp(name, (const xmlChar *)(a))) &&                       \
	 XML_READER_TYPE_ELEMENT == xmlTextReaderNodeType(reader))

char * check_and_get(const xmlChar *name, const char *what, xmlTextReaderPtr reader)
{
	const xmlChar *value;
	if ((!xmlStrcmp(name, (const xmlChar *)what)) &&
	    XML_READER_TYPE_ELEMENT == xmlTextReaderNodeType(reader))
	{
		(void)xmlTextReaderRead(reader);
		value = xmlTextReaderConstValue(reader);
		return (char *)value;
	}

	return NULL;
}

int process_xml(xmlTextReaderPtr reader, struct feed_element *feed)
{
	enum xmltype type = feed->type;

	if (type == UNDEF)
	{
		return 1;
	}

	int ret;
	const xmlChar *name;

	if (type == RDF || type == RSS)
	{
		ret = xmlTextReaderRead(reader);
		name = xmlTextReaderConstName(reader);

		if (xmlStrcmp(name, (const xmlChar *)"channel"))
		{
			ret = xmlTextReaderRead(reader);
			name = xmlTextReaderConstName(reader);
		}

		if (xmlStrcmp(name, (const xmlChar *)"channel"))
		{
			fprintf(stderr,
					"Bad document: did not immediately find the channel"
					" element.\n");
			fprintf(stderr, "Got %s\n", name);
			return 1;
		}
	}

	if (type == RDF)
	{
		ret = xmlTextReaderRead(reader);
		ret = xmlTextReaderRead(reader);
	}

	if (type == FEED)
	{
		ret = xmlTextReaderRead(reader);
	}

	while (ret == 1)
	{
		name = xmlTextReaderConstName(reader);
		if ( ((type == RDF || type == RSS) && (!xmlStrcmp(name, (const xmlChar *)"item"))) ||
		     ((type == FEED) && (!xmlStrcmp(name, (const xmlChar *)"entry"))))
		{
			char title[256];
			char link[256];
			char description[4096];
			char pubdate[256];
			char timestamp[256];
			title[0] = '\0';
			link[0] = '\0';
			description[0] = '\0';
			pubdate[0] = '\0';
			timestamp[0] = '\0';
			pubdate[11] = '\0';
			pubdate[26] = '\0';
			for (;;)
			{
				name = xmlTextReaderConstName(reader);
				//fprintf(stderr, "%s:%d:%s(): name %s \n", __FILE__, __LINE__, __func__, name);

				if (NAME_IS("title"))
					strncpy(title, check_and_get(name, "title", reader), 256);
				if (NAME_IS("pubDate"))
				{
					strncpy(timestamp, check_and_get(name, "pubDate", reader), 26);
					strncpy(pubdate, timestamp, 11);
				}

				if (type == RDF || type == RSS)
				{
					if (NAME_IS("link"))
						strncpy(link, check_and_get( name, "link", reader), 256);
					if (NAME_IS("description"))
						strncpy(description, check_and_get(name, "description", reader), 4096);
				}

				if (type == FEED)
				{
					if (NAME_IS("id"))
						strncpy(link, check_and_get(name, "id", reader), 256);
					if (NAME_IS("summary"))
						strncpy(description, check_and_get(name, "summary", reader), 4096);
				}

				if (strlen(description) && strlen(pubdate))
				{
					//fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__, __func__);
					post_element new_entry;
					new_entry.name = title;
					new_entry.url = link;
					new_entry.summary = description;
					new_entry.pubdate = pubdate;
					new_entry.tag = feed->tag;

					if (add_entry(&new_entry, timestamp) == 0)
					{
						(*new_entry_callback)(title, link, pubdate, description);
					}
					title[0] = '\0';
					link[0] = '\0';
					description[0] = '\0';
					pubdate[0] = '\0';
					timestamp[0] = '\0';
				}

				if (type == RDF || type == RSS)
				{
				if ((!xmlStrcmp(name, (const xmlChar *)"item")) && XML_READER_TYPE_END_ELEMENT == xmlTextReaderNodeType(reader))
				{
					break;
				}
				}

				if (type == FEED)
				{
				if ((!xmlStrcmp(name, (const xmlChar *)"entry")) && XML_READER_TYPE_END_ELEMENT == xmlTextReaderNodeType(reader))
				{
					break;
				}
				}
				ret = xmlTextReaderRead(reader);
			}
		}
		ret = xmlTextReaderRead(reader);
	}
	if (ret != 0)
	{
		fprintf(stderr, "failed to parse\n");
	}
	return 0;
}

void check_for_updates(feed_element *feed)
{
	LIBXML_TEST_VERSION

	xmlTextReaderPtr reader;
	int ret;
	const xmlChar *name;

	reader = xmlReaderForFile(feed->url, NULL, 0);

	if (reader == NULL)
	{
		fprintf(stderr, "Unable to open %s\n", feed->url);
		return;
	}

	do
	{
		ret = xmlTextReaderRead(reader);
		if (XML_READER_TYPE_ELEMENT == xmlTextReaderNodeType(reader))
		{
			break;
		}
	} while (ret == 1);

	name = xmlTextReaderConstName(reader);

	if (feed->type == UNDEF)
	{
		if (xmlStrstr(name, (const xmlChar *)"rss"))
		{
			feed->type = RSS;
		}
		else if (xmlStrstr(name, (const xmlChar *)"rdf") ||
				xmlStrstr(name, (const xmlChar *)"RDF"))
		{
			feed->type = RDF;
		}
		else if (xmlStrstr(name, (const xmlChar *)"feed"))
		{
			feed->type = FEED;
		}
		else
		{
			fprintf(stderr, "Bad document type\n");
		}
	}

	if (process_xml(reader, feed) != 0)
	{
		fprintf(stderr, "Error processing %s\n", feed->url);
	}

	xmlFreeTextReader(reader);
	xmlCleanupParser();
	xmlMemoryDump();
}

void html_header()
{
	FILE *fp;
	fp = fopen("collection.html", "w");
	fprintf(fp, "<!DOCTYPE html>\n");
	fprintf(fp, "<html>\n");
	fprintf(fp, "<head>\n");
	fprintf(fp, "<link rel=\"stylesheet\" type=\"text/css\" "
	            "href=\"style.css\" />\n");
	fprintf(fp, "</head>\n");
	fprintf(fp, "<body>\n");
	fprintf(fp, "<table>\n");
	fclose(fp);
}

void html_footer()
{
	FILE *fp;
	fp = fopen("collection.html", "a");
	fprintf(fp, "</table>\n");
	fprintf(fp, "</body>\n");
	fprintf(fp, "</html>\n");
	fclose(fp);
}

void run_rss_aggregator(unsigned int wait)
{
	feed_element *feed;
	if (wait < 900)
	{
		wait = 900;
	}

	for (;;)
	{
		sleep(3);
		LL_FOREACH(feeds_head, feed)
		{
			check_for_updates(feed);
		}

		LL_SORT(posts_head, cmp_epochs);

		html_header();
		post_element *item = NULL;
		LL_FOREACH(posts_head, item)
		{
			FILE *fp;
			fp = fopen("collection.html", "a");
			fprintf(fp, "<tr><td>%s <a href=\"%s\">%s</a> [%s]</td></tr>\n", item->pubdate, item->url, item->name, item->tag);
			fclose(fp);
		}
		html_footer();
		sleep(wait);
	}
}

#endif // LIBXML_READER_ENABLED
