/* watch - execute program periodicaly, showing output fullscreen */
/*
 * Copyright (c) 2003 demon <demon@vhost.dymdns.org>
 * All rights reserved.
 *   
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of Demon nor the names of his contributors may
 *   be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
