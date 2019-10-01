#define _CRT_SECURE_NO_WARNINGS // For strncpy

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
void parse(char * url, char * filename, char * buffer); // Метод для парсинга ссылок
size_t getDist(char * p1, char * p2); // Разница между 2 указателями

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
	char * html = NULL; // html
	struct string s;
	if (curl) {
		init_string(&s);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, insecure);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, insecure);
		curl_easy_setopt(curl, CURLOPT_USERAGENT,
			"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.6) Gecko/20070725 Firefox/2.0.0.6");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		html = s.ptr;
		parse(url, filename, html);
		free(html); // Высвобождаем память
	}
	printf("%s\nPress any key to exit...\n", curl_easy_strerror(res));
}

void parse(char * url, char * filename, char * buffer) {
	char * begin = buffer; // Тут будет указатель на начало ссылки
	char * end = NULL; // Тут указатель на конец
	int total = 0; // Количество ссылок
	FILE *fp;
	strcat(filename, ".txt");
	fopen_s(&fp, filename, "w"); // Создать файл вывода
	if (fp == NULL) { // Проверить что файл создался
		printf_s("Can't create output file!\n");
		exit(1);
	}
	while (1) {
		begin = strstr(begin, "<a href=\""); // Поиск вхождения
		if (begin == NULL) break; // Условие прерывания цикла
		begin = begin + 9; // Смещаем на длину вхождения
		end = strstr(begin, "\""); // Ищем конец
		size_t URILength = getDist(begin, end); // Узнаем длину ссылки
		if (end != NULL) { // Если конец найден
			char * _url = (char*)malloc(URILength + 1); // Выделить память под ссылку

			strncpy(_url, begin, URILength); // Скопировать URILength
											// символов начиная с begin
											// в url
			_url[URILength] = '\0'; // Добавляем символ конца
			fprintf_s(fp, "%4d. %s%s%s\n", total + 1, // Вывод в файл
				(isHTTP || isHTTPS || is2Slash ? "" : url),
				(strstr(_url, "#") != NULL ? "/" : ""), _url);

			if (_url) free(_url); // Высвобождаем память
		}
		total++; // Инкремент
		begin = end; // Меняем начало на конец
	}
	fclose(fp); // Закрыть файл
	printf_s("%d URLs written to file '%s' successful\n", total, filename);
	return;
}

size_t getDist(char * p1, char * p2) {
	return (size_t)p2 - (size_t)p1;
}