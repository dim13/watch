/* $Id$ */
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
/*
 * watch -- execute program repeatedly, displaying output fullscreen
 * Based on the original 1991 'watch' by Tony Rems <rembo@unisoft.com>
 * (with mods and corrections by Francois Pinard)
 */

static const char copyright[] =
"@(#) Copyright (c) 2003, 2004 demon <demon@vhost.dyndns.org>\n";
static const char rcsid[] =
"$Id$";
static const char version[] = "0.6.3";

#include <sys/ioctl.h>
#include <sys/time.h>

#include <curses.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#if defined(__linux__)
#define __dead __volatile
#endif

#define BUFSIZE _POSIX_MAX_INPUT

static int readargs(char **);
static int readcmd(void);
static void display(int);
static int title(void);
static void resize(int);
static void settimer(int);
static void die(int);
static __dead void usage(void);

extern char *__progname;
char buffer[BUFSIZE];

int period = 2;
int f_die = 0;
int f_notitle = 0;
int lines;
int cols;

int
main(int argc, char **argv)
{
	int hold_curs;
	char ch;

	while ((ch = getopt(argc, argv, "+s:vn")) != -1)
		switch (ch) {
		case 'v':
			(void) fprintf(stderr, "%s %s %s",
			    __progname, version, copyright + 5);
			exit(1);
			break;
		case 's':
			period = atoi(optarg);
			if (period < 1)
				usage();
			break;
		case 'n':
			f_notitle = 1;
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	if (readargs(argv) == -1)
		if (readcmd() == -1)
			usage();

	signal(SIGINT, die);
	signal(SIGTERM, die);
	signal(SIGHUP, die);

	initscr();
	lines = LINES;
	cols = COLS;
	hold_curs = curs_set(0);

	signal(SIGALRM, display);
	signal(SIGWINCH, resize);
	raise(SIGALRM);
	settimer(period);

	while (f_die == 0)
		pause();

	curs_set(hold_curs);
	endwin();
	exit(0);
}

static int
readargs(char **argv)
{
	int alen;
	int blen;

	if (*argv == NULL)
		return -1;

	alen = strlen(*argv);
	if (alen >= (int) sizeof(buffer))
		return -1;
	memcpy(buffer, *argv, alen);

	while (*++argv != NULL) {
		alen = strlen(*argv);
		blen = strlen(buffer);
		if (alen + blen + 1 >= (int) sizeof(buffer))
			return -1;
		buffer[blen] = ' ';
		memcpy(buffer + blen + 1, *argv, alen);
		buffer[alen + blen + 1] = '\0';
	}
	return 0;
}

static int
readcmd(void)
{
	if (isatty(fileno(stdin)) == 0)
		return -1;
	(void) fprintf(stderr, "command: ");
	if (fgets(buffer, sizeof(buffer), stdin) == NULL)
		return -1;
	buffer[strlen(buffer) - 1] = '\0';
	if (strlen(buffer) == 0)
		return -1;
	return 0;
}

static void
display(int ignored)
{
	int line_count;
	char output[BUFSIZE];
	FILE *pipe;

	(void) ignored;
	move(0, 0);
	line_count = 0;

	if (f_notitle == 0) {
		title();
		line_count = 2;
	}
	pipe = popen(buffer, "r");

	while (fgets(output, sizeof(output), pipe) != NULL &&
	    line_count < lines && cols < (int) sizeof(output)) {
		output[cols] = '\0';
		mvaddstr(line_count++, 0, output);
	}
	pclose(pipe);
	clrtobot();
	refresh();
}

static int
title(void)
{
	int tlen;
	int tlen2;
	char title[BUFSIZE];
	time_t tval;
	struct tm *tm;

	if (cols >= (int) sizeof(title) || cols - 10 < 0)
		return -1;

	tval = time(NULL);
	tm = localtime(&tval);

	snprintf(title, cols, "Every %ds: %s", period, buffer);

	tlen = strlen(title);
	tlen2 = cols - tlen;

	if (tlen2 > 0)
		memset(title + tlen, ' ', tlen2);
	if (tlen2 < 9)
		title[cols - 10] = '>';

	snprintf(title + cols - 9, 10, " %.2d:%.2d:%.2d",
	    tm->tm_hour, tm->tm_min, tm->tm_sec);
	title[cols] = '\0';

	attron(A_BOLD);
	addstr(title);
	attroff(A_BOLD);

	move(2, 0);
	return 0;
}

static void
resize(int ignored)
{
	struct winsize ws;

	(void) ignored;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
		lines = ws.ws_row;
		cols = ws.ws_col;
		resizeterm(lines, cols);
	}
	clear();
	raise(SIGALRM);
}

static void
settimer(int wait)
{
	struct itimerval itv;

	itv.it_value.tv_sec = wait;
	itv.it_value.tv_usec = 0;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void
die(int ignored)
{
	(void) ignored;
	f_die = 1;
}

static __dead void
usage(void)
{
	(void) fprintf(stderr,
	    "usage: %s [-v] [-n] [-s <seconds>] [command]\n", __progname);
	exit(1);
}
