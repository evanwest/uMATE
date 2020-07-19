#ifndef MX_CONTROLLER_H
#define MX_CONTROLLER_H

#include <stdint.h>
#include "uMate.h"

enum MxStatus {
    Wakeup = 0,
    Float = 1,
    Unknown3 = 2,
    Unknown4 = 3,
    Equalize = 4,
};

typedef struct {
    int8_t pv_current;
    int16_t bat_current;
    uint16_t raw_ah;
    uint16_t raw_kwh;
    uint8_t status;
    uint8_t errors;
    uint16_t bat_voltage;
    uint16_t pv_voltage;
} mx_status_t;

typedef struct {
    int day;
    int amp_hours;
    int kilowatt_hours;
    int volts_peak;
    int amps_peak;
    int kilowatts_peak;
    int bat_min;
    int bat_max;
    int absorb_time;
    int float_time;
} mx_logpage_t;

/*
typedef struct {
    uint8_t a; // raw_ah(part1)
    int8_t b;  // pv_current = b+128
    int8_t c;  // bat_current = c+128
    int8_t d;  // raw_kwh(part1)
    uint8_t e; // raw_ah(part2)
    uint8_t f; // ???
    uint8_t status;
    uint8_t errors;
    uint8_t j; // raw_kwh(part2)
    uint16_t bat_voltage;
    uint16_t pv_voltage;
} mx_status_packed_t;
*/

// #if (sizeof(mx_status_t) != 12)
// #error "Invalid struct size"
// #endif

class MxDeviceController : public MateControllerDevice 
{
public:
    MxDeviceController(MateControllerProtocol& protocol) 
        : MateControllerDevice(protocol, DeviceType::Mx) {}

    uint16_t query_watts();

    float query_watts_float();

    uint16_t query_kwh();

    float query_kwh_float();

    uint16_t query_amps_dc();

    float query_amps_dc_float();

    uint16_t query_batt_voltage();

    float query_batt_voltage_float();

    uint16_t query_pv_voltage();

    float query_pv_voltage_float();

    mx_status_t query_status();

    mx_logpage_t query_logpage(int day);

private:
    bool recv_status_response(OUT uint8_t* for_command, OUT mx_status_t* response);

    bool recv_logpage_response(OUT uint8_t* for_command, OUT mx_logpage_t* response);

};

#endif /* MX_CONTROLLER_H */