#ifndef FE_CMDLINE_H_SEEN
#define FE_CMDLINE_H_SEEN

/* from fe_cmdline.c */
extern char *deviceId;
extern char *print_file;
extern char *send_gcode;

/* from actions.c */
void printTemperature();
void printTestResponse();

#endif /* ! FE_CMDLINE_H_SEEN */