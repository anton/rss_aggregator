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

#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <libxml/xmlreader.h>
#include "utlist.h"

#ifdef LIBXML_READER_ENABLED

static int initial_read;
static int create_failed;
static unsigned int hash_size;

typedef struct el {
    char *name;
    struct el *next;
} el;

el *feed_head = NULL;
el *key_head = NULL;

void (*new_entry_fn)(char *, char *, char *) = NULL;

/**
 * add_entry:
 * @key: the key to be added
 *
 * Add an entry to the hash
 */
static void add_entry(const char *key)
{
    ENTRY e, *ep;
    el *new_key = malloc(sizeof(el));
    fprintf(stderr, "Adding %s\n", key);

    new_key->name = strdup(key);
    LL_APPEND(key_head, new_key);

    e.key = new_key->name;
    ep = hsearch(e, ENTER);
    if (ep == NULL) {
        fprintf(stderr, "entry failed\n");
        create_failed = 1;
    }
}

/**
 * check_and_print:
 * @name: tag name
 * @what: what to search for
 * @reader: the xmlReader
 *
 * Helper function for printing value if @name and @what match
 */
static void check_and_print(const xmlChar *name,
        const char *what,
        xmlTextReaderPtr reader)
{
    const xmlChar *value;
    if ((!xmlStrcmp(name, (const xmlChar *)what)) &&
            XML_READER_TYPE_ELEMENT ==
            xmlTextReaderNodeType(reader)) {
        (void)xmlTextReaderRead(reader);
        value = xmlTextReaderConstValue(reader);
        printf("%s: %s\n", what, (char *)value);
    }
}

/**
 * NAMEIS:
 * @a: char * to match with
 *
 * Helper macro function for checking if const xmlChar *name is matching with a.
 */
#define NAME_IS(a) ((0 == xmlStrcmp(name, (const xmlChar *)(a))) && \
        XML_READER_TYPE_ELEMENT == xmlTextReaderNodeType(reader))

/**
 * check_and_get:
 * @name: tag name
 * @what: what to search for
 * @reader: the xmlReader
 *
 * Helper function for getting value if @name and @what match
 */
static char * check_and_get(const xmlChar *name,
        const char *what,
        xmlTextReaderPtr reader)
{
    const xmlChar *value;
    if ((!xmlStrcmp(name, (const xmlChar *)what)) &&
            XML_READER_TYPE_ELEMENT ==
            xmlTextReaderNodeType(reader)) {
        (void)xmlTextReaderRead(reader);
        value = xmlTextReaderConstValue(reader);
        /* printf("%s: %s\n", what, (char *)value); */
        return (char *)value;
    }

    return NULL;
}

/**
 * process_feed:
 * @reader: the xmlReader
 *
 * Process the feed type
 */
static void process_feed(xmlTextReaderPtr reader) {
    int ret;
    const xmlChar *name, *value;
    ret = xmlTextReaderRead(reader);
    while (ret == 1) {
        name = xmlTextReaderConstName(reader);
        if ((!xmlStrcmp(name, (const xmlChar *)"entry"))) {
            char title[256];
            char id[256];
            char summary[256];
            title[0] = '\0';
            id[0] = '\0';
            summary[0] = '\0';
            for(;;) {
                name = xmlTextReaderConstName(reader);
                if (NAME_IS("title"))
                    strncpy(title, check_and_get(name, "title", reader), 256);
                if (NAME_IS("id"))
                    strncpy(id, check_and_get(name, "id", reader), 256);
                if (NAME_IS("summary"))
                    strncpy(summary, check_and_get(name, "summary", reader),
                            256);

                if (strlen(summary)) {
                    if (initial_read) {
                        add_entry(title);
                        fprintf(stderr, "id: %s\n", id);
                        fprintf(stderr, "summary: %s\n", summary);
                    } else {
                        ENTRY e, *ep;
                        e.key = title;
                        ep = hsearch(e, FIND);
                        if (!ep) {
                            add_entry(title);
                            (*new_entry_fn)(id, title, summary);
                        }
                    }
                    title[0] = '\0';
                    id[0] = '\0';
                    summary[0] = '\0';
                }

                /* check_and_print(name, "summary", reader); */
                if ((!xmlStrcmp(name, (const xmlChar *)"entry")) &&
                        XML_READER_TYPE_END_ELEMENT ==
                        xmlTextReaderNodeType(reader)) {
                    break;
                }
                ret = xmlTextReaderRead(reader);
            }
        }
        ret = xmlTextReaderRead(reader);
    }
    if (ret != 0) {
        fprintf(stderr, "failed to parse\n");
    }
}

/**
 * process_rss_rdf:
 * @reader: the xmlReader
 * @rdf: 0 for rss and 1 for rdf
 *
 * Process the rss and rdf type
 */
static void process_rss_rdf(xmlTextReaderPtr reader, const int rdf)
{
    int ret;
    const xmlChar *name, *value;
    ret = xmlTextReaderRead(reader);
    ret = xmlTextReaderRead(reader);

    name = xmlTextReaderConstName(reader);

    if (xmlStrcmp(name, (const xmlChar *)"channel")) {
            fprintf(stderr, "Bad document: did not immediately find the channel"
                    " element.\n");
        return;
    }

    if (rdf) {
        ret = xmlTextReaderRead(reader);
        ret = xmlTextReaderRead(reader);
    }

    while (ret == 1) {
        name = xmlTextReaderConstName(reader);
        if ((!xmlStrcmp(name, (const xmlChar *)"item"))) {
            char title[256];
            char link[256];
            char description[256];
            title[0] = '\0';
            link[0] = '\0';
            description[0] = '\0';
            for(;;) {
                name = xmlTextReaderConstName(reader);

                if (NAME_IS("title"))
                    strncpy(title, check_and_get(name, "title", reader), 256);
                if (NAME_IS("link"))
                    strncpy(link, check_and_get(name, "link", reader), 256);
                if (NAME_IS("description"))
                    strncpy(description,
                            check_and_get(name, "description", reader), 256);

                if (strlen(description)) {
                    if (initial_read) {
                        add_entry(title);
                        fprintf(stderr, "link: %s\n", link);
                        fprintf(stderr, "description: %s\n", description);
                    } else {
                        ENTRY e, *ep;
                        e.key = title;
                        ep = hsearch(e, FIND);
                        if (!ep) {
                            add_entry(title);
                            (*new_entry_fn)(title, link, description);
                        }
                    }
                    title[0] = '\0';
                    link[0] = '\0';
                    description[0] = '\0';
                }

                if ((!xmlStrcmp(name, (const xmlChar *)"item")) &&
                        XML_READER_TYPE_END_ELEMENT ==
                        xmlTextReaderNodeType(reader)) {
                    break;
                }
                ret = xmlTextReaderRead(reader);
            }
        }
        ret = xmlTextReaderRead(reader);
    }
}

/**
 * streamFile:
 * @filename: the file name to parse
 *
 * Parse and print information about an XML file.
 */
static void
streamFile(const char *filename) {
    xmlTextReaderPtr reader;
    int ret;
    const xmlChar *name, *value;

    reader = xmlReaderForFile(filename, NULL, 0);
    if (reader == NULL) {
        fprintf(stderr, "Unable to open %s\n", filename);
        return;
    }

    do {
        ret = xmlTextReaderRead(reader);
        if (XML_READER_TYPE_ELEMENT == xmlTextReaderNodeType(reader)) {
            break;
        }
    } while(ret == 1);

    name = xmlTextReaderConstName(reader);
    /* printf("name %s\n", (char *)name); */
    if (xmlStrstr(name, (const xmlChar *)"rss")) {
        process_rss_rdf(reader, /* rss */ 0);
    } else if (xmlStrstr(name, (const xmlChar *)"rdf") ||
            xmlStrstr(name, (const xmlChar *)"RDF")) {
        process_rss_rdf(reader, /* rdf */ 1);
    } else if (xmlStrstr(name, (const xmlChar *)"feed")) {
        process_feed(reader);
    } else {
        printf("Bad document type\n");
    }

    xmlFreeTextReader(reader);
}

static void check_for_updates(const char *filename)
{
    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    fprintf(stderr, "%s\n", __FUNCTION__);

    if (initial_read) {
        hcreate(hash_size);
    }

    streamFile(filename);

    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();
    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
}

void add_feed(const char *feed_url)
{
    el *new_entry = malloc(sizeof(el));
    new_entry->name = strdup(feed_url);
    LL_APPEND(feed_head, new_entry);
    hash_size += 20;
};

void init_rss_aggregator(void (*fn)(char *, char *, char *))
{
    new_entry_fn = fn;
    initial_read = 1;
    hash_size = 50;
    create_failed = 0;
};

void recreate_hash(void)
{
    el *elt, *tmp;
    initial_read = 1;
    while(create_failed) {
        create_failed = 0;
        fprintf(stderr, "Recreating hash with size %d\n", hash_size);
        hdestroy();
        LL_FOREACH_SAFE(key_head, elt, tmp) {
            LL_DELETE(key_head, elt);
            free(elt->name);
        }
        hcreate(hash_size);
        LL_FOREACH(feed_head, elt) {
            check_for_updates(elt->name);
        }
        if (create_failed) {
            hash_size *= 1.1;
            fprintf(stderr, "Resizing hash to %d\n", hash_size);
        }
        sleep(30);
    }
    initial_read = 0;
}

void run_rss_aggregator(void)
{
    el *elt;
    LL_FOREACH(feed_head, elt)
        check_for_updates(elt->name);

    recreate_hash();

    for(;;) {
        sleep(300);
        LL_FOREACH(feed_head, elt)
            check_for_updates(elt->name);
        if (create_failed)
            recreate_hash();
    }
}

#else
int main(void) {
    fprintf(stderr, "XInclude support not compiled in\n");
    exit(1);
}
#endif
