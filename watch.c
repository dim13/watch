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
#include <signal.h>
#include <curses.h>
#define MAXBUF 255

static char copyright[] = "Copyright (C) 2003 demon (demon@vhost.dyndns.org)";
static char version[] = "0.2";
static char terms[] = "This program comes with ABSOLUTELY NO WARANTY.\n\
This is a free software, and you are welcome to redistribute it\n\
under certain conditions. See the file COPYING for deatails.";
extern char *__progname;
time_t tval;
int die_flag;

void usage();
void die();

int main (int argc, char *argv[]) {
    int i;
    int period=5;
    char ch;
    char cmd[MAXBUF];
    char buf[MAXBUF];
    char curtime[30];
    FILE *pipe;
    
    signal(SIGINT, die);
    signal(SIGTERM, die);
    signal(SIGHUP, die);
    
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
        strcpy(cmd, *argv);
	while(*++argv) {
	    strcat(cmd, " ");
	    strcat(cmd, *argv);
	}
    } else {
	if(isatty(fileno(stdin)))
	    fprintf(stderr, "Command: ");
	(void)fgets(cmd, sizeof(cmd), stdin);
	cmd[strlen(cmd) - 1] = '\0';
    }

    if(strlen(cmd)) {
	initscr();
	while(!die_flag) {
	    time(&tval);
	    strcpy(curtime, ctime(&tval));
	    curtime[strlen(curtime) - 6] = '\0';
	    move(0,0);
	    printw("%s\tEvery %ds: %s\n\n", curtime, period, cmd);
	    pipe = popen(cmd, "r");
	    while(fgets(buf, sizeof(buf), pipe))
		printw("%s",buf);
	    refresh();
	    pclose(pipe);
	    sleep(period);
	}
	endwin();
    } else {
	usage();
    }

    exit(0);
}

void usage() {
    (void)fprintf(stderr, "Usage: %s [-s <seconds> | -v ] [command]\n", __progname);
    exit (1);
}

void die() {
    die_flag=1;
}
