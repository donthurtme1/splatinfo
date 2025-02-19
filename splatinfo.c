#include <cjson/cJSON.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include "types.h"

static struct winsize winsize;

int main(int argc, char *argv[]) {
	FILE *fcurloutput = fopen("/home/basil/splat-info/output-info.json", "w");

	/* Initialise stuff */
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);

	/* Start child to call curl */
	static int fds[2];
	pipe(fds);
	int curl_pid = fork();
	if (curl_pid == 0) {
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO);
		execvp("curl", (char *[]){ "curl", "--silent", "https://splatoon.oatmealdome.me/api/v1/three/versus/phases?count=24", NULL });
		exit(0);
	}
	close(fds[1]);
	dup2(fds[0], STDIN_FILENO);

	static size_t len;
	static char *curl_output;
	wait(&curl_pid);
	if (getline(&curl_output, &len, stdin) < 0) {
		printf("asdf\n");
	}

	cJSON *full_json = cJSON_Parse(curl_output);
	cJSON *normal = cJSON_GetObjectItem(full_json, "normal");

	/* Regular battle */
	cJSON *regular = cJSON_GetObjectItem(normal->child, "Regular");
	cJSON *regular_mode = cJSON_GetObjectItem(regular, "rule");
	cJSON *regular_stages = cJSON_GetObjectItem(regular, "stages");

	/* Anarchy series */
	cJSON *bankara = cJSON_GetObjectItem(normal->child, "Bankara");
	cJSON *bankara_mode = cJSON_GetObjectItem(bankara, "rule");
	cJSON *bankara_stages = cJSON_GetObjectItem(bankara, "stages");

	/* Anarchy open */
	cJSON *open = cJSON_GetObjectItem(normal->child, "BankaraOpen");
	cJSON *open_mode = cJSON_GetObjectItem(open, "rule");
	cJSON *open_stages = cJSON_GetObjectItem(open, "stages");

	/* X battle */
	cJSON *xbattle = cJSON_GetObjectItem(normal->child, "X");
	cJSON *xbattle_mode = cJSON_GetObjectItem(xbattle, "rule");
	cJSON *xbattle_stages = cJSON_GetObjectItem(xbattle, "stages");

	/* Salmon run */
	cJSON *salmon = cJSON_GetObjectItem(normal->child, "X");
	cJSON *salmon_mode = cJSON_GetObjectItem(salmon, "rule");
	cJSON *salmon_stages = cJSON_GetObjectItem(salmon, "stages");

	printf("\n");

	printf("\e[91mCurrent\e[37m:\n");
	printf("    \e[96mSeries\e[37m: \e[93m%s                  \e[92mRegular\e[37m: \e[93mTurf\e[0m\n", bankara_mode->valuestring);
	printf("        %s\e[39G%s\n", stage_str[bankara_stages->child->valueint], stage_str[regular_stages->child->valueint]);
	printf("        %s\e[39G%s\n", stage_str[bankara_stages->child->next->valueint], stage_str[regular_stages->child->next->valueint]);
	printf("    \e[96mOpen\e[37m: \e[93m%s                    \e[96mSalmon Run\e[37m:\e[0m\n", open_mode->valuestring);
	printf("        %s\e[39G%s\n", stage_str[open_stages->child->valueint], stage_str[salmon_stages->child->valueint]);
	printf("        %s\e[39G%s\n", stage_str[open_stages->child->next->valueint], stage_str[salmon_stages->child->next->valueint]);
	printf("    \e[94mX Battle\e[37m: \e[93m%s\e[0m\n", xbattle_mode->valuestring);
	printf("        %s\n", stage_str[xbattle_stages->child->valueint]);
	printf("        %s\n\n", stage_str[xbattle_stages->child->next->valueint]);

	/*
	bankara = cJSON_GetObjectItem(normal->child->next, "Bankara");
	bankara_mode = cJSON_GetObjectItem(bankara, "rule");
	bankara_stages = cJSON_GetObjectItem(bankara, "stages");
	open = cJSON_GetObjectItem(normal->child->next, "BankaraOpen");
	open_mode = cJSON_GetObjectItem(open, "rule");
	open_stages = cJSON_GetObjectItem(open, "stages");
	xbattle = cJSON_GetObjectItem(normal->child->next, "X");
	xbattle_mode = cJSON_GetObjectItem(xbattle, "rule");
	xbattle_stages = cJSON_GetObjectItem(xbattle, "stages");

	printf("\e[95mNext\e[37m:\n");
	printf("    \e[96mSeries\e[37m: \e[93m%s\e[0m\n        %s\n        %s\n\n", bankara_mode->valuestring, stage_str[bankara_stages->child->valueint], stage_str[bankara_stages->child->next->valueint]);
	printf("    \e[96mOpen\e[37m: \e[93m%s\e[0m\n        %s\n        %s\n\n", open_mode->valuestring, stage_str[open_stages->child->valueint], stage_str[open_stages->child->next->valueint]);
	printf("    \e[94mX Battle\e[37m: \e[93m%s\e[0m\n        %s\n        %s\n\n", xbattle_mode->valuestring, stage_str[xbattle_stages->child->valueint], stage_str[xbattle_stages->child->next->valueint]);

	printf("\e[92mFuture\e[37m:\n");
	printf("   \e[96mSeries\e[37m:              \e[93m%s\e[0m\n      %s\n      %s\n\n", bankara_mode->valuestring, stage_str[bankara_stages->child->valueint], stage_str[bankara_stages->child->next->valueint]);
	printf("   \e[96mOpen\e[37m:                \e[93m%s\e[0m\n      %s\n      %s\n\n", open_mode->valuestring, stage_str[open_stages->child->valueint], stage_str[open_stages->child->next->valueint]);
	printf("   \e[94mX Battle\e[37m:            \e[93m%s\e[0m\n      %s\n      %s\n\n", xbattle_mode->valuestring, stage_str[xbattle_stages->child->valueint], stage_str[xbattle_stages->child->next->valueint]);
	*/

	return 0;
}
