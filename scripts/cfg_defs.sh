#!/bin/sh
# $LynxId: cfg_defs.sh,v 1.2 2021/01/07 00:38:05 tom Exp $
# Translate the lynx_cfg.h and config.cache data into a table, useful for
# display at runtime.

TOP="${1-.}"
OUT=cfg_defs.h

# just in case we want to run this outside the makefile
: "${SHELL:=/bin/sh}"

{
cat <<EOF
#ifndef CFG_DEFS_H
#define CFG_DEFS_H 1

static const struct {
	const char *name;
	const char *value;
} config_cache[] = {
EOF

sed \
	-e '/^#/d'     \
	-e 's/^.[^=]*_cv_//' \
	-e 's/=\${.*=/=/'  \
	-e 's/}$//'          \
	config.cache | $SHELL "$TOP/scripts/cfg_edit.sh"

cat <<EOF
};

static const struct {
	const char *name;
	const char *value;
} config_defines[] = {
EOF
${FGREP-fgrep}	'#define' lynx_cfg.h |
sed	-e 's@	@ @g' \
	-e 's@  @ @g' \
	-e 's@^[ 	]*#define[ 	]*@@' \
	-e 's@[ ]*/\*.*\*/@@' \
	-e 's@[ 	][ 	]*@=@' \
    | $SHELL "$TOP/scripts/cfg_edit.sh"

cat <<EOF
};

#endif /* CFG_DEFS_H */
EOF
} >$OUT
