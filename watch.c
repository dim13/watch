/*
 * watch - execute program periodicaly, showing output fullscreen
 *
 * Copyright (C) 2003 demon <demon@vhost.dyndns.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#define MAXBUF 255

static char copyright[] = "Copyright (C) 2003 demon (demon@vhost.dyndns.org)";
static char version[] = "0.1";
static char terms[] = "This program comes with ABSOLUTELY NO WARANTY.\n\
This is a free software, and you are welcome to redistribute it\n\
under certain conditions. See the file COPYING for deatails.";
extern char *__progname;
time_t tval;

void usage(void);

int main (int argc, char *argv[]) {
    int i;
    int period=5;
    char ch;
    char buf[MAXBUF];
    char curtime[30];
    while ((ch = getopt(argc, argv, "s:v")) != -1)
	switch (ch) {
	    case 'v':
		(void)fprintf(stderr, "%s version %s %s\n", __progname, version, copyright);
		(void)fprintf(stderr, "%s\n", terms);
		exit(1);
		break;
	    case 's':
		period = atoi(optarg);
		if(period < 1)
		    usage();
		break;
	    case '?':
	    default:
		usage();
	}
    argc -= optind;
    argv += optind;

    if(*argv) {
        strcpy(buf, *argv);
	while(*++argv) {
	    strcat(buf, " ");
	    strcat(buf, *argv);
	}
    } else {
	if(isatty(fileno(stdin)))
	    fprintf(stderr, "Command: ");
	(void)fgets(buf, sizeof(buf), stdin);
	buf[strlen(buf) - 1] = '\0';
    }

    if(strlen(buf)) {
	while(1) {
	    time(&tval);
	    strcpy(curtime,ctime(&tval));
	    curtime[strlen(curtime) - 6] = '\0';
	    printf("\e[H\e[2J%s\tEvery %ds: %s\n\n", curtime, period, buf);
	    system(buf);
	    sleep(period);
	}
    } else {
	usage();
    }

    exit(0);
}

void usage(void) {
    (void)fprintf(stderr, "Usage: %s [-s <seconds> | -v ] [command]\n", __progname);
    exit (1);
}
