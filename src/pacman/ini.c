/*
 *  ini.c
 *
 *  Copyright (c) 2013 Pacman Development Team <pacman-dev@archlinux.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <glob.h>
#include <limits.h>
#include <string.h> /* strdup */

#include <alpm.h>

#include "ini.h"
#include "util.h"

/**
 * @brief INI parser backend.
 *
 * @param file path to the config file
 * @param cb callback for key/value pairs
 * @param data caller defined data to be passed to the callback
 * @param section_name the name of the current section
 * @param depth recursion depth, should initially be 0
 *
 * @return 0 on success, 1 on parsing errors, the callback return value
 * otherwise
 */
static int _parse_ini(const char *file, ini_parser_fn cb, void *data,
		char **section_name, int depth)
{
	FILE *fp = NULL;
	char line[PATH_MAX];
	int linenum = 0;
	int ret = 0;
	const int max_depth = 10;

	if(depth >= max_depth) {
		pm_printf(ALPM_LOG_ERROR,
				_("config parsing exceeded max recursion depth of %d.\n"), max_depth);
		ret = 1;
		goto cleanup;
	}

	pm_printf(ALPM_LOG_DEBUG, "config: attempting to read file %s\n", file);
	fp = fopen(file, "r");
	if(fp == NULL) {
		pm_printf(ALPM_LOG_ERROR, _("config file %s could not be read: %s\n"),
				file, strerror(errno));
		ret = 1;
		goto cleanup;
	}

	while(fgets(line, PATH_MAX, fp)) {
		char *key, *value, *ptr;
		size_t line_len;

		linenum++;

		/* ignore whole line and end of line comments */
		if((ptr = strchr(line, '#'))) {
			*ptr = '\0';
		}

		line_len = strtrim(line);

		if(line_len == 0) {
			continue;
		}

		if(line[0] == '[' && line[line_len - 1] == ']') {
			char *name;
			/* only possibility here is a line == '[]' */
			if(line_len <= 2) {
				pm_printf(ALPM_LOG_ERROR, _("config file %s, line %d: bad section name.\n"),
						file, linenum);
				ret = 1;
				goto cleanup;
			}
			/* new config section, skip the '[' */
			name = strdup(line + 1);
			name[line_len - 2] = '\0';

			ret = cb(file, linenum, name, NULL, NULL, data);
			free(*section_name);
			*section_name = name;

			/* we're at a new section; perform any post-actions for the prior */
			if(ret) {
				goto cleanup;
			}
			continue;
		}

		/* directive */
		/* strsep modifies the 'line' string: 'key \0 value' */
		key = line;
		value = line;
		strsep(&value, "=");
		strtrim(key);
		strtrim(value);

		if(key == NULL) {
			pm_printf(ALPM_LOG_ERROR, _("config file %s, line %d: syntax error in config file- missing key.\n"),
					file, linenum);
			ret = 1;
			goto cleanup;
		}
		/* Include is allowed in both options and repo sections */
		if(strcmp(key, "Include") == 0) {
			glob_t globbuf;
			int globret;
			size_t gindex;

			if(value == NULL) {
				pm_printf(ALPM_LOG_ERROR, _("config file %s, line %d: directive '%s' needs a value\n"),
						file, linenum, key);
				ret = 1;
				goto cleanup;
			}
			/* Ignore include failures... assume non-critical */
			globret = glob(value, GLOB_NOCHECK, NULL, &globbuf);
			switch(globret) {
				case GLOB_NOSPACE:
					pm_printf(ALPM_LOG_DEBUG,
							"config file %s, line %d: include globbing out of space\n",
							file, linenum);
					break;
				case GLOB_ABORTED:
					pm_printf(ALPM_LOG_DEBUG,
							"config file %s, line %d: include globbing read error for %s\n",
							file, linenum, value);
					break;
				case GLOB_NOMATCH:
					pm_printf(ALPM_LOG_DEBUG,
							"config file %s, line %d: no include found for %s\n",
							file, linenum, value);
					break;
				default:
					for(gindex = 0; gindex < globbuf.gl_pathc; gindex++) {
						pm_printf(ALPM_LOG_DEBUG, "config file %s, line %d: including %s\n",
								file, linenum, globbuf.gl_pathv[gindex]);
						_parse_ini(globbuf.gl_pathv[gindex], cb, data,
								section_name, depth + 1);
					}
					break;
			}
			globfree(&globbuf);
			continue;
		}
		if((ret = cb(file, linenum, *section_name, key, value, data)) != 0) {
			goto cleanup;
		}
	}

	if(depth == 0) {
		ret = cb(NULL, 0, NULL, NULL, NULL, data);
	}

cleanup:
	if(fp) {
		fclose(fp);
	}
	if(depth == 0) {
		free(*section_name);
		*section_name = NULL;
	}
	pm_printf(ALPM_LOG_DEBUG, "config: finished parsing %s\n", file);
	return ret;
}

/**
 * @brief Parse a pacman-style INI config file.
 *
 * @param file path to the config file
 * @param cb callback for key/value pairs
 * @param data caller defined data to be passed to the callback
 *
 * @return 0 on success, 1 on parsing errors, the callback return value
 * otherwise
 *
 * @note The callback will be called at the beginning of each section with an
 * empty key and value, for each key/value pair, and when parsing is complete
 * with all arguments except @a data empty.
 *
 * @note The @a key and @a value passed to @ cb will be overwritten between
 * calls.  The section name will remain valid until after @a cb is called to
 * begin a new section.
 *
 * @note Parsing will immediately stop if the callback returns non-zero.
 */
int parse_ini(const char *file, ini_parser_fn cb, void *data)
{
	char *section_name = NULL;
	return _parse_ini(file, cb, data, &section_name, 0);
}

/* vim: set ts=2 sw=2 noet: */