#include "modbus/constants.hpp"
#include "modbus/server.hpp"
#include "modbus/protocol.hpp"
#include "modbus/exception.hpp"
#include "rsyslog.hpp"

#include <ppltasks.h>

using namespace WarGrey::SCADA;

using namespace Concurrency;
using namespace Windows::Foundation;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;

static inline Platform::String^ socket_identity(StreamSocket^ socket) {
    return socket->Information->RemoteHostName->RawName + ":" + socket->Information->RemotePort;
}

static void modbus_process_loop(IModbusServer* server, DataReader^ mbin, DataWriter^ mbout, uint8* response
    , StreamSocket^ client, Platform::String^ id) {
    uint16 header_length = (uint16)(2 + 2 + 2 + 1); // MMIG page 5
    create_task(mbin->LoadAsync(header_length)).then([=](unsigned int size) {
        uint16 transaction, protocol, length;
        uint8 unit;

        if (size < header_length) {
            if (size == 0) {
                rsyslog(L"%s has disconnected", id->Data());
            } else {
                rsyslog(L"MBAP header from %s is too short(%u < %hu)", id->Data(), size, header_length);
            }

            cancel_current_task();
        }

        uint16 pdu_length = modbus_read_mbap(mbin, &transaction, &protocol, &length, &unit);

        return create_task(mbin->LoadAsync(pdu_length)).then([=](unsigned int size) {
            if (size < pdu_length) {
                rsyslog(L"PDU data from %s has been truncated(%u < %hu)", id->Data(), size, pdu_length);
                cancel_current_task();
            }

            uint8 function_code = mbin->ReadByte();

			if (server->debug_enabled()) {
				rsyslog(L"[received ADU indication(%hu, %hu, %hu, %hhu, %hhu) from %s]",
					transaction, protocol, length, unit, function_code, id->Data());
			}

            int retcode = server->process(function_code, mbin, response);
            
            if (retcode >= 0) {
				modbus_write_adu(mbout, transaction, protocol, unit, function_code, response, retcode);                
            } else {
				modbus_write_exn_adu(mbout, transaction, protocol, unit, function_code, (uint8)(-retcode));
            }
        });
    }).then([=](task<void> doHandlingRequest) {
        try {
            doHandlingRequest.get();

            return create_task(mbout->StoreAsync());
        } catch (Platform::Exception^ e) {
            rsyslog(e->Message);
            cancel_current_task();
        } catch (task_canceled&) {
            rsyslog(L"Cancel dealing with request from %s", id->Data());
            cancel_current_task();
        }
    }).then([=](task<unsigned int> doReplying) {
        try {
            unsigned int sent = doReplying.get();

            if (server->debug_enabled()) {
                rsyslog(L"[sent %u bytes to %s]", sent, id->Data());
            }

            modbus_process_loop(server, mbin, mbout, response, client, id);
        } catch (Platform::Exception^ e) {
            rsyslog(e->Message);
            delete client;
        } catch (task_canceled&) {
            rsyslog(L"Cancel replying to %s", id->Data());
            delete client;
        }
    });
}

static inline int modbus_echo(uint8* response, uint16 address, uint16 value) {
	MODBUS_SET_INT16_TO_INT8(response, 0, address);
	MODBUS_SET_INT16_TO_INT8(response, 2, value);
	return 4;
}

// delegate only accepts C++/CX class
private ref class WarGrey::SCADA::ModbusListener sealed {
internal:
    ModbusListener(IModbusServer* server, uint16 port) : server(server), port(port) {
        this->listener = ref new StreamSocketListener();
        this->listener->Control->QualityOfService = SocketQualityOfService::LowLatency;
        this->listener->Control->KeepAlive = false;
    }

public:
    void run() {
        create_task(this->listener->BindEndpointAsync(nullptr, this->port.ToString())).then([this](task<void> binding) {
            try {
                binding.get();
                this->listener->ConnectionReceived
                    += ref new TypedEventHandler<StreamSocketListener^, StreamSocketListenerConnectionReceivedEventArgs^>(
                        this,
                        &ModbusListener::welcome);

                rsyslog(L"## 0.0.0.0:%u", this->port);
            } catch (Platform::Exception^ e) {
                rsyslog(e->Message);
            }
        });
    }

    void welcome(StreamSocketListener^ listener, StreamSocketListenerConnectionReceivedEventArgs^ e) {
        auto client = e->Socket;
        auto id = socket_identity(client);
        auto mbin = ref new DataReader(client->InputStream);
        auto mbout = ref new DataWriter(client->OutputStream);
        uint8 response_pdu[MODBUS_TCP_MAX_ADU_LENGTH];

        mbin->UnicodeEncoding = UnicodeEncoding::Utf8;
        mbin->ByteOrder = ByteOrder::BigEndian;
        mbout->UnicodeEncoding = UnicodeEncoding::Utf8;
        mbout->ByteOrder = ByteOrder::BigEndian;

        modbus_process_loop(this->server, mbin, mbout, response_pdu, client, id);
    }

private:
    uint16 port;
    StreamSocketListener^ listener;
    IModbusServer* server;
};

/*************************************************************************************************/
IModbusServer::IModbusServer(uint16 port) {
    this->listener = ref new ModbusListener(this, port);
    this->debug = false;
};

void IModbusServer::listen() {
    this->listener->run();
}

void IModbusServer::enable_debug(bool on_or_off) {
    this->debug = on_or_off;
}

bool IModbusServer::debug_enabled() {
    return this->debug;
}

int IModbusServer::process(uint8 funcode, DataReader^ mbin, uint8 *response) { // MAP: Page 10
	int retcode = -MODBUS_EXN_DEVICE_FAILURE;

    switch (funcode) {
	case MODBUS_READ_COILS: case MODBUS_READ_DISCRETE_INPUTS: { // MAP: Page 12, 13
        uint16 address = mbin->ReadUInt16();
		uint16 quantity = mbin->ReadUInt16();

        if ((quantity < 0x01) || (quantity > MODBUS_MAX_READ_BITS)) {
            retcode = -modbus_illegal_data_value(quantity, 0x01, MODBUS_MAX_READ_BITS, this->debug);
		} else if (funcode == MODBUS_READ_COILS) {
			retcode = this->read_coils(address, quantity, response + 1);
		} else {
			retcode = this->read_discrete_inputs(address, quantity, response + 1);
		}
			
		if (retcode >= 0) {
			response[0] = (uint8)retcode;
			retcode += 1;
		}
	}; break;
    case MODBUS_WRITE_SINGLE_COIL: { // MAP: Page 17
        uint16 address = mbin->ReadUInt16();
        uint16 value = mbin->ReadUInt16();
        
        switch (value) {
		case 0x0000: retcode = this->write_coil(address, false); break;
		case 0xFF00: retcode = this->write_coil(address, true); break;
		default: retcode = -modbus_illegal_bool_value(value, 0xFF00, 0x0000, this->debug); break;
        }

        if (retcode >= 0) {
            retcode = modbus_echo(response, address, value);
        }
	}; break;
    case MODBUS_WRITE_MULTIPLE_COILS: { // MAP: Page 29
		uint16 address = mbin->ReadUInt16();
        uint16 quantity = mbin->ReadUInt16();
        uint8 count = mbin->ReadByte();
        
		MODBUS_READ_BYTES(mbin, response, count);
		
        if ((quantity < 0x01) || (quantity > MODBUS_MAX_WRITE_BITS)) {
            retcode = -modbus_illegal_data_value(quantity, 0x01, MODBUS_MAX_WRITE_BITS, this->debug);
        } else if (count != MODBUS_NStar(quantity)) {
			retcode = -modbus_illegal_data_value(count, MODBUS_NStar(quantity), this->debug);
		} else {
			retcode = this->write_coils(address, quantity, response);
		}

		if (retcode >= 0) {
			retcode = modbus_echo(response, address, quantity);
		}
	}; break;
    default: {
        uint8 request[MODBUS_TCP_MAX_ADU_LENGTH];
        int data_length = mbin->UnconsumedBufferLength;

        MODBUS_READ_BYTES(mbin, request, data_length);

        retcode = this->do_private_function(funcode, request, data_length, response);
    };
    }

	return retcode;
}

int IModbusServer::do_private_function(uint8 function_code, uint8* request, uint16 data_length, uint8* response) { // MAP: Page 10
    return -modbus_illegal_function(function_code, this->debug);
}
