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
#include <sys/ioctl.h>
#define DEBUG 1

static char *copyright = "(c) 2003 demon <demon@vhost.dyndns.org>";
static char *version   = "0.3 alpha 2";
extern char *__progname;

unsigned short die_flag = 0;
unsigned short n_flag = 0;
unsigned short color_flag = 0;;
unsigned int period = 2;

void loop(char *);
void title(char *);
void resize();
void die();
void usage();

int main (int argc, char **argv) {
    char ch;
    char *cmd = NULL;

    signal(SIGINT, die);
    signal(SIGTERM, die);
    signal(SIGHUP, die);
    signal(SIGWINCH, resize);
    
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

        initscr();
	if(has_colors())
	    color_flag = 1;
	loop(cmd);
        endwin();
	free(cmd);

    } else
	usage();

    exit(0);
}

void loop(char *cmd) {
    char buf;
    FILE *pipe;
    int i,j;

    while(!die_flag) {
	i = 0; j = 0;
	clear();
        if(!n_flag) {
            title(cmd);
	    i += 2;
	}
        pipe = popen(cmd, "r");
        while((buf = fgetc(pipe)) != EOF) {
	    if((buf == '\0') || (buf == '\n')) {
	    	i++;
		j = 0;
	    }
	    if(i >= LINES)
		break;
	    if(j < COLS) {
	        printw("%c",buf);
		j++;
	    }
	}
	pclose(pipe);
	refresh();
        sleep(period);
    }
}

void title(char *cmd) {
    char *title = NULL;
    time_t tval = time(NULL);
    int len;

    struct tm *tm = localtime(&tval);

    title = (char *)malloc(COLS + 1);
    snprintf(title, COLS," Every %ds   %s", period, cmd);
    len = strlen(title);
    if((COLS - len) > 0)
        memset(title + len, ' ', COLS - len);
    len = strlen(title);
    snprintf(title + len - 9, 10, "%.2d:%.2d:%.2d ",
	    tm->tm_hour, tm->tm_min, tm->tm_sec);
    title[COLS] = '\0';

    if(color_flag)
	attron(A_REVERSE);
    
    printw("%s", title);
    
    if(color_flag)
	attroff(A_REVERSE);

    free(title);
    move(2,0);
}

void resize() {
    struct winsize ws;

    if(!ioctl(NULL, TIOCGWINSZ, &ws)){
	LINES = ws.ws_row;
	COLS = ws.ws_col;
        resizeterm(LINES, COLS);
        clear();
    }
}

void die() {
    die_flag=1;
}

void usage() {
    (void)fprintf(stderr, "Usage: %s [-vns <seconds>] [command]\n", __progname);
    exit (1);
}
