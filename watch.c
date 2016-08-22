/*
 * Copyright (c) 2003 Dimitri Sokolyuk <demon@dim13.org>
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
#include <sysexits.h>

#ifndef DELAY
#define DELAY	2	/* default delay between screen updates in seconds */
#endif

int	die_flag = 0;
int	title_flag = 1;
int	resize_flag = 0;

extern	char *__progname;
extern	int LINES, COLS;

void	catchsig(int);
int	readargs(char **, char *, size_t);
int	readcmd(char *, size_t);
int	display(WINDOW *, char *, char *, size_t);
void	title(WINDOW *, char *, int);
void	resize(void);
void	settimer(int);
void	usage(void);

void
catchsig(int sig)
{
	switch (sig) {
	case SIGWINCH:
		resize_flag = 1;
		/* FALLTHROUGH */
	case SIGALRM:
		break;
	case SIGINT:
	case SIGTERM:
	case SIGHUP:
	default:
		die_flag = 1;
		break;
	}
}

int
main(int argc, char **argv)
{
	WINDOW	*titlew = NULL;
	WINDOW	*outw = stdscr;
	struct	sigaction sa;
	char	buf[_POSIX_MAX_INPUT];
	char	cmd[_POSIX_MAX_INPUT + 5];
	char	out[_POSIX_MAX_INPUT];
	int	hold_curs;
	int	ret = EX_SOFTWARE;
	int	delay = DELAY;
	int	ch;

	while ((ch = getopt(argc, argv, "+hn:t")) != -1)
		switch (ch) {
		case 'n':
			delay = atoi(optarg);
			if (delay < 1)
				usage();
				/* NOTREACHED */
			break;
		case 't':
			title_flag = 0;
			break;
		case 'h':
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}

	argc -= optind;
	argv += optind;

	memset(buf, 0, sizeof(buf));

	if (readargs(argv, buf, sizeof(buf)) && readcmd(buf, sizeof(buf)))
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

	if (title_flag) {
		titlew = newwin(1, 0, 0, 0);
		wattron(titlew, A_BOLD);
		outw = newwin(LINES - 2, 0, 2, 0);
	}

	settimer(delay);

	do {
		if (resize_flag) {
			resize();
			if (title_flag) {
				wresize(titlew, 1, COLS); 
				clearok(titlew, TRUE);
				wresize(outw, LINES - 2, COLS); 
			}
			clearok(outw, TRUE);
			resize_flag = 0;
		}

		if (title_flag)
			title(titlew, buf, delay);

		ret = display(outw, cmd, out, sizeof(out));
		doupdate();

		sigsuspend(&sa.sa_mask);
	} while (!die_flag);

	if (title_flag) {
		delwin(outw);
		delwin(titlew);
	}

	curs_set(hold_curs);
	endwin();

	if (ret)
		(void)fprintf(stderr, "%s: %s", __progname, out);

	return ret;
}

int
readargs(char **argv, char *buf, size_t sz)
{
	if (!*argv)
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
	char	*s;

	if (!isatty(fileno(stdin)))
		return -1;

	(void)fprintf(stderr, "command: ");

	if (fgets(buf, sz - 1, stdin) && (s = strchr(buf, '\n')))
		*s = '\0';

	return 0;
}

int
display(WINDOW *outw, char *cmd, char *out, size_t sz)
{
	FILE	*pipe;
	int	ret = EX_OSERR;
	int	y, x;

	pipe = popen(cmd, "r");

	if (pipe) {
		wmove(outw, 0, 0);
		getmaxyx(outw, y, x);

		while (fgets(out, sz - 1, pipe) && y--)
			waddnstr(outw, out, x);

		ret = pclose(pipe);
		if (ret)
			die_flag = 1;

		wclrtobot(outw);
		wnoutrefresh(outw);
	}

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

void
usage(void)
{
	(void)fprintf(stderr, "usage: %s [-htv] [-n time] [command]\n",
	    __progname);
	exit(EX_USAGE);
}
