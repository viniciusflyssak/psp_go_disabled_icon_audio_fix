#ifndef AUDIOFIX_LOGIC_H
#define AUDIOFIX_LOGIC_H

/* Platform-independent timing/formatting, shared by the PRX and host tests. */
typedef struct {
    unsigned int next_attempt;
    int remaining;
} AudiofixRecovery;

typedef struct {
    unsigned long long last;
    int valid;
} AudiofixClock;

int audiofix_due(unsigned int now, unsigned int deadline);
int audiofix_recovery_request(AudiofixRecovery *state, unsigned int now);
int audiofix_recovery_take(AudiofixRecovery *state, unsigned int now);
int audiofix_clock_gap(AudiofixClock *clock, int read_ok,
                      unsigned long long tick, unsigned long long gap_ticks);
void audiofix_volume_text(char text[13], int level, int maximum);
#endif
