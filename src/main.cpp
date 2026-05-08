#include <Arduino.h>
#include <Wire.h>

#define DS1307_ADDR 0x68

enum RTCStatus {
    RTC_OK = 0,
    RTC_READ_FAILED = -1,
    RTC_INVALID_DATA = -2
};

static uint8_t bcdToDec(uint8_t value) {
    return ((value >> 4) * 10) + (value & 0x0F);
}

static bool isValidTime(
    uint16_t year,
    uint8_t month,
    uint8_t date,
    uint8_t hours,
    uint8_t minutes,
    uint8_t seconds
) {
    if (year < 2000 || year > 2099) return false;
    if (month < 1 || month > 12) return false;
    if (date < 1 || date > 31) return false;
    if (hours > 23) return false;
    if (minutes > 59) return false;
    if (seconds > 59) return false;
    return true;
}

static RTCStatus readDS1307(
    uint16_t &year,
    uint8_t &month,
    uint8_t &date,
    uint8_t &hours,
    uint8_t &minutes,
    uint8_t &seconds
) {
    Wire.beginTransmission(DS1307_ADDR);
    Wire.write(0x00);

    if (Wire.endTransmission(false) != 0) {
        return RTC_READ_FAILED;
    }

    uint8_t received = Wire.requestFrom(DS1307_ADDR, 7);
    if (received != 7) {
        return RTC_READ_FAILED;
    }

    uint8_t rawSeconds = Wire.read();
    uint8_t rawMinutes = Wire.read();
    uint8_t rawHours = Wire.read();
    Wire.read(); // day of week, not used
    uint8_t rawDate = Wire.read();
    uint8_t rawMonth = Wire.read();
    uint8_t rawYear = Wire.read();

    seconds = bcdToDec(rawSeconds & 0x7F);
    minutes = bcdToDec(rawMinutes);
    hours = bcdToDec(rawHours & 0x3F);
    date = bcdToDec(rawDate);
    month = bcdToDec(rawMonth);
    year = 2000 + bcdToDec(rawYear);

    if (!isValidTime(year, month, date, hours, minutes, seconds)) {
        return RTC_INVALID_DATA;
    }

    return RTC_OK;
}

void setup() {
    Serial.begin(115200);
    delay(100);

    Wire.setSDA(PB7);
    Wire.setSCL(PB6);
    Wire.begin();

    Serial.println("BOOT OK");

    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t date = 0;
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;

    RTCStatus status = RTC_READ_FAILED;

    for (int attempt = 0; attempt < 3; attempt++) {
        status = readDS1307(year, month, date, hours, minutes, seconds);
        if (status == RTC_OK) {
            break;
        }
        delay(50);
    }

    if (status == RTC_OK) {
        char buffer[32];
        snprintf(
            buffer,
            sizeof(buffer),
            "TIME %04u-%02u-%02u %02u:%02u:%02u",
            year,
            month,
            date,
            hours,
            minutes,
            seconds
        );
        Serial.println(buffer);
    } else if (status == RTC_INVALID_DATA) {
        Serial.println("ERROR RTC_INVALID_DATA");
    } else {
        Serial.println("ERROR RTC_READ_FAILED");
    }
}

void loop() {}
