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

static char *copyright = "(c) 2003 demon <demon@vhost.dyndns.org>";
static char *version   = "0.3 alpha 3";
extern char *__progname;

unsigned int period = 2;
unsigned short die_flag = 0;
unsigned short n_flag = 0;
unsigned short color_flag = 0;;

void loop(char *);
void title(char *);
void resize();
void die();
void usage();

struct buf {
    char *b_val;
    unsigned int b_size;
};

int main (int argc, char **argv) {
    unsigned int a_len;
    unsigned int c_len;
    int hold_curs;
    char ch;
    struct buf cmd;

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
	cmd.b_size = strlen(*argv) + 1;
	if(!(cmd.b_val = (char *)malloc(cmd.b_size)))
	    perror("malloc");
	memcpy(cmd.b_val, *argv, strlen(*argv));

	while(*++argv) {
	    a_len = strlen(*argv);
	    cmd.b_size += a_len + 1;
	    if(!(cmd.b_val = (char *)realloc(cmd.b_val, cmd.b_size)))
		perror("realloc");
	    c_len = strlen(cmd.b_val);
	    cmd.b_val[c_len] = ' ';
	    memcpy(cmd.b_val + c_len + 1, *argv, a_len);
 	}

        initscr();
	hold_curs = curs_set(0);
	if(has_colors())
	    color_flag = 1;

	loop(cmd.b_val);

	curs_set(hold_curs);
        endwin();

	free(cmd.b_val);
	cmd.b_size = 0;

    } else
	usage();

    exit(0);
}

void loop(char *cmd) {
    unsigned int char_count;
    unsigned int line_count;
    char ch;
    FILE *pipe;

    while(!die_flag) {
	char_count = 0;
	line_count = 0;

	clear();

        if(!n_flag) {
            title(cmd);
	    line_count += 2;
	}

        pipe = popen(cmd, "r");

        while((ch = fgetc(pipe)) != EOF) {
	    if((ch == '\0') || (ch == '\n')) {
	    	line_count++;
		char_count = 0;
	    }

	    if(line_count >= LINES)
		break;

	    if(char_count < COLS) {
	        printw("%c",ch);
		char_count++;
	    }
	}

	pclose(pipe);
	refresh();
        sleep(period);
    }
}

void title(char *cmd) {
    unsigned int t_len;
    unsigned int t_len2;

    time_t tval = time(NULL);

    struct buf title;
    struct tm *tm = localtime(&tval);

    title.b_size = COLS + 1;
    if(!(title.b_val = (char *)malloc(title.b_size)))
	perror("malloc");

    snprintf(title.b_val, COLS," Every %ds : %s", period, cmd);

    t_len = strlen(title.b_val);
    t_len2 = COLS - t_len;

    if(t_len2 > 0)
        memset(title.b_val + t_len, ' ', t_len2);

    if(t_len2 < 12)
	title.b_val[COLS - 12] = '>';

    t_len = strlen(title.b_val);

    snprintf(title.b_val + t_len - 11, 12, "  %.2d:%.2d:%.2d ",
	    tm->tm_hour, tm->tm_min, tm->tm_sec);

    title.b_val[COLS] = '\0';

    if(color_flag)
	attron(A_REVERSE);
    
    printw("%s", title.b_val);
    
    if(color_flag)
	attroff(A_REVERSE);

    free(title.b_val);
    title.b_size = 0;

    move(2,0);
}

void resize() {
    struct winsize ws;

    if(!ioctl(STDOUT_FILENO, TIOCGWINSZ, (char *)&ws)) {
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
    (void)fprintf(stderr, "Usage: %s [-v] [-n] [-s <seconds>] command\n", __progname);
    exit (1);
}
