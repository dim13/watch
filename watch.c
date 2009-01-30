/* $Id$ */
/*
 * Copyright (c) 2003 Dimitri Sokolyuk <demon@vhost.dymdns.org>
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

#ifndef lint
static const char rcsid[] =
"$Id$";
#endif /* not lint */

static const char version[] = "2.0";

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <curses.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#if defined(__linux__)
#ifndef __dead
#define __dead	__attribute__((noreturn))
#endif
#define strlcpy(d,s,l)	(strncpy(d,s,l), (d)[(l) - 1] = '\0')
#define strlcat(d,s,l)	strncat(d,s,(l) - strlen(d) - 1)
#endif

#ifndef DELAY
#define DELAY	2	/* default delay between screen updates in seconds */
#endif

#define BUFSIZE		_POSIX_MAX_INPUT

#define F_DIE		0x01
#define F_TITLE		0x02
#define F_UPDATE	0x04
#define F_RESIZE	0x08

int	flags = F_TITLE|F_UPDATE;

extern	char *__progname;
extern	int LINES, COLS;

void	catchsig(int);
int	readargs(char **, char *, size_t);
int	readcmd(char *, size_t);
int	display(WINDOW *, char *, char *, size_t);
void	title(WINDOW *, char *, int);
void	resize(void);
void	settimer(int);
__dead	void usage(void);

void
catchsig(int sig)
{
	switch (sig) {
	case SIGWINCH:
		flags |= F_RESIZE;
		/* FALLTHROUGH */
	case SIGALRM:
		flags |= F_UPDATE;
		break;
	case SIGINT:
	case SIGTERM:
	case SIGHUP:
	default:
		flags |= F_DIE;
		break;
	}
}

int
main(int argc, char **argv)
{
	WINDOW	*titlew;
	WINDOW	*outw;
	struct	sigaction sa;
	char	buf[BUFSIZE];
	char	cmd[BUFSIZE + 5];
	char	out[BUFSIZE];
	int	hold_curs;
	int	ret = -1;
	int	delay = DELAY;
	int	ch;

	while ((ch = getopt(argc, argv, "+n:tv")) != -1)
		switch (ch) {
		case 'n':
			delay = atoi(optarg);
			if (delay < 1)
				usage();
				/* NOTREACHED */
			break;
		case 't':
			flags &= ~F_TITLE;
			break;
		case 'v':
			(void)fprintf(stderr, "%s %s\n", __progname, version);
			exit(1);
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}

	argc -= optind;
	argv += optind;

	memset(buf, 0, sizeof(buf));

	if (readargs(argv, buf, sizeof(buf)) != 0 && readcmd(buf, sizeof(buf)) != 0)
		usage();
		/* NOTREACHED */

	memcpy(cmd, buf, sizeof(buf));
	strlcat(cmd, " 2>&1", sizeof(cmd));

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = catchsig;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGWINCH, &sa, NULL);

	initscr();
	hold_curs = curs_set(0);

	if (flags & F_TITLE) {
		titlew = newwin(1, 0, 0, 0);
		wattron(titlew, A_BOLD);
		outw = newwin(LINES - 2, 0, 2, 0);
	} else {
		titlew = NULL;
		outw = stdscr;
	}

	settimer(delay);

	for (;;) {
		if (flags & F_RESIZE) {
			resize();
			if (flags & F_TITLE) {
				wresize(titlew, 1, COLS); 
				clearok(titlew, TRUE);
				wresize(outw, LINES - 2, COLS); 
			}
			clearok(outw, TRUE);
			flags &= ~F_RESIZE;
		}

		if (flags & F_UPDATE) {
			if (flags & F_TITLE)
				title(titlew, buf, delay);

			ret = display(outw, cmd, out, sizeof(out));
			doupdate();
			flags &= ~F_UPDATE;
		}

		if (flags & F_DIE)
			break;

		sigsuspend(&sa.sa_mask);
	}

	if (flags & F_TITLE) {
		delwin(outw);
		delwin(titlew);
	}
	curs_set(hold_curs);
	endwin();

	if (ret != 0)
		(void)fprintf(stderr, "%s: %s", __progname, out);

	return ret;
}

int
readargs(char **argv, char *buf, size_t sz)
{
	if (*argv == NULL)
		return -1;

	while (*argv) {
		strlcat(buf, *argv++, sz);
		strlcat(buf, " ", sz);
	}
	return 0;
}

int
readcmd(char *buf, size_t sz)
{
	if (isatty(fileno(stdin))) {
		(void)fprintf(stderr, "command: ");
		if (fgets(buf, sz - 1, stdin) != NULL
			&& strlen(buf) > 1) {
			buf[strlen(buf) - 1] = '\0';
			return 0;
		}
	}
	return -1;
}

int
display(WINDOW *outw, char *cmd, char *out, size_t sz)
{
	FILE	*pipe;
	int	ret = -1;
	int	y, x;

	if ((pipe = popen(cmd, "r")) != NULL) {
		wmove(outw, 0, 0);
		getmaxyx(outw, y, x);

		while (fgets(out, sz - 1, pipe) != NULL && y--)
			waddnstr(outw, out, x);

		if ((ret = pclose(pipe)) == 0) {
			wclrtobot(outw);
			wnoutrefresh(outw);
			return 0;
		}
	}
	raise(SIGINT);
	return WEXITSTATUS(ret);
}

void
title(WINDOW *titlew, char *buf, int delay)
{
	time_t	tval;
	struct	tm *tm;
	int	y, x;

	tval = time(NULL);
	tm = localtime(&tval);

	werase(titlew);
	mvwprintw(titlew, 0, 0, "Every %ds: %s", delay, buf);

	getyx(titlew, y, x);
	if (x > COLS - 8)
		mvwaddstr(titlew, 0, COLS - 12, "... ");

	mvwprintw(titlew, 0, COLS - 8, "%.2d:%.2d:%.2d",
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	wnoutrefresh(titlew);
}

void
resize(void)
{
	struct	winsize ws;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1)
		resizeterm(ws.ws_row, ws.ws_col);

	raise(SIGALRM);
}

void
settimer(int sec)
{
	struct	itimerval itv;

	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = 0;
	itv.it_interval = itv.it_value;

	setitimer(ITIMER_REAL, &itv, NULL);
}

__dead void
usage(void)
{
	(void)fprintf(stderr,
		"usage: %s [-tv] [-n time] [command]\n", __progname);
	exit(1);
}
