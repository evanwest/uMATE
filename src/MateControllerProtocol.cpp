
#include "MateControllerProtocol.h"

/*
bool MateControllerProtocol::begin()
{
    // auto dtype = _bus.scan(port);
    // if (dtype == DeviceType::Mate) {
    //     this->_port = port;
    //     return true;
    // }
    //return false; // Device not found on this port

    return true;
}
*/

void MateControllerProtocol::set_timeout(int timeoutMillisec)
{
    this->timeout = timeoutMillisec;
}

void MateControllerProtocol::send_packet(uint8_t port, packet_t* packet)
{
    if (packet == nullptr)
        return;

    // AVR is little-endian, but protocol is big-endian. Must swap bytes...
    packet->addr = SWAPENDIAN_16(packet->addr);
    packet->param = SWAPENDIAN_16(packet->param);

    send_data(port, reinterpret_cast<uint8_t*>(packet), sizeof(packet_t));
    delay(1); // BUGFIX: Other end gets confused if we send packets too fast...
}

bool MateControllerProtocol::recv_register_response(OUT uint8_t* for_command, OUT response_t* response)
{
    uint8_t len = sizeof(response_t);
    bool recv_success = this->recv_response(for_command, reinterpret_cast<uint8_t*>(response), len);

    if(recv_success){
        // Now deal with swapping endianness of response
        response->value = SWAPENDIAN_16(response->value);
    }

    return recv_success;
}

bool MateControllerProtocol::recv_register_response_blocking(OUT uint8_t* for_command, OUT response_t* response)
{
    uint8_t len = sizeof(response_t);
    bool recv_success = this->recv_response_blocking(for_command, reinterpret_cast<uint8_t*>(response), len);

    if(recv_success){
        // Now deal with swapping endianness of response
        response->value = SWAPENDIAN_16(response->value);
    }

    return recv_success;
}


bool MateControllerProtocol::recv_response(OUT uint8_t* for_command, OUT uint8_t* response, uint8_t expected_len)
{
    if (for_command == nullptr || response == nullptr)
        return false;

    uint8_t len = expected_len;
    auto err = recv_data(for_command, response, &len);
    if ((err == CommsStatus::Success) && (len == expected_len)) {
        // port is actually the command we're responding to, plus an error flag in bit7
        uint8_t c = *for_command;
        if (c & 0x80) {
            if (debug) { 
                debug->print("Invalid command: ");
                debug->println(c & 0x7F, 16);
            }
            return false; // Invalid command
        }

        // Expect that the caller will deal with endianness themselves.
        return true;
    }
    return false;

}

bool MateControllerProtocol::recv_response_blocking(OUT uint8_t* for_command, OUT uint8_t* response, uint8_t expected_len)
{
    for (int i = 0; i < this->timeout; i++) {
        if (recv_response(for_command, response, expected_len)) {
            return true;
        }
        delay(1);
    }
    if (debug) { debug->println("RX timeout"); }
    return false;
}

// int16_t MateControllerProtocol::increment(uint16_t addr, uint8_t port)
// {

// }
// int16_t MateControllerProtocol::decrement(uint16_t addr, uint8_t port)
// {

// }
// int16_t MateControllerProtocol::disable(uint16_t addr, uint8_t port)
// {

// }
// int16_t MateControllerProtocol::enable(uint16_t addr, uint8_t port)
// {

// }

int16_t MateControllerProtocol::query(uint16_t reg, uint16_t param, uint8_t port)
{
    return read(reg, param, port);
}

int16_t MateControllerProtocol::read(uint16_t addr, uint16_t param, uint8_t port)
{
    packet_t packet;
    //packet.port = port;
    packet.type = PacketType::Read;
    packet.addr = addr;
    packet.param = param;

    send_packet(port, &packet);

    // Wait for response, with timeout (BLOCKING)
    uint8_t for_command;
    response_t response;
    if (recv_register_response_blocking(&for_command, &response)) {
        return response.value;
    }

    return -1;
}

bool MateControllerProtocol::control(uint16_t reg, uint16_t value, uint8_t port)
{
    return write(reg, value, port);
}

bool MateControllerProtocol::write(uint16_t addr, uint16_t value, uint8_t port)
{
    packet_t packet;
    packet.type = PacketType::Write;
    packet.addr = addr;
    packet.param = value;
    
    send_packet(port, &packet);

    // Does this send a response?
    return true;
}

void MateControllerProtocol::scan_ports()
{
    // Clear existing devices
    devices_scanned = false;
    for (int i = 0; i < NUM_PORTS; i++) {
        devices[i] = DeviceType::None;
    }

    DeviceType root_dtype = scan(0);
    devices[0] = root_dtype;

    if (root_dtype == DeviceType::Hub) {
        for (int i = 1; i < NUM_PORTS; i++) {
            devices[i] = scan(i);
        }
    }

    devices_scanned = true;
}

DeviceType MateControllerProtocol::scan(uint8_t port)
{
    if (port > NUM_PORTS)
        return DeviceType::None;

    if (devices_scanned) {
        return devices[port];
    }
    else {
        int16_t value = query(0x00, 0, port);
        if (value >= 0 && value < DeviceType::MaxDevices) {
            return (DeviceType)value;
        }
        return DeviceType::None;
    }
}

int8_t MateControllerProtocol::find_device(DeviceType dtype)
{
    // Use cached devices
    if (!devices_scanned) {
        scan_ports();
    }

    for (int i = 0; i < NUM_PORTS; i++) {
        if (devices[i] == dtype) {
            return i;
        }
    }

    return -1; // Not found.
}

revision_t MateControllerProtocol::get_revision(uint8_t port)
{
    revision_t rev;
    rev.a = query(2, 0, port);
    rev.b = query(3, 0, port);
    rev.c = query(4, 0, port);
    return rev;
}