/* $Id$ */
/*
 * watch -- execute program repeatedly, displaying output fullscreen
 * Based on the original 1991 'watch' by Tony Rems <rembo@unisoft.com>
 * (with mods and corrections by Francois Pinard)
 */
/*
 * Copyright (c) 2003, 2004 demon <demon@vhost.dymdns.org>
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <curses.h>
#include <errno.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <sys/time.h>

static char buffer[_POSIX_MAX_INPUT];
static char *copyright = "(c) 2003, 2004 demon <demon@vhost.dyndns.org>";
static char *version = "0.5";
extern char *__progname;

unsigned int period = 2;
unsigned short die_flag = 0;
unsigned short n_flag = 0;
unsigned short color_flag = 0;

static int lines, cols;

int readargs(char **);
void display();
int title();
void resize();
void settimer(int);
void die();
void usage();

int
main(int argc, char **argv)
{
	int hold_curs;
	char ch;

	while ((ch = getopt(argc, argv, "+s:vn")) != -1)
		switch (ch) {
		case 'v':
			(void) fprintf(stderr, "watch %s %s\n",
			    version, copyright);
			exit(1);
			break;
		case 's':
			period = atoi(optarg);
			if (period < 1)
				usage();
			break;
		case 'n':
			n_flag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (readargs(argv) == -1)
		usage();	/* does not return */

	signal(SIGINT, die);
	signal(SIGTERM, die);
	signal(SIGHUP, die);

	initscr();
	lines = LINES;
	cols = COLS;
	signal(SIGWINCH, resize);

	hold_curs = curs_set(0);
	if (has_colors())
		color_flag = 1;

	signal(SIGALRM, display);
	settimer(period);
	display();

	while (die_flag == 0)
		pause();

	curs_set(hold_curs);
	endwin();
	exit(0);
}

int
readargs(char **argv)
{
	int alen, blen;

	if (*argv == NULL)
		return -1;

	alen = strlen(*argv);
	if (alen + 1 >= _POSIX_MAX_INPUT)
		return -1;
	memcpy(buffer, *argv, alen);

	while (*++argv != NULL) {
		alen = strlen(*argv);
		blen = strlen(buffer);
		if (alen + blen + 1 >= _POSIX_MAX_INPUT)
			return -1;
		buffer[blen] = ' ';
		memcpy(buffer + blen + 1, *argv, alen);
		buffer[blen + alen + 1] = '\0';
	}
	return 0;
}

void
display()
{
	char ch;
	FILE *pipe;
	int char_count = 0;
	int line_count = 0;

	clear();

	if (n_flag == 0) {
		title();
		line_count = 2;
	}
	pipe = popen(buffer, "r");

	while ((ch = fgetc(pipe)) != EOF) {
		if ((ch == '\0') || (ch == '\n')) {
			line_count++;
			char_count = 0;
		}
		if (line_count >= lines)
			break;
		if (char_count < cols) {
			printw("%c", ch);
			char_count++;
		}
	}

	pclose(pipe);
	refresh();
}

int
title()
{
	char title[_POSIX_MAX_INPUT];
	int tlen, tlen2;

	time_t tval = time(NULL);
	struct tm *tm = localtime(&tval);

	if (cols + 1 >= _POSIX_MAX_INPUT)
		return -1;

	snprintf(title, cols, " Every %ds : %s", period, buffer);

	tlen = strlen(title);
	tlen2 = cols - tlen;

	if (tlen2 > 0)
		memset(title + tlen, ' ', tlen2);

	if (tlen2 < 12)
		title[cols - 12] = '>';

	snprintf(title + cols - 11, 12, "  %.2d:%.2d:%.2d ",
	    tm->tm_hour, tm->tm_min, tm->tm_sec);

	title[cols] = '\0';

	if (color_flag == 1)
		attron(A_REVERSE);

	printw("%s", title);

	if (color_flag == 1)
		attroff(A_REVERSE);

	move(2, 0);
	return 0;
}

void
resize()
{
	int save_errno = errno;
	struct winsize ws;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
		lines = ws.ws_row;
		cols = ws.ws_col;
		resizeterm(lines, cols);
	}
	raise(SIGALRM);
	errno = save_errno;
}

void
settimer(int wait)
{
	int save_errno = errno;
	struct itimerval itv;

	itv.it_value.tv_sec = wait;
	itv.it_value.tv_usec = 0;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
	errno = save_errno;
}

void
die()
{
	die_flag = 1;
}

void
usage()
{
	(void) fprintf(stderr, "Usage: %s [-v] [-n] [-s <seconds>] command\n",
	    __progname);
	exit(1);
}
