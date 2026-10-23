#ifndef AUDIOFIX_OSD_H
#define AUDIOFIX_OSD_H
/* Called exclusively by the worker, or before that worker starts. */
void audiofix_osd_show(const char *message, int result);
void audiofix_osd_volume(int level);
/* Returns 1 when VBlank paced this iteration, otherwise 0. */
int audiofix_osd_tick(int log_result);
#endif
