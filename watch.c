/* From: Tony Rems <rembo@unisoft.com>
   Newsgroups: comp.sources.unix
   Subject: v24i084:  Repeatedly execute a program under curses(3)
   Date: 26 Mar 91 21:41:30 GMT

   Slighty modified, and corrected, François Pinard, 91-04.
*/

#include <curses.h>
#include <stdio.h>
#include <signal.h>
#include <sys/fcntl.h>

int die_flag;
void die ();

extern FILE *popen ();
extern int pclose ();
extern long time ();
extern char *ctime ();

/*-----------------------------------------.
| Decode parameters and launch execution.  |
`-----------------------------------------*/

char *program_name;

void
main (argc, argv)
     int argc;
char *argv[];
{
  int hor = 1, ver = 0;
  FILE *piper;
  char buf[180];
  char cmd[128];
  int count = 1;
  long timer;
  int nsecs = 2;

  program_name = argv[0];
  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s COMMAND [-n SLEEP] [ARG ...]\n", argv[0]);
      exit (1);
    }

  /* If -n is specified, convert the next argument to the numver for the
     number of seconds.  */

  if (strcmp (argv[1], "-n") == 0)
    {
      nsecs = atoi (argv[2]);
      count = 3;
      if (nsecs == 0 || argc < 4)
	{
	  fprintf (stderr, "Usage: %s COMMAND [-n SLEEP] [ARG ...]\n", argv[0]);
	  exit (1);
	}
    }

  /* Build command string to give to popen.  */
  
  bzero (cmd, sizeof (cmd));
  strcpy (cmd, argv[count]);
  while (++count < argc)
    {
      strcat (cmd, " ");
      strcat (cmd, argv[count]);
    }

  /* Catch keyboard interrupts so we can put tty back in a sane state.  */
  
  signal (SIGINT, die);
  signal (SIGTERM, die);
  signal (SIGHUP, die);

  /* Set up tty for curses use.  */

  initscr ();
  nonl ();
  noecho ();
  crmode ();

  die_flag = 0;
  while (!die_flag)
    {

      /* Put up time interval and current time.  */

      hor = 1;
      move (hor, ver);
      time (&timer);
      printw ("Every %d seconds\t\t%s\t\t%s", nsecs, cmd, ctime (&timer));
      hor = 3;

      /* Open pipe to command. */
      
      if ((piper = popen (cmd, "r")) == (FILE *)NULL)
	{
	  perror ("popen");
	  exit (2);
	}

      /* Read in output from the command and make sure that it will fit on 1
	 screen.  */ 

      while ((fgets(buf, sizeof (buf), piper) != NULL) && hor < LINES)
	{
	  buf[COLS-1] = '\0';
	  mvaddstr (hor, ver, buf);
	  clrtoeol ();
	  hor++;
	}
      pclose (piper);

      if (hor < LINES)
	{
	  move (hor, ver);
	  clrtobot ();
	}
      refresh ();
      sleep (nsecs);
    }
  nocrmode ();
  echo ();
  nl ();
  endwin ();
  exit (0);
}

/*------------------------------------.
| Stop the program on any interrupt.  |
`------------------------------------*/

void
die ()
{
  die_flag = 1;
}
