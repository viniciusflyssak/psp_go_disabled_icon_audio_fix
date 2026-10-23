#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "logic.h"

static void test_recovery_window(void)
{
    AudiofixRecovery recovery = {0, 0};
    unsigned int now = 0xffff0000u;
    int i;
    assert(!audiofix_recovery_take(&recovery, now));
    assert(audiofix_recovery_request(&recovery, now));
    assert(!audiofix_recovery_take(&recovery, now));
    assert(!audiofix_recovery_request(&recovery, now + 1));
    now += AUDIOFIX_RESUME_DELAY_US; /* Crosses 32-bit timer wrap. */
    assert(!audiofix_recovery_take(&recovery, now - 1));
    for (i = 0; i < AUDIOFIX_RESUME_ATTEMPTS; ++i) {
        assert(audiofix_recovery_take(&recovery, now));
        assert(!audiofix_recovery_take(&recovery, now));
        if (i + 1 < AUDIOFIX_RESUME_ATTEMPTS)
            assert(!audiofix_recovery_request(&recovery, now));
        now += AUDIOFIX_RESUME_STEP_US;
    }
    assert(recovery.remaining == 0);
    assert(!audiofix_recovery_take(&recovery, now));
    /* A later, distinct resume can start its own bounded window. */
    assert(audiofix_recovery_request(&recovery, now));
    /* Missed deadlines produce one attempt, not a catch-up burst. */
    now += 10000000u;
    assert(audiofix_recovery_take(&recovery, now));
    assert(!audiofix_recovery_take(&recovery, now));
}

static void test_snapshot_clock(void)
{
    AudiofixClock clock = {0, 0};
    const unsigned long long threshold = 2000000;
    assert(!audiofix_clock_gap(&clock, 1, 10000000, threshold));
    assert(!audiofix_clock_gap(&clock, 1, 10250000, threshold));
    assert(!audiofix_clock_gap(&clock, 1, 12250000, threshold));
    /* The saved plugin clock is old but the independent RTC advanced. */
    assert(audiofix_clock_gap(&clock, 1, 60000000, threshold));
    assert(!audiofix_clock_gap(&clock, 1, 60250000, threshold));
    /* A backwards RTC change also requests one safe recovery window. */
    assert(audiofix_clock_gap(&clock, 1, 5000000, threshold));
    /* A failed RTC read must not create a false pause interval. */
    assert(!audiofix_clock_gap(&clock, 0, 0, threshold));
    assert(!audiofix_clock_gap(&clock, 1, 90000000, threshold));
}

static void test_volume_labels(void)
{
    struct {
        unsigned char before;
        char text[13];
        unsigned char after;
    } output = {0xa5, {0}, 0x5a};
    int level;
    for (level = 0; level <= 30; ++level) {
        audiofix_volume_text(output.text, level, 30);
        assert(strlen(output.text) == 12);
        assert(!memcmp(output.text, "VOLUME ", 7));
        assert(output.text[7] == '0' + level / 10);
        assert(output.text[8] == '0' + level % 10);
        assert(!strcmp(output.text + 9, "/30"));
        assert(output.before == 0xa5 && output.after == 0x5a);
    }
    audiofix_volume_text(output.text, -1, 30);
    assert(!strcmp(output.text, "VOLUME 00/30"));
    audiofix_volume_text(output.text, 999, 30);
    assert(!strcmp(output.text, "VOLUME 30/30"));
    audiofix_volume_text(output.text, 12, 20);
    assert(!strcmp(output.text, "VOLUME 12/20"));
}

int main(void)
{
    test_recovery_window();
    test_snapshot_clock();
    test_volume_labels();
    puts("PASS: bounded recovery, timer wrap, snapshot clock, volume labels");
    return 0;
}
