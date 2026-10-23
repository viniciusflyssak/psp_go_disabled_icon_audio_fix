#include "config.h"
#include "logic.h"

int audiofix_due(unsigned int now, unsigned int deadline)
{
    return (int)(now - deadline) >= 0;
}

int audiofix_recovery_request(AudiofixRecovery *state, unsigned int now)
{
    /* Coalesce related resume events, never restart an active retry window. */
    if (state->remaining)
        return 0;
    state->remaining = AUDIOFIX_RESUME_ATTEMPTS;
    state->next_attempt = now + AUDIOFIX_RESUME_DELAY_US;
    return 1;
}

int audiofix_recovery_take(AudiofixRecovery *state, unsigned int now)
{
    if (!state->remaining || !audiofix_due(now, state->next_attempt))
        return 0;
    --state->remaining;
    /* Schedule from now, avoiding a burst after another long pause. */
    state->next_attempt = now + AUDIOFIX_RESUME_STEP_US;
    return 1;
}

int audiofix_clock_gap(AudiofixClock *clock, int read_ok,
                      unsigned long long tick, unsigned long long gap_ticks)
{
    int gap = 0;
    if (!read_ok) {
        clock->valid = 0;
        return 0;
    }
    if (clock->valid)
        gap = tick < clock->last || tick - clock->last > gap_ticks;
    clock->last = tick;
    clock->valid = 1;
    return gap;
}

void audiofix_volume_text(char text[13], int level, int maximum)
{
    static const char prefix[] = "VOLUME ";
    int i;
    if (maximum < 0)
        maximum = 0;
    if (maximum > 99)
        maximum = 99;
    if (level < 0)
        level = 0;
    if (level > maximum)
        level = maximum;
    for (i = 0; i < 7; ++i)
        text[i] = prefix[i];
    text[7] = (char)('0' + level / 10);
    text[8] = (char)('0' + level % 10);
    text[9] = '/';
    text[10] = (char)('0' + maximum / 10);
    text[11] = (char)('0' + maximum % 10);
    text[12] = 0;
}
