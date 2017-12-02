#pragma once

#include <cinttypes>

namespace WarGrey::SCADA {
    ref class ModbusListener;

    private class IModbusServer {
    public:
        IModbusServer(uint16 port);
        virtual ~IModbusServer() noexcept {};

    public:
        void listen();
        int process(uint8 function_code, Windows::Storage::Streams::DataReader^ mbin, uint8 *response);
        void enable_debug(bool on_or_off);
        bool debug_enabled();

    public: // data access
        virtual int read_coils(uint16 address, uint16 quantity, uint8* coil_status) = 0;
		virtual int read_discrete_inputs(uint16 address, uint16 quantity, uint8* input_status) = 0;
        virtual int write_coil(uint16 address, bool value) = 0;
        virtual int write_coils(uint16 address, uint16 quantity, uint8* src) = 0;

		virtual int read_holding_registers(uint16 address, uint16 quantity, uint8* register_values) = 0;
		virtual int read_input_registers(uint16 address, uint16 quantity, uint8* input_registers) = 0;
		virtual int write_register(uint16 address, uint16 value) = 0;
		virtual int write_registers(uint16 address, uint16 quantity, uint8* src) = 0;

		virtual int mask_write_register(uint16 address, uint16 and, uint16 or) = 0;
		virtual int write_read_registers(uint16 waddr, uint16 wquantity, uint16 raddr, uint16 rquantity, uint8* rwpool) = 0;

    public: // Diagnostics

    public: // Other
        virtual int do_private_function(uint8 function_code, uint8* request, uint16 request_data_length, uint8* response);

    protected:
        bool debug;

    private:
        WarGrey::SCADA::ModbusListener^ listener;
    };
}
