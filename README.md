WATCH(1) - General Commands Manual

# NAME

**watch** - execute program periodically, showing output fullscreen

# SYNOPSIS

**watch**
\[**-tv**]
\[**-n**&nbsp;*time*]
\[*command*]

# DESCRIPTION

The
**watch**
utility runs a
*command*
repeatedly and displays its output fullscreen.
This allow you to watch an output of the
*command*
as it changes.
If the
*command*
is absent,
**watch**
will prompt for it.

The options are as follows:

**-n** *time*

> Set delay between screen updates in seconds.
> The default value is 2 seconds.

**-t**

> Disable output of the title bar.

**-h**

> Display usage and terminate.

**Watch**
will terminate with the keyboard interrupt
('^C')

# EXAMPLE

**watch**
\-n 1 ps

# HISTORY

The original
**watch**
was written by Tony Rems &lt;rembo@unisoft.com&gt; in 1991,
with mods and corrections by Francois Pinard.
It was rewritten completly from scratch by Dimitri Sokolyuk
&lt;demon@dim13.org&gt; in 2003.

OpenBSD 6.5 - December 25, 2004
