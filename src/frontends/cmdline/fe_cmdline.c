#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fe_cmdline.h"


static struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"quiet", no_argument, NULL, 'q'},
		{"verbose", no_argument, NULL, 'v'},

		{"get", required_argument, NULL, 'g'},
		{"temperature", no_argument, NULL, 't'},
		{"progress", no_argument, NULL, 'p'},
		{"supported", no_argument, NULL, 's'},

		{"device", required_argument, NULL, 'd'},
		{"heatup", required_argument, NULL, 'w'},
		{"gcode-file", required_argument, NULL, 'f'},
		{"gcode", required_argument, NULL, 'c'},

		{NULL, 0, NULL, 0}
};

int verbosity = 0; //-1 for quiet, 0 for normal, 1 for verbose
char *deviceId = NULL;
char *print_file = NULL, *send_gcode = NULL;
int heatup_temperature = -1;

static int deviceIdRequired = 0;
static ACTION_TYPE action = AT_NONE;


void parse_options(int argc, char **argv) {
	char ch;
	while ((ch = getopt_long(argc, argv, "hqvg:tpsd:w:f:c:", long_options, NULL)) != -1) {
		switch (ch) {
		case 'h': action = AT_SHOW_HELP; break;
		case 'q': verbosity = -1; break;
		case 'v': verbosity = 1; break;
		case 'g':
			if (strcmp(optarg, "temperature") == 0) {
				action = AT_GET_TEMPERATURE;
				deviceIdRequired = 1;
			} else if (strcmp(optarg, "test") == 0) {
				action = AT_GET_TEST;
			} else if (strcmp(optarg, "progress") == 0) {
				action = AT_GET_PROGRESS;
			}
			break;
		case 't':
			action = AT_GET_TEMPERATURE;
			deviceIdRequired = 1;
			break;
		case 'p':
			action = AT_GET_PROGRESS;
			deviceIdRequired = 1;
			break;
		case 's':
			action = AT_GET_SUPPORTED;
			break;
		case 'd':
			deviceId = optarg;
			break;
		case 'w':
			action = AT_HEATUP;
			heatup_temperature = atoi(optarg);
			deviceIdRequired = 1;
			break;
		case 'f':
			print_file = optarg;
			action = AT_PRINT_FILE;
			deviceIdRequired = 1;
			break;
		case 'c':
			send_gcode = optarg;
			action = AT_SEND_CODE;
			deviceIdRequired = 1;
			break;
		case '?': /* signifies an error (like option with missing argument) */
			action = AT_ERROR;
			break;
		}
	}
}


int main(int argc, char **argv) {
	parse_options(argc, argv);

	if (action == AT_ERROR) {
		exit(2);
	}

	if (verbosity >= 0) printf("Print3D command-line client%s\n", (verbosity > 0) ? " (verbose mode)" : "");

	if (deviceIdRequired && !deviceId) {
		deviceId = "xyz"; //default tho this for now
		fprintf(stderr, "error: missing device-id\n");
		exit(1);
	}

	int rv = handle_action(argc, argv, action);

	exit(rv);
}
