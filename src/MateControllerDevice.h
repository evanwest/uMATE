#ifndef MATE_CONTROLLER_DEVICE_H
#define MATE_CONTROLLER_DEVICE_H

class MateControllerDevice {
public:
    MateControllerDevice(
        MateControllerProtocol& protocol, 
        DeviceType dtype
    ) : 
        protocol(protocol),
        dtype(dtype),
        port(-1),
        is_open(false)
    { }

    // Open a device on the specified port.
    // returns false if the device is not present on that port.
    bool begin(uint8_t port) {
        this->port = port;
        is_open = (protocol.scan(port) == dtype);
        return is_open;
    }

    // Query a register and retrieve its value (BLOCKING)
    // reg:      The register address
    // param:    Optional parameter
    // returns:  The register value
    uint16_t query(uint16_t reg, uint16_t param = 0) {
        return protocol.query(reg, param, this->port);
    }

    // Control something (BLOCKING)
    // reg:      The control address
    // value:    The value to use for controlling (eg. disable/enable)
    void control(uint16_t reg, uint16_t value) {
        protocol.control(reg, value, this->port);
    }

    // Read the revision from the target device
    revision_t get_revision() {
        return protocol.get_revision(this->port);
    }

    int8_t get_port() {
        return this->port;
    }

    void send(uint8_t port, uint8_t type, uint16_t addr, uint16_t param) {
        packet_t packet = {.type = type, .addr = addr, .param = param};
        this->protocol.send_packet(port, &packet);
    }

protected:
    MateControllerProtocol& protocol;
    DeviceType dtype;
    bool is_open;
    int8_t port;
};

#endif /* MATE_CONTROLLER_DEVICE_H */