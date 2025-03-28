/***************************************/
/* mkwpd main source file              */
/* (c) 2025 Nathan Gill                */
/* https://github.com/OldUser101/mkpwd */
/***************************************/

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mkpwd.h"

bool DEBUG = false;
int DEF_PASSWORDLEN = 18;
int DEF_PASSWORDN = 1;
bool DEF_PASSWORDCHARS[4] = { true, true, true, false };

char* CHARSET = 0;

const char* CHARSET_LOWER = "abcdefghijklmnopqrstuvwxyz";
const char* CHARSET_UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char* CHARSET_NUMERIC = "0123456789";
const char* CHARSET_SPECIAL = "!\"#$%'()*+,-./:;<=>?@[]^_{|}~";

unsigned int FAULTCODE = 0;

void ERRORROUTINE(const char* msg, int errorCode) {
	printf("%s. see \"man mkpwd\" for more information.\n", msg);
	FAULTCODE = errorCode;
	exit(FAULTCODE);
}

int getTotalCharSetLen(bool* chars) {
	int total = 0;
	total += chars[0] ? strlen(CHARSET_LOWER) : 0;
	total += chars[1] ? strlen(CHARSET_UPPER) : 0;
	total += chars[2] ? strlen(CHARSET_NUMERIC) : 0;
	total += chars[3] ? strlen(CHARSET_SPECIAL) : 0;
	return total;
}

void compileCharSet(bool* chars) {
	int charsetLen = getTotalCharSetLen(chars) + 1;

	if (charsetLen <= 1) {
		ERRORROUTINE("argument error", 1);
		return;
	}

	CHARSET = (char*)malloc(charsetLen);
	if (!CHARSET) {
		ERRORROUTINE("memory allocation error", 2);
		return;
	}

	char* cptr = CHARSET;

	if (chars[0]) {
		memcpy(cptr, CHARSET_LOWER, strlen(CHARSET_LOWER));
		cptr += strlen(CHARSET_LOWER);
	}
	if (chars[1]) {
		memcpy(cptr, CHARSET_UPPER, strlen(CHARSET_UPPER));
		cptr += strlen(CHARSET_UPPER);
	}
	if (chars[2]) {
		memcpy(cptr, CHARSET_NUMERIC, strlen(CHARSET_NUMERIC));
		cptr += strlen(CHARSET_NUMERIC);
	}
	if (chars[3]) {
		memcpy(cptr, CHARSET_SPECIAL, strlen(CHARSET_SPECIAL));
		cptr += strlen(CHARSET_SPECIAL);
	}

	(*cptr) = '\0';
}

char getNextChar(bool* chars) {
	int charsetLen = getTotalCharSetLen(chars);
	int sel = rand() % charsetLen;
	return CHARSET[sel];
}

char* generatePassword(PasswordOptions* options) {
	char* password = (char*)calloc(1, options->length + 1);
	for (int i = 0; i < options->length; i++) {
		char nextChar = getNextChar(&(options->chars[0]));
		password[i] = nextChar;
	}
	return password;
}

unsigned int getRandomSeed() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec ^ ts.tv_nsec;
}

char* stripPrefixes(char* s) {
	while ((*s == ' ' || *s == '-' || *s == '/') && *s != '\0') {
		s++;
	}
	return s;
}

void toLower(char* s) {
	while (*s != '\0') {
		if (*s >= 65 && *s <= 90) {
			(*s) += 32;
		}
		s++;
	}
}

int getArgNumber(int argc, char* argv[], int* i) {
	if ((*i) + 1 < argc) {
		int n = atoi(stripPrefixes(argv[(*i) + 1]));
		(*i)++;		// Skip the next argument

		if (DEBUG) {
			printf("%i: %dn\n", *(i) + 1, n);
		}

		return n;
	}
	ERRORROUTINE("argument error", 1);
	return 0;
}

void showHelp() {
	printf("mkpwd - generate secure passwords\n");
	printf("version 1.0\n\n");
	printf("USAGE\n");
	printf("	mkpwd [OPTIONS]\n\n");
	printf("OPTIONS\n");
	printf("	-n XX, --number XX\n");
	printf("		Specify the number of passwords to generate (default: 1, or environment variable).\n\n");
	printf("	-l XX, --length XX\n");
	printf("		Set the length of each password (default: 18, or environment variable).\n\n");
	printf("	-h, --help\n");
	printf("		Display this help message and exit.\n\n");
	printf("	+lusn");
	printf("		Specify character sets to include:\n");
	printf("		  l  Lowercase letters\n");
	printf("		  u  Uppercase letters\n");
	printf("		  s  Symbols\n");
	printf("		  n  Numbers\n");
	printf("		Default: +lun if not specified (or environment variable).\n\n");
	printf("AUTHOR\n");
	printf("	Written by Nathan Gill.\n");
	printf("	Licensed under the BSD 2-Clause \"Simplified\" License.\n");
	printf("	See \"copyright\" for more information.\n\n");
	printf("Run \"man mkpwd\" for examples, environment variables, and extended help.\n");
}

bool parseCharArguments(bool* chars, char* args) {
	char* stripped = stripPrefixes(args);
	bool chars_set = false;
	for (int j = 0; j < strlen(stripped); j++) {
		switch (stripped[j]) {
			case 'l':
				chars[0] = true;
				chars_set = true;
				break;
			case 'u':
				chars[1] = true;
				chars_set = true;
				break;
			case 'n':
				chars[2] = true;
				chars_set = true;
				break;
			case 's':
				chars[3] = true;
				chars_set = true;
				break;
			case '+':
				break;
			default:
				ERRORROUTINE("argument error", 1);
				break;
		}
	}
	return chars_set;
}

void fillFromArgs(PasswordOptions* options, int argc, char* argv[]) {
	bool n_set = false;
	bool length_set = false;
	bool chars_set = false;

	bool custom_chars[4] = { false, false, false, false };

	for (int i = 1; i < argc; i++) {
		toLower(argv[i]);
		char* stripped = stripPrefixes(argv[i]);

		if (DEBUG) {
			printf("%i: \"%s\" -> \"%s\"\n", i, argv[i], stripped);
		}

		if (!strcmp(stripped, "n") || !strcmp(stripped, "number")) {
			options->n = getArgNumber(argc, argv, &i);
			n_set = true;
		}
		else if (!strcmp(stripped, "l") || !strcmp(stripped, "length")) {
			options->length = getArgNumber(argc, argv, &i);
			length_set = true;
		}
		else if (!strcmp(stripped, "h") || !strcmp(stripped, "help")) {
			showHelp();
			exit(0);
		}
		else if (stripped[0] == '+') {
                        chars_set = parseCharArguments(&(custom_chars[0]), stripped);
		}
	}

	if (!n_set) {
		options->n = DEF_PASSWORDN;
	}

	if (!length_set) {
		options->length = DEF_PASSWORDLEN;
	}

	if (!chars_set) {
		memcpy(&(options->chars[0]), DEF_PASSWORDCHARS, sizeof(bool) * 4);
	}
	else {
		memcpy(&(options->chars[0]), custom_chars, sizeof(bool) * 4);
	}
}

void trim(char* s) {
	while (isspace((unsigned char)*s)) {
		s++;
	}

	char* end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char)*end)) {
		end--;
	}

	*(end + 1) = '\0';
}

void getConf(char* p) {
	const char* home = getenv("HOME");
	char expanded[1024];
	snprintf(expanded, sizeof(expanded), "%s/.mkpwd", home);
	strncpy(p, expanded, 1023);
	p[1023] = '\0';
}

void parseConfig() {
	char path[1024];
	getConf(path);

	FILE* f = fopen(path, "r");
	if (!f) {
		return;
	}

	char line[256];
	while (fgets(line, sizeof(line), f)) {
		if (line[0] == '#' || isspace(line[0])) {
			continue;

		}

		line[strcspn(line, "\n")] = '\0';

		char* eqs = strchr(line, '=');
		if (eqs) {
			*eqs = '\0';
			eqs++;

			trim(line);
			trim(eqs);

			if (DEBUG) {
				printf("Key: '%s', Value: '%s'\n", line, eqs);
			}

			if (!strcmp("DEF_PASSWORDN", line)) {
                		if (strcmp(eqs, "") != 0) {
                        		DEF_PASSWORDN = atoi(eqs);
                		}
			}
			else if (!strcmp("DEF_PASSWORDLEN", line)) {
                		if (strcmp(eqs, "") != 0) {
                        		DEF_PASSWORDLEN = atoi(eqs);
                		}
			}
			else if (!strcmp("DEF_PASSWORDCHARS", line)) {
                		if (strcmp(eqs, "") != 0) {
                        		DEF_PASSWORDCHARS[0] = false;
                        		DEF_PASSWORDCHARS[1] = false;
                        		DEF_PASSWORDCHARS[2] = false;
                        		DEF_PASSWORDCHARS[3] = false;
                        		parseCharArguments(&(DEF_PASSWORDCHARS[0]), eqs);
                		}
			}
			else {
				printf("unknown configuration: '%s'='%s'. see 'man mkpwd'.\n", line, eqs);
			}
		}
	}

	fclose(f);
}

bool checkConfig() {
	char path[1024];
	getConf(path);

	FILE* f = fopen(path, "r");
	if (!f) {
		if (DEBUG) {
			printf("Configuration file does not exist\n");
		}
		return false;
	}
	fclose(f);

	if (DEBUG) {
		printf("Configuration file exists\n");
	}
	return true;
}

int main(int argc, char* argv[]) {
	const char* debug_env = getenv("MKPWD_DEBUG");
	if (debug_env != NULL && !strcmp(debug_env, "TRUE")) {
		DEBUG = true;
	}

	if (checkConfig()) {
		parseConfig();
	}

	srand(getRandomSeed());

	PasswordOptions* options = (PasswordOptions*)calloc(1, sizeof(PasswordOptions));
	fillFromArgs(options, argc, argv);

#ifdef DEBUG
	printf("Number: %d\n", options->n);
#endif

	compileCharSet(&(options->chars[0]));

	for (int i = 0; i < options->n; i++) {
		char* password = generatePassword(options);
		printf("%s\n", password);

		free(password);
	}

	free(options);

	return 0;
}
