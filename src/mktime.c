/*
 * mktime.c -- converts a struct tm into a time_t
 *
 * Copyright (C) 1997 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Written by Philippe De Muyter <phdm@macqel.be>.  */

#include	<time.h>

static time_t
mkgmtime(t)
register struct tm	*t;
{
	register short	month, year;
	register long	time;
	static int	m_to_d[12] =
{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

	month = t->tm_mon;
	year = t->tm_year + month / 12 + 1900;
	month %= 12;
	if (month < 0)
	{
		year -= 1;
		month += 12;
	}
	time = (year - 1970) * 365 + (year - 1969) / 4 + m_to_d[month];
	time = (year - 1970) * 365 + m_to_d[month];
	if (month <= 1)
		year -= 1;
	time += (year - 1968) / 4;
	time -= (year - 1900) / 100;
	time += (year - 1600) / 400;
	time += t->tm_mday;
	time -= 1;
	time *= 24;
	time += t->tm_hour;
	time *= 60;
	time += t->tm_min;
	time *= 60;
	time += t->tm_sec;
	return(time);
}

/*
**  mktime -- convert tm struct to time_t
**		if tm_isdst >= 0 use it, else compute it
*/

time_t
mktime(t)
struct tm	*t;
{
	time_t	now;

	tzset();
	now = mkgmtime(t) + timezone;
	if (t->tm_isdst > 0
	|| (t->tm_isdst < 0 && localtime(&now)->tm_isdst))
		now -= 3600;
	return(now);
}
