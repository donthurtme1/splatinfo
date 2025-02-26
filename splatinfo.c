#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "types.h"

static struct termios term;
static struct winsize winsize;
static Rotation rotation_data[12];

static const MapMode fav_stages[] = {
	/* Zones */
	{ AREA, UMAMI_RUINS },
	{ AREA, FLOUNDER_HEIGHTS },
	{ AREA, MUSEUM_D_ALFONSINO },
	{ AREA, MAHI_MAHI_RESORT },
	{ AREA, INKBLOT_ART_ACADEMY },
	{ AREA, STURGEON_SHIPYARD },
	{ AREA, MAKOMART },
	{ AREA, WAHOO_WORLD },
	{ AREA, CRABLEG_CAPITAL },
	{ AREA, ROBO_ROM_EN },
	{ AREA, MARLIN_AIRPORT },

	/* Clams */
	{ CLAM, UMAMI_RUINS },
	{ CLAM, MUSEUM_D_ALFONSINO },
	{ CLAM, INKBLOT_ART_ACADEMY },
	{ CLAM, STURGEON_SHIPYARD },
	{ CLAM, MAKOMART },
	{ CLAM, MANTA_MARIA },
	{ CLAM, CRABLEG_CAPITAL },
	{ CLAM, SHIPSHAPE_CARGO_CO },
	{ CLAM, ROBO_ROM_EN },
	{ CLAM, MARLIN_AIRPORT },

	/* Tower */
	{ LIFT, HAGGLEFISH_MARKET },
	{ LIFT, MUSEUM_D_ALFONSINO },
	{ LIFT, INKBLOT_ART_ACADEMY },
	{ LIFT, MAKOMART },
	{ LIFT, ROBO_ROM_EN },
	{ LIFT, MARLIN_AIRPORT },

	/* Rain */
	{ RAIN, UNDERTOW_SPILLWAY },
	{ RAIN, MUSEUM_D_ALFONSINO },
	{ RAIN, MAKOMART },
	{ RAIN, ROBO_ROM_EN },
};

static inline enum mode
str_to_mode_enum(const char *mode) {
	switch (mode[0]) {
		case 'P':
			return TURF;
		case 'R':
			return RAIN;
		case 'A':
			return AREA;
		case 'L':
			return LIFT;
		case 'C':
			return CLAM;
		default:
			__builtin_unreachable();
	}

	return 0;
}

void
fill_rotation_structs(Rotation *restrict rotation_array, int *starthour) {
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
	char *starttime = cJSON_GetObjectItem(normal, "startTime")->valuestring;
	*starthour = (starttime[11] - '0') * 10 + (starttime[12] - '0');

	for (int i = 0; i < 12; i++) {
		cJSON *regular = cJSON_GetObjectItem(normal, "Regular");
		cJSON *series = cJSON_GetObjectItem(normal, "Bankara");
		cJSON *open = cJSON_GetObjectItem(normal, "BankaraOpen");
		cJSON *x = cJSON_GetObjectItem(normal, "X");

		/* Regular battle */
		rotation_data[i].regular_mode = str_to_mode_enum(cJSON_GetObjectItem(regular, "rule")->valuestring);
		rotation_data[i].regular_stage[0] = cJSON_GetObjectItem(regular, "stages")->child->valueint;
		rotation_data[i].regular_stage[1] = cJSON_GetObjectItem(regular, "stages")->child->next->valueint;

		/* Anarchy series */
		rotation_data[i].series_mode = str_to_mode_enum(cJSON_GetObjectItem(series, "rule")->valuestring);
		rotation_data[i].series_stage[0] = cJSON_GetObjectItem(series, "stages")->child->valueint;
		rotation_data[i].series_stage[1] = cJSON_GetObjectItem(series, "stages")->child->next->valueint;

		/* Anarchy open */
		rotation_data[i].open_mode = str_to_mode_enum(cJSON_GetObjectItem(open, "rule")->valuestring);
		rotation_data[i].open_stage[0] = cJSON_GetObjectItem(open, "stages")->child->valueint;
		rotation_data[i].open_stage[1] = cJSON_GetObjectItem(open, "stages")->child->next->valueint;

		/* X battle */
		rotation_data[i].x_mode = str_to_mode_enum(cJSON_GetObjectItem(x, "rule")->valuestring);
		rotation_data[i].x_stage[0] = cJSON_GetObjectItem(x, "stages")->child->valueint;
		rotation_data[i].x_stage[1] = cJSON_GetObjectItem(x, "stages")->child->next->valueint;

		if (normal->next == NULL)
			break;
		normal = normal->next;
	}
}

void
print_anarchy_rotation(int width, int idx) {
	/* Save cursor position */
	fwrite("\e[s", 3, sizeof(char), stdout);

	/* Series */
	int isfav[2] = { 0, 0 };
	printf("\e[B\e[3G\e[96mSeries\e[37m:\e[15G\e[93m%s\e[0m", mode_str[rotation_data[idx].series_mode]);
	for (int i = 0; i < sizeof fav_stages; i++) {
		if (rotation_data[idx].series_mode != fav_stages[i].mode)
			continue;

		if (rotation_data[idx].series_stage[0] == fav_stages[i].map)
			isfav[0] = 1;
		if (rotation_data[idx].series_stage[1] == fav_stages[i].map)
			isfav[1] = 1;
		if (isfav[0] & isfav[1])
			break;
	}
	printf("\e[B\e[5G%c %s\e[B\e[5G%c %s", isfav[0] ? '*' : ' ', stage_str[rotation_data[idx].series_stage[0]], isfav[1] ? '*' : ' ', stage_str[rotation_data[idx].series_stage[1]]);

	/* Open */
	memset(isfav, 0, sizeof isfav);
	printf("\e[2B\e[3G\e[96mOpen\e[37m:\e[15G\e[93m%s\e[0m", mode_str[rotation_data[idx].open_mode]);
	for (int i = 0; i < sizeof fav_stages; i++) {
		if (rotation_data[idx].open_mode != fav_stages[i].mode)
			continue;

		if (rotation_data[idx].open_stage[0] == fav_stages[i].map)
			isfav[0] = 1;
		if (rotation_data[idx].open_stage[1] == fav_stages[i].map)
			isfav[1] = 1;
		if (isfav[0] & isfav[1])
			break;
	}
	printf("\e[B\e[5G%c %s\e[B\e[5G%c %s", isfav[0] ? '*' : ' ', stage_str[rotation_data[idx].open_stage[0]], isfav[1] ? '*' : ' ', stage_str[rotation_data[idx].open_stage[1]]);

	/* Load cursor position */
	fwrite("\e[u", 3, sizeof(char), stdout);
}

void
print_turf_x_rotation(int width, int idx) {
	/* Check width and height are greater than minimum */
	/* TODO */

	/* Calculate x coordinate of second column */
	int mid = width / 2 + 1;

	/* Save cursor position */
	fwrite("\e[s", 3, sizeof(char), stdout);

	/* Turf */
	printf("\e[B\e[%dG\e[92mRegular\e[37m:\e[%dG\e[93m%s\e[0m", mid, mid + 12, mode_str[rotation_data[idx].regular_mode]);
	printf("\e[B\e[%dG%s\e[B\e[%dG%s", mid + 4, stage_str[rotation_data[idx].regular_stage[0]], mid + 4, stage_str[rotation_data[idx].regular_stage[1]]);

	/* X battle */
	int isfav[2] = { 0, 0 };
	printf("\e[2B\e[%dG\e[94mX Battle\e[37m:\e[%dG\e[93m%s\e[0m", mid, mid + 12, mode_str[rotation_data[idx].x_mode]);
	for (int i = 0; i < sizeof fav_stages; i++) {
		if (rotation_data[idx].x_mode != fav_stages[i].mode)
			continue;

		if (rotation_data[idx].x_stage[0] == fav_stages[i].map)
			isfav[0] = 1;
		if (rotation_data[idx].x_stage[1] == fav_stages[i].map)
			isfav[1] = 1;
		if (isfav[0] & isfav[1])
			break;
	}
	printf("\e[B\e[%dG%c %s\e[B\e[%dG%c %s", mid + 2, isfav[0] ? '*' : ' ', stage_str[rotation_data[idx].x_stage[0]], mid + 2, isfav[1] ? '*' : ' ', stage_str[rotation_data[idx].x_stage[1]]);

	/* Load cursor position */
	fwrite("\e[u", 3, sizeof(char), stdout);
}

void
print_rotation_box(int width, int idx, int row, int starttime) {
	/* Check width and height are greater than minimum */
	/* TODO */

	/* Create enough space for the whole box */
	for (int i = 0; i < 10 * (row + 1) - 1; i++)
		putc('\n', stdout);
	printf("\e[%dA", 9);

	/* Save cursor position (top left of rotation box) */
	fwrite("\e[s", 3, sizeof(char), stdout);

	/* Print title and top of rotation box */
	char *titlestr;
	int boxcolour;
	switch (idx) {
		case 0:
			titlestr = malloc(sizeof "Current");
			strcpy(titlestr, "Current");
			boxcolour = 1;
			break;
		case 1:
			titlestr = malloc(sizeof "Next");
			strcpy(titlestr, "Next");
			boxcolour = 5;
			break;
		default:
			titlestr = malloc(sizeof "Future");
			strcpy(titlestr, "Future");
			boxcolour = 2;
			break;
	}

	int timestrlen = 16;
	starttime %= 24;
	if (starttime >= 8) timestrlen++;
	if (starttime >= 10) timestrlen++;
	if (starttime >= 22) timestrlen--;
	printf("\e[9%dm┌─┐ %s\e[37m: \e[9%dm┌", boxcolour, titlestr, boxcolour);
	for (int i = 0; i < width - 8 - timestrlen - strlen(titlestr); i++)
		printf("─");
	if (idx == 0)
		printf("┐ \e[97m%d:00 \e[37m- \e[97m%d:00 \e[9%dm┌─┐", starttime, (starttime + 2) % 24, boxcolour);
	else
		printf("┐ \e[37m%d:00 - %d:00 \e[9%dm┌─┐", starttime, (starttime + 2) % 24, boxcolour);

	/* Print sides of rotation box */
	for (int i = 0; i < 8; i++) {
		printf("│\e[K\e[%dG", width);
		printf("│\e[1G");
		printf("\e[B\e[G");
	}

	/* Print bottom of rotation box */
	printf("└");
	for (int i = 0; i < width - 2; i++)
		printf("─");
	printf("┘");

	/* Cursor position back to top left of box and print contents of box */
	fwrite("\e[u", 3, sizeof(char), stdout);
	print_anarchy_rotation(winsize.ws_col, idx);
	print_turf_x_rotation(winsize.ws_col, idx);
	if (row > 0)
		printf("\e[%dA", 10 * row);
}

int
main(int argc, char *argv[]) {
	/* Initialise stuff */
	static int rows = 2;
	if (argc > 1) {
		int i = 0;
		for (; argv[i] != NULL; i++) {
			int j = 0;
			for (; j < strlen(argv[i]); j++) {
				if (argv[i][j] <= '0' || argv[i][j] >= '9') {
					break;
				}
			}
			if (j == strlen(argv[i])) {
				break;
			}
		}
		if (argv[i] != NULL) {
			rows = 0;
			for (int j = 0; j < strlen(argv[i]); j++) {
				rows *= 10;
				rows += argv[i][j] - '0';
			}
			if (rows > 12)
				rows = 12;
			else if (rows < 1)
				rows = 1;
		}
	}

	tcgetattr(STDIN_FILENO, &term);
	struct termios tmp = term;
	tmp.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, 0, &tmp);

	printf("\e[?25l"); // Hide cursor
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);

	/* Parse rotation data */
	static int starthour;
	fill_rotation_structs(rotation_data, &starthour);

	for (int i = 0; i < rows; i++) {
		print_rotation_box(winsize.ws_col, i, i, i * 2 + starthour);
	}
	fflush(stdout);

	/* Main loop */
	time_t unixtime = time(NULL);
	int waittime = ((unixtime + 3600 % 3600) - unixtime) * 1000;

	static int idx = 0;
	static int run = 1;
	while (run) {
		static struct pollfd pollfds = { .fd = STDIN_FILENO, .events = POLLIN };
		int polret = poll(&pollfds, 1, waittime);
		if (polret > 0) {
			register char c = getc(stdin);
			switch (c) {
				case 'j':
					if (idx < 12 - rows)
						idx++;
					break;
				case 'k':
					if (idx > 0)
						idx--;
					break;
				case 'q':
					run = 0;
					break;
				default:
					break;
			}
		} else if (polret == 0) {
			fill_rotation_structs(rotation_data, &starthour);
			waittime = 900000; // Check every 15 minutes
		} else {
			perror("poll");
		}

		for (int i = 0; i < rows; i++) {
			print_rotation_box(winsize.ws_col, idx + i, i, (idx + i) * 2 + starthour);
		}
		fflush(stdout);
	}

	printf("\e[%dB", rows * 9 + 1);
	printf("\e[?25h\n"); // Show cursor
	tcsetattr(STDIN_FILENO, 0, &term);

	return 0;
}
