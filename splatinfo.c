#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include "types.h"

static struct termios term;
static struct winsize winsize;
static Rotation rotation_data;

void
print_anarchy_rotation(int width, int height) {
	/* Save cursor position */
	fwrite("\e[s", 3, sizeof(char), stdout);

	/* Series */
	printf("\e[B\e[3G\e[96mSeries\e[37m:\e[15G\e[93m%s\e[0m", rotation_data.series_mode);
	printf("\e[B\e[7G%s\e[B\e[7G%s", stage_str[rotation_data.series_stage[0]], stage_str[rotation_data.series_stage[1]]);

	/* Open */
	printf("\e[2B\e[3G\e[96mOpen\e[37m:\e[15G\e[93m%s\e[0m", rotation_data.open_mode);
	printf("\e[B\e[7G%s\e[B\e[7G%s", stage_str[rotation_data.open_stage[0]], stage_str[rotation_data.open_stage[1]]);

	/* Load cursor position */
	fwrite("\e[u", 3, sizeof(char), stdout);
}

void
print_turf_x_rotation(int width, int height) {
	/* Check width and height are greater than minimum */
	/* TODO */

	/* Calculate x coordinate of second column */
	int mid = width / 2 + 1;

	/* Save cursor position */
	fwrite("\e[s", 3, sizeof(char), stdout);

	/* Turf */
	printf("\e[B\e[%dG\e[92mRegular\e[37m:\e[%dG\e[93m%s\e[0m", mid, mid + 12, rotation_data.series_mode);
	printf("\e[B\e[%dG%s\e[B\e[%dG%s", mid + 4, stage_str[rotation_data.series_stage[0]], mid + 4, stage_str[rotation_data.series_stage[1]]);

	/* X battle */
	printf("\e[2B\e[%dG\e[94mX Battle\e[37m:\e[%dG\e[93m%s\e[0m", mid, mid + 12, rotation_data.open_mode);
	printf("\e[B\e[%dG%s\e[B\e[%dG%s", mid + 4, stage_str[rotation_data.open_stage[0]], mid + 4, stage_str[rotation_data.open_stage[1]]);

	/* Load cursor position */
	fwrite("\e[u", 3, sizeof(char), stdout);
}

void
print_rotation_box(int width, int height) {
	/* Check width and height are greater than minimum */
	/* TODO */

	/* Create enough space for the whole box */
	for (int i = 0; i < height; i++)
		putc('\n', stdout);
	printf("\e[%dA", height);

	/* Save cursor position
	 * (top left of rotation box) */
	fwrite("\e[s", 3, sizeof(char), stdout);

	/* Print title and top of rotation box */
	printf("\e[91m┌─┐ Current\e[37m: \e[91m┌");
	for (int i = 0; i < width - 15; i++)
		printf("─");
	printf("┐");

	/* Print sides of rotation box */
	for (int i = 0; i < height - 2; i++) {
		printf("│\e[%dG", width);
		printf("│\e[1G");
		printf("\e[B\e[G");
	}

	/* Print bottom of rotation box */
	printf("└");
	for (int i = 0; i < width - 2; i++)
		printf("─");
	printf("┘");

	/* Load cursor position */
	fwrite("\e[u", 3, sizeof(char), stdout);
}

int
main(int argc, char *argv[]) {
	FILE *fcurloutput = fopen("/home/basil/splat-info/output-info.json", "w");

	/* Initialise stuff */
	tcgetattr(STDIN_FILENO, &term);
	struct termios tmp = term;
	tmp.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, 0, &tmp);

	printf("\e[?25l"); // Hide cursor
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);

	/* Start child to call curl */
	int oldstdin = dup(0);
	int fds[2];
	pipe(fds);

	if (fork() == 0) {
		close(fds[0]);
		dup2(fds[1], 1);
		execvp("curl", (char *[]){ "curl", "--silent", "https://splatoon.oatmealdome.me/api/v1/three/versus/phases?count=12", NULL });
		exit(1);
	}
	close(fds[1]);
	dup2(fds[0], 0);

	size_t len = 0;
	char *curl_output = 0;
	getdelim(&curl_output, &len, EOF, stdin);
	clearerr(stdin);
	dup2(oldstdin, 0);

	/* Parse rotation data */
	cJSON *curljson = cJSON_Parse(curl_output);
	cJSON *normal = cJSON_GetObjectItem(curljson, "normal")->child;
	for (int i = 0; i < 1; i++) {
		cJSON *regular = cJSON_GetObjectItem(normal, "Regular");
		cJSON *series = cJSON_GetObjectItem(normal, "Bankara");
		cJSON *open = cJSON_GetObjectItem(normal, "BankaraOpen");
		cJSON *x = cJSON_GetObjectItem(normal, "X");

		/* Regular battle */
		rotation_data.regular_mode = cJSON_GetObjectItem(regular, "rule")->valuestring;
		rotation_data.regular_stage[0] = cJSON_GetObjectItem(regular, "stages")->child->valueint;
		rotation_data.regular_stage[1] = cJSON_GetObjectItem(regular, "stages")->child->next->valueint;

		/* Anarchy series */
		rotation_data.series_mode = cJSON_GetObjectItem(series, "rule")->valuestring;
		rotation_data.series_stage[0] = cJSON_GetObjectItem(series, "stages")->child->valueint;
		rotation_data.series_stage[1] = cJSON_GetObjectItem(series, "stages")->child->next->valueint;

		/* Anarchy open */
		rotation_data.open_mode = cJSON_GetObjectItem(open, "rule")->valuestring;
		rotation_data.open_stage[0] = cJSON_GetObjectItem(open, "stages")->child->valueint;
		rotation_data.open_stage[1] = cJSON_GetObjectItem(open, "stages")->child->next->valueint;

		/* X battle */
		rotation_data.x_mode = cJSON_GetObjectItem(x, "rule")->valuestring;
		rotation_data.x_stage[0] = cJSON_GetObjectItem(x, "stages")->child->valueint;
		rotation_data.x_stage[1] = cJSON_GetObjectItem(x, "stages")->child->next->valueint;

		normal = normal->next;
	}

	print_rotation_box(winsize.ws_col, 10);
	print_anarchy_rotation(winsize.ws_col, 10);
	print_turf_x_rotation(winsize.ws_col, 10);
	fflush(stdout);

	/* Main loop */
	static int run = 1;
	while (run) {
		char c = getc(stdin);
		switch (c) {
			case 'q':
				run = 0;
				break;
			default:
				printf("%c\n", c);
				break;
		}
	}

	printf("\e[?25h\n"); // Show cursor
	tcsetattr(STDIN_FILENO, 0, &term);

	return 0;
}
