#include "unity.h"
#include "log_helpers.h"

#include <assert.h>
#include <string.h>

#include "io_reg.h"

static io_reg_t io_reg;

// ---- Mock functions ----

typedef struct {
    interrupt_regs_t *interrupts;
} interrupts_init_call_t;

typedef struct {
    size_t call_count;
    interrupts_init_call_t calls[10];
} interrupts_init_stats_t;

static interrupts_init_stats_t interrupts_init_stats;

void interrupts_init(interrupt_regs_t *interrupts) {
    if (interrupts_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for interrupts_init_stats");
    }

    interrupts_init_call_t *call = &interrupts_init_stats.calls[interrupts_init_stats.call_count];
    call->interrupts = interrupts;

    interrupts_init_stats.call_count++;
}

typedef struct {
    interrupt_regs_t *interrupts;
    uint16_t addr;
    uint8_t return_value;
} interrupts_read_call_t;

typedef struct {
    size_t call_count;
    interrupts_read_call_t calls[10];
} interrupts_read_stats_t;

static interrupts_read_stats_t interrupts_read_stats;

uint8_t interrupts_read(interrupt_regs_t *interrupts, uint16_t addr) {
    if (interrupts_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for interrupts_read_stats");
    }

    interrupts_read_call_t *call = &interrupts_read_stats.calls[interrupts_read_stats.call_count];
    call->interrupts = interrupts;
    call->addr = addr;

    interrupts_read_stats.call_count++;

    return call->return_value;
}

typedef struct {
    interrupt_regs_t *interrupts;
    uint16_t addr;
    uint8_t value;
} interrupts_write_call_t;

typedef struct {
    size_t call_count;
    interrupts_write_call_t calls[10];
} interrupts_write_stats_t;

static interrupts_write_stats_t interrupts_write_stats;

void interrupts_write(interrupt_regs_t *interrupts, uint16_t addr, uint8_t value) {
    if (interrupts_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for interrupts_write_stats");
    }

    interrupts_write_call_t *call = &interrupts_write_stats.calls[interrupts_write_stats.call_count];
    call->interrupts = interrupts;
    call->addr = addr;
    call->value = value;

    interrupts_write_stats.call_count++;
}

typedef struct {
    timer_regs_t *timer;
} timer_init_call_t;

typedef struct {
    size_t call_count;
    timer_init_call_t calls[10];
} timer_init_stats_t;

static timer_init_stats_t timer_init_stats;

void timer_init(timer_regs_t *timer) {
    if (timer_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for timer_init_stats");
    }

    timer_init_call_t *call = &timer_init_stats.calls[timer_init_stats.call_count];
    call->timer = timer;

    timer_init_stats.call_count++;
}

typedef struct {
    timer_regs_t *timer;
    uint16_t addr;
    uint8_t return_value;
} timer_read_call_t;

typedef struct {
    size_t call_count;
    timer_read_call_t calls[10];
} timer_read_stats_t;

static timer_read_stats_t timer_read_stats;

uint8_t timer_read(timer_regs_t *timer, uint16_t addr) {
    if (timer_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for timer_read_stats");
    }

    timer_read_call_t *call = &timer_read_stats.calls[timer_read_stats.call_count];
    call->timer = timer;
    call->addr = addr;

    timer_read_stats.call_count++;

    return call->return_value;
}

typedef struct {
    timer_regs_t *timer;
    uint16_t addr;
    uint8_t value;
} timer_write_call_t;

typedef struct {
    size_t call_count;
    timer_write_call_t calls[10];
} timer_write_stats_t;

static timer_write_stats_t timer_write_stats;

void timer_write(timer_regs_t *timer, uint16_t addr, uint8_t value) {
    if (timer_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for timer_write_stats");
    }

    timer_write_call_t *call = &timer_write_stats.calls[timer_write_stats.call_count];
    call->timer = timer;
    call->addr = addr;
    call->value = value;

    timer_write_stats.call_count++;
}

typedef struct {
    joypad_reg_t *joyp;
} joypad_init_call_t;

typedef struct {
    size_t call_count;
    joypad_init_call_t calls[10];
} joypad_init_stats_t;

static joypad_init_stats_t joypad_init_stats;

void joypad_init(joypad_reg_t *joyp) {
    if (joypad_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for joypad_init_stats");
    }

    joypad_init_call_t *call = &joypad_init_stats.calls[joypad_init_stats.call_count];
    call->joyp = joyp;

    joypad_init_stats.call_count++;
}

typedef struct {
    joypad_reg_t *joyp;
    uint8_t return_value;
} joypad_read_call_t;

typedef struct {
    size_t call_count;
    joypad_read_call_t calls[10];
} joypad_read_stats_t;

static joypad_read_stats_t joypad_read_stats;

uint8_t joypad_read(joypad_reg_t *joyp) {
    if (joypad_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for joypad_read_stats");
    }

    joypad_read_call_t *call = &joypad_read_stats.calls[joypad_read_stats.call_count];
    call->joyp = joyp;

    joypad_read_stats.call_count++;

    return call->return_value;
}

typedef struct {
    joypad_reg_t *joyp;
    uint8_t value;
} joypad_write_call_t;

typedef struct {
    size_t call_count;
    joypad_write_call_t calls[10];
} joypad_write_stats_t;

static joypad_write_stats_t joypad_write_stats;

void joypad_write(joypad_reg_t *joyp, uint8_t value) {
    if (joypad_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for joypad_write_stats");
    }

    joypad_write_call_t *call = &joypad_write_stats.calls[joypad_write_stats.call_count];
    call->joyp = joyp;
    call->value = value;

    joypad_write_stats.call_count++;
}

typedef struct {
    st_regs_t *serial;
} serial_transfer_init_call_t;

typedef struct {
    size_t call_count;
    serial_transfer_init_call_t calls[10];
} serial_transfer_init_stats_t;

static serial_transfer_init_stats_t serial_transfer_init_stats;

void serial_transfer_init(st_regs_t *serial) {
    if (serial_transfer_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for serial_transfer_init_stats");
    }

    serial_transfer_init_call_t *call = &serial_transfer_init_stats.calls[serial_transfer_init_stats.call_count];
    call->serial = serial;

    serial_transfer_init_stats.call_count++;
}

typedef struct {
    st_regs_t *serial;
    uint16_t addr;
    uint8_t return_value;
} serial_transfer_read_call_t;

typedef struct {
    size_t call_count;
    serial_transfer_read_call_t calls[10];
} serial_transfer_read_stats_t;

static serial_transfer_read_stats_t serial_transfer_read_stats;

uint8_t serial_transfer_read(st_regs_t *serial, uint16_t addr) {
    if (serial_transfer_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for serial_transfer_read_stats");
    }

    serial_transfer_read_call_t *call = &serial_transfer_read_stats.calls[serial_transfer_read_stats.call_count];
    call->serial = serial;
    call->addr = addr;

    serial_transfer_read_stats.call_count++;

    return call->return_value;
}

typedef struct {
    st_regs_t *serial;
    uint16_t addr;
    uint8_t value;
} serial_transfer_write_call_t;

typedef struct {
    size_t call_count;
    serial_transfer_write_call_t calls[10];
} serial_transfer_write_stats_t;

static serial_transfer_write_stats_t serial_transfer_write_stats;

void serial_transfer_write(st_regs_t *serial, uint16_t addr, uint8_t value) {
    if (serial_transfer_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for serial_transfer_write_stats");
    }

    serial_transfer_write_call_t *call = &serial_transfer_write_stats.calls[serial_transfer_write_stats.call_count];
    call->serial = serial;
    call->addr = addr;
    call->value = value;

    serial_transfer_write_stats.call_count++;
}

typedef struct {
    audio_regs_t *audio;
} audio_init_call_t;

typedef struct {
    size_t call_count;
    audio_init_call_t calls[10];
} audio_init_stats_t;

static audio_init_stats_t audio_init_stats;

void audio_init(audio_regs_t *audio) {
    if (audio_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for audio_init_stats");
    }

    audio_init_call_t *call = &audio_init_stats.calls[audio_init_stats.call_count];
    call->audio = audio;

    audio_init_stats.call_count++;
}

typedef struct {
    audio_regs_t *audio;
    uint16_t addr;
    uint8_t return_value;
} audio_read_call_t;

typedef struct {
    size_t call_count;
    audio_read_call_t calls[10];
} audio_read_stats_t;

static audio_read_stats_t audio_read_stats;

uint8_t audio_read(audio_regs_t *audio, uint16_t addr) {
    if (audio_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for audio_read_stats");
    }

    audio_read_call_t *call = &audio_read_stats.calls[audio_read_stats.call_count];
    call->audio = audio;
    call->addr = addr;

    audio_read_stats.call_count++;

    return call->return_value;
}

typedef struct {
    audio_regs_t *audio;
    uint16_t addr;
    uint8_t value;
} audio_write_call_t;

typedef struct {
    size_t call_count;
    audio_write_call_t calls[10];
} audio_write_stats_t;

static audio_write_stats_t audio_write_stats;

void audio_write(audio_regs_t *audio, uint16_t addr, uint8_t value) {
    if (audio_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for audio_write_stats");
    }

    audio_write_call_t *call = &audio_write_stats.calls[audio_write_stats.call_count];
    call->audio = audio;
    call->addr = addr;
    call->value = value;

    audio_write_stats.call_count++;
}

typedef struct {
    lcd_regs_t *lcd;
} lcd_init_call_t;

typedef struct {
    size_t call_count;
    lcd_init_call_t calls[10];
} lcd_init_stats_t;

static lcd_init_stats_t lcd_init_stats;

void lcd_init(lcd_regs_t *lcd) {
    if (lcd_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for lcd_init_stats");
    }

    lcd_init_call_t *call = &lcd_init_stats.calls[lcd_init_stats.call_count];
    call->lcd = lcd;

    lcd_init_stats.call_count++;
}

typedef struct {
    lcd_regs_t *lcd;
    uint16_t addr;
    uint8_t return_value;
} lcd_read_call_t;

typedef struct {
    size_t call_count;
    lcd_read_call_t calls[10];
} lcd_read_stats_t;

static lcd_read_stats_t lcd_read_stats;

uint8_t lcd_read(lcd_regs_t *lcd, uint16_t addr) {
    if (lcd_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for lcd_read_stats");
    }

    lcd_read_call_t *call = &lcd_read_stats.calls[lcd_read_stats.call_count];
    call->lcd = lcd;
    call->addr = addr;

    lcd_read_stats.call_count++;

    return call->return_value;
}

typedef struct {
    lcd_regs_t *lcd;
    uint16_t addr;
    uint8_t value;
} lcd_write_call_t;

typedef struct {
    size_t call_count;
    lcd_write_call_t calls[10];
} lcd_write_stats_t;

static lcd_write_stats_t lcd_write_stats;

void lcd_write(lcd_regs_t *lcd, uint16_t addr, uint8_t value) {
    if (lcd_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for lcd_write_stats");
    }

    lcd_write_call_t *call = &lcd_write_stats.calls[lcd_write_stats.call_count];
    call->lcd = lcd;
    call->addr = addr;
    call->value = value;

    lcd_write_stats.call_count++;
}

void setUp(void) {
    suppress_logs();

    io_reg = (io_reg_t){ 0 };

    interrupts_init_stats = (interrupts_init_stats_t){ 0 };
    interrupts_read_stats = (interrupts_read_stats_t){ 0 };
    interrupts_write_stats = (interrupts_write_stats_t){ 0 };
    timer_init_stats = (timer_init_stats_t){ 0 };
    timer_read_stats = (timer_read_stats_t){ 0 };
    timer_write_stats = (timer_write_stats_t){ 0 };
    joypad_init_stats = (joypad_init_stats_t){ 0 };
    joypad_read_stats = (joypad_read_stats_t){ 0 };
    joypad_write_stats = (joypad_write_stats_t){ 0 };
    serial_transfer_init_stats = (serial_transfer_init_stats_t){ 0 };
    serial_transfer_read_stats = (serial_transfer_read_stats_t){ 0 };
    serial_transfer_write_stats = (serial_transfer_write_stats_t){ 0 };
    audio_init_stats = (audio_init_stats_t){ 0 };
    audio_read_stats = (audio_read_stats_t){ 0 };
    audio_write_stats = (audio_write_stats_t){ 0 };
    lcd_init_stats = (lcd_init_stats_t){ 0 };
    lcd_read_stats = (lcd_read_stats_t){ 0 };
    lcd_write_stats = (lcd_write_stats_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- io_reg_init ----

void test_io_reg_init_initializes_joypad(void) {
    io_reg_init(&io_reg);

    TEST_ASSERT_EQUAL_size_t(1, joypad_init_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.joyp, joypad_init_stats.calls[0].joyp);
}

void test_io_reg_init_initializes_serial_transfer(void) {
    io_reg_init(&io_reg);

    TEST_ASSERT_EQUAL_size_t(1, serial_transfer_init_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.serial_transfer, serial_transfer_init_stats.calls[0].serial);
}

void test_io_reg_init_initializes_interrupts(void) {
    io_reg_init(&io_reg);

    TEST_ASSERT_EQUAL_size_t(1, interrupts_init_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.interrupts, interrupts_init_stats.calls[0].interrupts);
}

void test_io_reg_init_initializes_timer(void) {
    io_reg_init(&io_reg);

    TEST_ASSERT_EQUAL_size_t(1, timer_init_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.timer, timer_init_stats.calls[0].timer);
}

void test_io_reg_init_initializes_audio(void) {
    io_reg_init(&io_reg);

    TEST_ASSERT_EQUAL_size_t(1, audio_init_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.audio, audio_init_stats.calls[0].audio);
}

void test_io_reg_init_initializes_lcd(void) {
    io_reg_init(&io_reg);

    TEST_ASSERT_EQUAL_size_t(1, lcd_init_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.lcd, lcd_init_stats.calls[0].lcd);
}

// ---- io_reg_read ----

void test_io_reg_read_joypad(void) {
    joypad_read_stats.calls[0].return_value = 0x3C;

    TEST_ASSERT_EQUAL_UINT8(0x3C, io_reg_read(&io_reg, 0xFF00));

    TEST_ASSERT_EQUAL_size_t(1, joypad_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.joyp, joypad_read_stats.calls[0].joyp);
}

void test_io_reg_read_serial_transfer(void) {
    serial_transfer_read_stats.calls[0].return_value = 0x3C;

    TEST_ASSERT_EQUAL_UINT8(0x3C, io_reg_read(&io_reg, 0xFF01));

    TEST_ASSERT_EQUAL_size_t(1, serial_transfer_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.serial_transfer, serial_transfer_read_stats.calls[0].serial);
    TEST_ASSERT_EQUAL_UINT16(0xFF01, serial_transfer_read_stats.calls[0].addr);
}

void test_io_reg_read_timer(void) {
    timer_read_stats.calls[0].return_value = 0x3C;

    TEST_ASSERT_EQUAL_UINT8(0x3C, io_reg_read(&io_reg, 0xFF04));

    TEST_ASSERT_EQUAL_size_t(1, timer_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.timer, timer_read_stats.calls[0].timer);
    TEST_ASSERT_EQUAL_UINT16(0xFF04, timer_read_stats.calls[0].addr);
}

void test_io_reg_read_interrupts(void) {
    interrupts_read_stats.calls[0].return_value = 0x3C;

    TEST_ASSERT_EQUAL_UINT8(0x3C, io_reg_read(&io_reg, 0xFF0F));

    TEST_ASSERT_EQUAL_size_t(1, interrupts_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.interrupts, interrupts_read_stats.calls[0].interrupts);
    TEST_ASSERT_EQUAL_UINT16(0xFF0F, interrupts_read_stats.calls[0].addr);
}

void test_io_reg_read_interrupt_enable(void) {
    interrupts_read_stats.calls[0].return_value = 0x3C;

    TEST_ASSERT_EQUAL_UINT8(0x3C, io_reg_read(&io_reg, 0xFFFF));

    TEST_ASSERT_EQUAL_size_t(1, interrupts_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.interrupts, interrupts_read_stats.calls[0].interrupts);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, interrupts_read_stats.calls[0].addr);
}

void test_io_reg_read_audio(void) {
    audio_read_stats.calls[0].return_value = 0x3C;

    TEST_ASSERT_EQUAL_UINT8(0x3C, io_reg_read(&io_reg, 0xFF10));

    TEST_ASSERT_EQUAL_size_t(1, audio_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.audio, audio_read_stats.calls[0].audio);
    TEST_ASSERT_EQUAL_UINT16(0xFF10, audio_read_stats.calls[0].addr);
}

void test_io_reg_read_lcd(void) {
    lcd_read_stats.calls[0].return_value = 0x3C;

    TEST_ASSERT_EQUAL_UINT8(0x3C, io_reg_read(&io_reg, 0xFF40));

    TEST_ASSERT_EQUAL_size_t(1, lcd_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.lcd, lcd_read_stats.calls[0].lcd);
    TEST_ASSERT_EQUAL_UINT16(0xFF40, lcd_read_stats.calls[0].addr);
}

void test_io_reg_read_unimplemented_returns_0xFF(void) {
    // 0xFF03 is not mapped to any component
    TEST_ASSERT_EQUAL_UINT8(0xFF, io_reg_read(&io_reg, 0xFF03));

    TEST_ASSERT_EQUAL_size_t(0, joypad_read_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, serial_transfer_read_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, timer_read_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, interrupts_read_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, audio_read_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, lcd_read_stats.call_count);
}

// ---- io_reg_write ----

void test_io_reg_write_joypad(void) {
    io_reg_write(&io_reg, 0xFF00, 0x3C);

    TEST_ASSERT_EQUAL_size_t(1, joypad_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.joyp, joypad_write_stats.calls[0].joyp);
    TEST_ASSERT_EQUAL_UINT8(0x3C, joypad_write_stats.calls[0].value);
}

void test_io_reg_write_serial_transfer(void) {
    io_reg_write(&io_reg, 0xFF01, 0x3C);

    TEST_ASSERT_EQUAL_size_t(1, serial_transfer_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.serial_transfer, serial_transfer_write_stats.calls[0].serial);
    TEST_ASSERT_EQUAL_UINT16(0xFF01, serial_transfer_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x3C, serial_transfer_write_stats.calls[0].value);
}

void test_io_reg_write_timer(void) {
    io_reg_write(&io_reg, 0xFF04, 0x3C);

    TEST_ASSERT_EQUAL_size_t(1, timer_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.timer, timer_write_stats.calls[0].timer);
    TEST_ASSERT_EQUAL_UINT16(0xFF04, timer_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x3C, timer_write_stats.calls[0].value);
}

void test_io_reg_write_interrupts(void) {
    io_reg_write(&io_reg, 0xFF0F, 0x3C);

    TEST_ASSERT_EQUAL_size_t(1, interrupts_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.interrupts, interrupts_write_stats.calls[0].interrupts);
    TEST_ASSERT_EQUAL_UINT16(0xFF0F, interrupts_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x3C, interrupts_write_stats.calls[0].value);
}

void test_io_reg_write_interrupt_enable(void) {
    io_reg_write(&io_reg, 0xFFFF, 0x3C);

    TEST_ASSERT_EQUAL_size_t(1, interrupts_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.interrupts, interrupts_write_stats.calls[0].interrupts);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, interrupts_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x3C, interrupts_write_stats.calls[0].value);
}

void test_io_reg_write_audio(void) {
    io_reg_write(&io_reg, 0xFF10, 0x3C);

    TEST_ASSERT_EQUAL_size_t(1, audio_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.audio, audio_write_stats.calls[0].audio);
    TEST_ASSERT_EQUAL_UINT16(0xFF10, audio_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x3C, audio_write_stats.calls[0].value);
}

void test_io_reg_write_lcd(void) {
    io_reg_write(&io_reg, 0xFF40, 0x3C);

    TEST_ASSERT_EQUAL_size_t(1, lcd_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&io_reg.lcd, lcd_write_stats.calls[0].lcd);
    TEST_ASSERT_EQUAL_UINT16(0xFF40, lcd_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x3C, lcd_write_stats.calls[0].value);
}

void test_io_reg_write_unimplemented_is_ignored(void) {
    // 0xFF03 is not mapped to any component
    io_reg_write(&io_reg, 0xFF03, 0x3C);

    TEST_ASSERT_EQUAL_size_t(0, joypad_write_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, serial_transfer_write_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, timer_write_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, interrupts_write_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, audio_write_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, lcd_write_stats.call_count);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_io_reg_init_initializes_joypad);
    RUN_TEST(test_io_reg_init_initializes_serial_transfer);
    RUN_TEST(test_io_reg_init_initializes_interrupts);
    RUN_TEST(test_io_reg_init_initializes_timer);
    RUN_TEST(test_io_reg_init_initializes_audio);
    RUN_TEST(test_io_reg_init_initializes_lcd);

    RUN_TEST(test_io_reg_read_joypad);
    RUN_TEST(test_io_reg_read_serial_transfer);
    RUN_TEST(test_io_reg_read_timer);
    RUN_TEST(test_io_reg_read_interrupts);
    RUN_TEST(test_io_reg_read_interrupt_enable);
    RUN_TEST(test_io_reg_read_audio);
    RUN_TEST(test_io_reg_read_lcd);
    RUN_TEST(test_io_reg_read_unimplemented_returns_0xFF);

    RUN_TEST(test_io_reg_write_joypad);
    RUN_TEST(test_io_reg_write_serial_transfer);
    RUN_TEST(test_io_reg_write_timer);
    RUN_TEST(test_io_reg_write_interrupts);
    RUN_TEST(test_io_reg_write_interrupt_enable);
    RUN_TEST(test_io_reg_write_audio);
    RUN_TEST(test_io_reg_write_lcd);
    RUN_TEST(test_io_reg_write_unimplemented_is_ignored);

    return UNITY_END();
}
