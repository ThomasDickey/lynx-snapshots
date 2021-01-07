#!/bin/sh
# $LynxId: fixtext.sh,v 1.2 2021/01/07 00:21:03 tom Exp $

# xgettext only processes literal strings.  Someone with a poor sense of humor
# decided to ignore strings in preprocessor lines.  So we construct a fake
# ".c" file with the definitions turned into literals.

sed	-e 's/")/");/' \
	-e 's/^#define[ 	]*\([^ 	]*\)[ 	]*gettext/char *\1 = gettext/' \
	-e 's,^#define[ 	]*\([^ 	]*\)[ 	]*\\,/* #define \1 */char *\1 = \\,' \
	"$@"
