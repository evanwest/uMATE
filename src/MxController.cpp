#include "MxController.h"


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

uint16_t MxDeviceController::query_watts() {
    return this->query(0x016A);
}

float MxDeviceController::query_watts_float() {
    return (float)this->query_watts();
}

uint16_t MxDeviceController::query_kwh() {
    return this->query(0x01EA);
}

float MxDeviceController::query_kwh_float() {
    return (float)this->query_kwh() / 10.0;
}

uint16_t MxDeviceController::query_amps_dc() {
    return this->query(0x01C7) - 128;
}

uint16_t MxDeviceController::query_batt_voltage() {
    return this->query(0x0008);
}

float MxDeviceController::query_batt_voltage_float() {
    return (float)this->query_batt_voltage() / 10.0;
}

uint16_t MxDeviceController::query_pv_voltage() {
    return this->query(0x01C6);
}

float MxDeviceController::query_pv_voltage_float() {
    return (float)this->query_pv_voltage();
}

mx_status_t MxDeviceController::query_status() {
    uint8_t for_command;
    packet_t request;
    request.type = PacketType::Status;
    request.addr = 1;

    mx_status_t response;
    bool recv_success;

    this->protocol.send_packet(this->get_port(), &request);
    recv_success = this->recv_status_response(&for_command, &response);

    return response;
}

mx_logpage_t MxDeviceController::query_logpage(int day) {
    uint8_t for_command;
    packet_t request;
    request.type = PacketType::Log;
    request.addr = day;

    mx_logpage_t response;
    bool recv_success;

    this->protocol.send_packet(this->get_port(), &request);
    recv_success = this->recv_logpage_response(&for_command, &response);

    return response;
}

bool MxDeviceController::recv_status_response(OUT uint8_t* for_command, OUT mx_status_t* response)
{
    mx_status_packed_t response_packed;
    uint8_t len = sizeof(mx_status_packed_t);
    bool recv_success = this->protocol.recv_response_blocking(for_command, reinterpret_cast<uint8_t*>(&response_packed), len);

    if(recv_success){
        response->raw_ah = (response_packed.e | ((response_packed.a & 0xF0) << 4)) - 2048;
        response->raw_kwh = SWAPENDIAN_16(response_packed.d | (response_packed.j << 8));
        response->pv_current = response_packed.b + 128;
        response->bat_current = ((response_packed.c + 128) * 10) + (response_packed.a & 0x0F);
        response->status = response_packed.status;
        response->errors = SWAPENDIAN_16(response_packed.errors);
        response->bat_voltage = SWAPENDIAN_16(response_packed.bat_voltage);
        response->pv_voltage = SWAPENDIAN_16(response_packed.pv_voltage);
    }

    return recv_success;
}

bool MxDeviceController::recv_logpage_response(OUT uint8_t* for_command, OUT mx_logpage_t* response)
{
    uint8_t len = sizeof(mx_logpage_t);
    bool recv_success = this->protocol.recv_response_blocking(for_command, reinterpret_cast<uint8_t*>(&response), len);

    if(recv_success){
        // TODO: deal with unpacking and swapping endianness
    }

    return recv_success;
}
