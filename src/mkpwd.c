#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "mkpwd.h"

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
	printf("%s\n", msg);
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
	CHARSET = (char*)malloc(charsetLen);
	if (!CHARSET) {
		ERRORROUTINE("memory allocation error", 1);
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
			*s += 32;
		}
	}
}

int getArgNumber(int argc, char* argv[], int* i) {
	if ((*i) + 1 < argc) {
		int n = atoi(stripPrefixes(argv[(*i) + 1]));
		(*i)++;		// Skip the next argument

#ifdef DEBUG
		printf("%i: %dn\n", i + 1, n);
#endif

		return n;
	}
	ERRORROUTINE("argument error", 1);
	return 0;
}

void fillFromArgs(PasswordOptions* options, int argc, char* argv[]) {
	bool n_set = false;
	bool length_set = false;
	bool chars_set = false;

	bool custom_chars[4] = { false, false, false, false };

	for (int i = 1; i < argc; i++) {
		//toLower(argv[i]);
		char* stripped = stripPrefixes(argv[i]);

#ifdef DEBUG
		printf("%i: \"%s\" -> \"%s\"\n", i, argv[i], stripped);
#endif

		if (!strcmp(stripped, "n") || !strcmp(stripped, "number")) {
			options->n = getArgNumber(argc, argv, &i);
			n_set = true;
		}
		else if (!strcmp(stripped, "l") || !strcmp(stripped, "length")) {
			options->length = getArgNumber(argc, argv, &i);
			length_set = true;
		}
		else if (stripped[0] == '+') {
			for (int j = 1; j < strlen(stripped); j++) {
				switch (stripped[j]) {
					case 'l':
						custom_chars[0] = true;
						chars_set = true;
						break;
					case 'u':
						custom_chars[1] = true;
						chars_set = true;
						break;
					case 'n':
						custom_chars[2] = true;
						chars_set = true;
						break;
					case 's':
						custom_chars[3] = true;
						chars_set = true;
						break;
					default:
						ERRORROUTINE("argument error", 1);
						break;
				}
			}
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

void searchEnvVars() {
	char* envPasswordN = getenv("MKPWD_DEF_PASSWORDN");
	if (envPasswordN) {
		char* stripped = stripPrefixes(envPasswordN);
		if (strcmp(stripped, "") != 0) {
			DEF_PASSWORDN = atoi(stripped);
		}
	}

	char* envPasswordLen = getenv("MKPWD_DEF_PASSWORDLEN");
	if (envPasswordLen) {
		char* stripped = stripPrefixes(envPasswordLen);
		if (strcmp(stripped, "") != 0) {
			DEF_PASSWORDLEN = atoi(stripped);
		}
	}

	char* envPasswordChars = getenv("MKPWD_DEF_PASSWORDCHARS");
	if (envPasswordChars) {
		char* stripped = stripPrefixes(envPasswordChars);
		if (strcmp(stripped, "") != 0 && strlen(stripped) == 4) {
			DEF_PASSWORDCHARS[0] = (stripped[0] == '1') ? true : false;
			DEF_PASSWORDCHARS[1] = (stripped[1] == '1') ? true : false;
			DEF_PASSWORDCHARS[2] = (stripped[2] == '1') ? true : false;
			DEF_PASSWORDCHARS[3] = (stripped[3] == '1') ? true : false;
		}
	}
}

int main(int argc, char* argv[]) {
	searchEnvVars();

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
