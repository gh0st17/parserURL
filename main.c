#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"
#include <curl.h>

#define isHTTP strstr(_url, "http://") != NULL
#define isHTTPS strstr(_url, "https://") != NULL
#define is2Slash strstr(_url, "//") != NULL
#define isInsecure strstr(argv[3], "--insecure") != NULL

void run(char * url, char * filename, long insecure);
void parse(char * url, char * filename, char * buffer);
size_t getDist(char * p1, char * p2);

int main(int argc, char * argv[]) {
	if (argc == 3)
		run(argv[1], argv[2], 1);
	else if (argc == 4 && isInsecure)
		run(argv[1], argv[2], 0);
	else {
		printf_s("USAGE: parserURL <URL> <outputFileName> [--insecure]\n");
		printf_s("<outputFileName> - Name of output file without extension\n");
		printf_s("--insecure       - Disable SSL\n");
		printf_s("Press any key to exit...\n");
	}
	_getch();
	return 0;
}

void run(char * url, char * filename, long insecure) {
	CURL * curl;
	CURLcode res;
	curl = curl_easy_init();
	char * html = NULL;
	struct string s;
	if (curl) {
		init_string(&s);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, insecure);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, insecure);
		curl_easy_setopt(curl, CURLOPT_USERAGENT,
			"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.6) Gecko/20070725 Firefox/2.0.0.6");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (!res) {
			html = s.ptr;
			parse(url, filename, html);
			free(html);
		} else if (res == 60) {
			printf_s("Try with '--insecure'\n");
			printf_s("%s\n", curl_easy_strerror(res));
		}
		else
			printf_s("%s\n", curl_easy_strerror(res));
	}
	printf_s("Press any key to exit...\n");
}

void parse(char * url, char * filename, char * buffer) {
	char * begin = buffer;
	char * end = NULL;
	int total = 0;
	FILE *fp;
	strcat(filename, ".txt");
	fopen_s(&fp, filename, "w");
	if (fp == NULL) {
		printf_s("Can't create output file!\n");
		exit(1);
	}
	while (1) {
		begin = strstr(begin, "<a href=\"");
		if (begin == NULL) break;
		begin = begin + 9;
		end = strstr(begin, "\"");
		size_t URILength = getDist(begin, end);
		if (end != NULL) {
			char * _url = (char*)malloc(URILength + 1);

			memmove_s(_url, URILength + 1, begin, URILength);
			_url[URILength] = '\0';
			fprintf_s(fp, "%4d. %s%s%s\n", total + 1,
				(isHTTP || isHTTPS || is2Slash ? "" : url),
				(strstr(_url, "#") != NULL ? "/" : ""), _url);

			if (_url) free(_url);
		}
		total++;
		begin = end;
	}
	fclose(fp);
	if (total)
		printf_s("%d URLs written to file '%s' successful\n", total, filename);
	else printf_s("URLs not found\n");
	return;
}

size_t getDist(char * p1, char * p2) {
	return (size_t)p2 - (size_t)p1;
}