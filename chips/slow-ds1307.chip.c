#include "wokwi-api.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// 前幾次連線 NACK，之後正常回應
#define NACK_TIMES 2

// DS1307 暫存器資料：對應 2001-02-03 04:05:06（BCD 格式）
static const uint8_t DS1307_REGS[7] = {
    0x06, // 0x00: seconds  06
    0x05, // 0x01: minutes  05
    0x04, // 0x02: hours    04 (24h mode)
    0x07, // 0x03: day-of-week (skip)
    0x03, // 0x04: date     03
    0x02, // 0x05: month    02
    0x01, // 0x06: year     01 → 2001
};

typedef struct {
    i2c_dev_t i2c;
    uint8_t   connect_count;
    uint8_t   reg_ptr;
    bool      write_phase;
} chip_state_t;

static chip_state_t chip;

static bool    on_i2c_connect(void *user_data, uint32_t address, bool read);
static uint8_t on_i2c_read(void *user_data);
static bool    on_i2c_write(void *user_data, uint8_t data);
static void    on_i2c_disconnect(void *user_data);

void chip_init(void) {
    memset(&chip, 0, sizeof(chip));

    const i2c_config_t cfg = {
        .user_data  = &chip,
        .address    = 0x68,
        .scl        = pin_init("SCL", INPUT),
        .sda        = pin_init("SDA", INPUT),
        .connect    = on_i2c_connect,
        .read       = on_i2c_read,
        .write      = on_i2c_write,
        .disconnect = on_i2c_disconnect,
    };
    chip.i2c = i2c_init(&cfg);
}

static bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
    chip_state_t *c = (chip_state_t *)user_data;
    (void)address;
    (void)read;

    c->connect_count++;

    if (c->connect_count <= NACK_TIMES) {
        // 前 NACK_TIMES 次：拒絕連線，模擬設備尚未就緒
        return false;
    }

    c->reg_ptr    = 0;
    c->write_phase = true;
    return true;
}

static uint8_t on_i2c_read(void *user_data) {
    chip_state_t *c = (chip_state_t *)user_data;

    if (c->reg_ptr < sizeof(DS1307_REGS)) {
        return DS1307_REGS[c->reg_ptr++];
    }
    return 0xFF;
}

static bool on_i2c_write(void *user_data, uint8_t data) {
    chip_state_t *c = (chip_state_t *)user_data;

    if (c->write_phase) {
        c->reg_ptr    = data;
        c->write_phase = false;
    }
    return true;
}

static void on_i2c_disconnect(void *user_data) {
    (void)user_data;
}
