/* $Id$ */
/*
 * watch -- execute program repeatedly, displaying output fullscreen
 * Based on the original 1991 'watch' by Tony Rems <rembo@unisoft.com>
 * (with mods and corrections by Francois Pinard)
 */
/*
 * Copyright (c) 2003 demon <demon@vhost.dymdns.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <curses.h>
#define DEBUG 1

static char *copyright = "(c) 2003 demon <demon@vhost.dyndns.org>";
static char *version   = "0.3 alpha";
extern char *__progname;

short die_flag;
short n_flag=0;
int period=2;

void loop(char *);
void title(char *);
void usage();
void die();

int main (int argc, char **argv) {
    char ch;
    char *cmd;

    signal(SIGINT, die);
    signal(SIGTERM, die);
    signal(SIGHUP, die);

    
    while ((ch = getopt(argc, argv, "s:vn")) != -1)
	switch (ch) {
	    case 'v':
		(void)fprintf(stderr, "%s %s %s\n", __progname, version, copyright);
		exit(1);
		break;
	    case 's':
		period = atoi(optarg);
		if(period < 1)
		    usage();
		break;
	    case 'n':
		n_flag=1;
		break;
	    case '?':
	    default:
		usage();
	}
    argc -= optind;
    argv += optind;

    if(*argv) {
	cmd = (char *)malloc(strlen(*argv) + 1);
	memcpy(cmd, *argv, strlen(*argv));
	while(*++argv) {
	    cmd = (char *)realloc(cmd, strlen(cmd) + strlen(*argv) + 2);
	    cmd[strlen(cmd)]= ' ';
	    memcpy(cmd + strlen(cmd), *argv, strlen(*argv));
 	}
	loop(cmd);
	free(cmd);
    } else
	usage();

    exit(0);
}

void loop(char *cmd) {
    char buf;
    FILE *pipe;

    initscr();
    while(!die_flag) {
        move(0,0);
        if(!n_flag)
            title(cmd);
        pipe = popen(cmd, "r");
        while((buf = fgetc(pipe)) != EOF)
    	    printw("%c",buf);
	refresh();
	pclose(pipe);
	sleep(period);
    }
    endwin();
}

void title(char *cmd) {
    char *curtime;
    time_t tval;

    time(&tval);
    curtime = strdup(ctime(&tval));
    curtime[strlen(curtime) - 6] = '\0';
    printw("%s\tEvery %ds: %s", curtime, period, cmd);
    move(2,0);
}

void usage() {
    (void)fprintf(stderr, "Usage: %s [-vns <seconds>] [command]\n", __progname);
    exit (1);
}

void die() {
    die_flag=1;
}
