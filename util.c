// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * src/util.c
 *
 * Data conversion and handling utilities
 *
 * Copyright (C) 2018-2021 SCANOSS.COM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "util.h"
#include "limits.h"
#include "debug.h"
//#include "license.h"
#include <scanoss/ldb.h>
#include "crc32c.h"

/* Case insensitive string start comparison,
	returns true if a starts with b or viceversa */
bool stristart(char *a, char *b)
{
	if (!*a || !*b) return false;
	while (*a && *b) if (tolower(*a++) != tolower(*b++)) return false;
	return true;
}

/* Reverse an uint32 number  */
void uint32_reverse(uint8_t *data)
{
	uint8_t tmp = data[0];
	data[0] = data[3];
	data[3] = tmp;
	tmp = data[1];
	data[1] = data[2];
	data[2] = tmp;
}

/* Compares two MD5 checksums */
bool md5cmp(uint8_t *md51, uint8_t *md52)
{
	for (int i = 0; i < 16; i++)
		if (md51[i] != md52[i])
			return false;
	return true;
}

/* Trim str */
void trim(char *str)
{
    int i = 0;

    /* Left trim */
    int len = strlen(str);
    for (i = 0; i < len; i++) if (!isspace(str[i])) break;
    if (i) memmove(str, str + i, strlen(str + i) + 1);

    /* Right trim */
    len = strlen(str);
    for (i = len - 1; i >= 0 ; i--) if (!isspace(str[i])) break;
    str[i + 1] = 0;
}

/* Trim string at first non-printable char */
void printable_only(char *text)
{
	for (int i = 0; i < strlen(text); i++)
		if (text[i] < '"' || text[i] > 'z') text[i] = 0;
}

/* Returns the pair md5 of "component/vendor" */
void vendor_component_md5(char *component, char *vendor, uint8_t *out)
{
	char pair[1024] = "\0";
	if (strlen(component) + strlen(vendor) + 2 >= 1024) return;

	/* Calculate pair_md5 */
	sprintf(pair, "%s/%s", component, vendor);
	for (int i = 0; i < strlen(pair); i++) pair[i] = tolower(pair[i]);
	MD5((uint8_t *)pair, strlen(pair), out);

	/* Log pair_md5 */
	char hex[MD5_LEN * 2 + 1] = "\0";
	ldb_bin_to_hex(out, MD5_LEN, hex);
	scanlog("vendor/component: %s = %s\n", pair, hex);
}

/* Returns the pair md5 of "component/vendor" */
void vendor_component_md5(char *component, char *vendor, uint8_t *out)
{
	char pair[1024] = "\0";
	if (strlen(component) + strlen(vendor) + 2 >= 1024) return;

	/* Calculate pair_md5 */
	sprintf(pair, "%s/%s", component, vendor);
	for (int i = 0; i < strlen(pair); i++) pair[i] = tolower(pair[i]);
	MD5((uint8_t *)pair, strlen(pair), out);

	/* Log pair_md5 */
	char hex[MD5_LEN * 2 + 1] = "\0";
	ldb_bin_to_hex(out, MD5_LEN, hex);
	scanlog("vendor/component: %s = %s\n", pair, hex);
}

/* Removes chr from str */
void remove_char(char *str, char chr)
{
	char *s = str;
	while (*s)
	{
		if (*s == chr) memmove(s, s + 1, strlen(s + 1) + 1);
		else s++;
	}
}

/* Cleans str from unprintable characters or quotes */
void string_clean(char *str)
{
  char *s = str;
  while (*s)
  {
    if (*s < ' ' || *s == '"') *s = ' ';
    else s++;
  }
}

/* Returns the current date stamp */
char *datestamp()
{
	time_t timestamp;
	struct tm *times;
	time(&timestamp);
	times = localtime(&timestamp);
	char *stamp = malloc(MAX_ARGLN);
	strftime(stamp, MAX_ARGLN, "%FT%T%z", times);
	return stamp;
}

/* Prints a "created" JSON element with the current datestamp */
void print_datestamp()
{
	char *stamp = datestamp();
	printf("%s", stamp);
	free(stamp);
}

/* Returns a string with a hex representation of md5 */
char *md5_hex(uint8_t *md5)
{
	char *out =  calloc(2 * MD5_LEN + 1, 1);
	for (int i = 0; i < MD5_LEN; i++) sprintf(out + strlen(out), "%02x", md5[i]);
	return out;
}

/* Returns the CRC32C for a string */
uint32_t string_crc32c(char *str)
{
	return calc_crc32c (str, strlen(str));
}

/* Check if a crc is found in the list (add it if not) */
bool add_CRC(uint32_t *list, uint32_t crc)
{
	for (int i = 0; i < CRC_LIST_LEN; i++)
	{
		if (list[i] == 0)
		{
			list [i] = crc;
			return false;
		}
		if (list[i] == crc) return true;
	}
	return false;
}

/* Check if a component is found in component_list (add it if not) */
// bool add_component(match_data *match)
// {
// 	char *purl = match->purl;
// 	char *vendor = match->vendor;
// 	char *component = match->component;
// 	char *version = match->version;
// 	char *latest_version = match->latest_version;

// 	/* Init component list */
// 	for (int i = 0; i < CRC_LIST_LEN; i++)
// 	{
// 		if (!strcmp(component_list[i].purl, purl)) return true;
// 		if (!*component_list[i].purl)
// 		{
// 			get_license(*match, component_list[i].license);
// 			strcpy(component_list[i].vendor, vendor);
// 			strcpy(component_list[i].component, component);
// 			strcpy(component_list[i].version, version);
// 			strcpy(component_list[i].latest_version, latest_version);
// 			strcpy(component_list[i].purl, purl);
// 			return false;
// 		}
// 	}
// 	return false;
// }
