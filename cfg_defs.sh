#!/bin/sh
# Translate the lynx_cfg.h and config.cache data into a table, useful for
# display at runtime.

cat <<EOF
#ifndef CFG_DEFS_H
#define CFG_DEFS_H 1

static CONST struct {
	CONST char *name;
	CONST char *value;
} config_cache[] = {
EOF
sed	-e '/^#/d' \
	-e 's/"/\\"/g' \
	-e 's/=}$/=""}/' \
	-e "s/'/\"/g" \
	-e 's/^.[^=]*_cv_/	{ "/' \
	-e 's/=${[^=]*="/", "/' \
	-e 's/=${[^=]*=/", "/' \
	-e 's/"}$/}/' \
	-e 's/}$/" },/' \
	config.cache | sort
cat <<EOF
};

static CONST struct {
	CONST char *name;
	CONST char *value;
} config_defines[] = {
EOF
fgrep	'#define' lynx_cfg.h |sort |
sed	-e 's@	@ @g' \
	-e 's@  @ @g' \
	-e 's@[ ]*#define @@' \
	-e 's@[ ]*/\*.*\*/@@' \
	-e 's@"$@@' \
	-e 's@"@@' \
	-e 's@ @", "@' \
	-e 's@^@	{ "@' \
	-e 's@$@" },@'
cat <<EOF
};

#endif /* CFG_DEFS_H */
EOF
