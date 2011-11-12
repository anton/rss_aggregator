# This file is part of rss_aggregator.

# rss_aggregator is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# rss_aggregator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with rss_aggregator.  If not, see <http://www.gnu.org/licenses/>.

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
