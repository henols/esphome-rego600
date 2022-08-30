#include "esphome.h"

#define REGO_600 0x81

// 1 - address 5char 16bit number Read from front panel (keyboard+leds) {reg09FF+xx}
#define PANEL_R 0x00
// 2 - address + data 1char confirm Write to front panel (keyboard+leds) {reg 09FF+xx}
#define PANEL_W 0x01
// 1 - address 5char 16bit number Read from system register (heat curve, temperatures, devices) {reg 1345+xx}
#define REGISTER_R 0x02
// 2 - address + data 1char confirm Write into system register (heat curve, temperatures, devices) {reg 1345+xx}
#define REGISTER_W 0x03
// 1 - address 5char 16bit number Read from timer registers {reg 1B45+xx}
#define TIMER_R 0x04
// 2 - address + data 1char confirm Write into timer registers {reg 1B45+xx}
#define TIMER_W 0x05
// 1 - address 5char 16bit number Read from register 1B61 {reg 1B61+xx}
#define REGISTER_HI_R 0x06
// 2 - address + data 1char confirm Write into register 1B61 {1B61+xx}
#define REGISTER_HI_W 0x07
// 1 - display line 42char text line Read from display {0AC7+15h*xx}
#define DISPLAY_R 0x20
// 0 42char text line Read last error line [4100/00]
#define ERROR_LAST 0x40
// 0 42char text line Read previous error line (prev from last reading) [4100/01]
#define ERROR_PREVIUS 0x42
// 0 5char 16bit number Read rego version {constant 0258 600 ?Rego 600?}

#define VERSION 0x7F

// Rego636-... Register
// Sensor values
#define RADIATOR_RETURN_GT1 0x020B	   // Radiator return[GT1]
#define OUTDOOR_TEMP_GT2 0x020C		   // Outdoor [GT2]
#define HOT_WATER_GT3 0x020D		   // Hot water [GT3]
#define FORWARD_GT4 0x020E			   // Forward [GT4]
#define ROOM_GT5 0x020F				   // Room [GT5]
#define COMPRESSOR_GT6 0x0210		   // Compressor [GT6]
#define HEAT_FLUID_OUT_GT8 0x0211	   // Heat fluid out [GT8]
#define HEAT_FLUID_IN_GT9 0x0212	   // Heat fluid in [GT9]
#define TRANSFER_FLUID_IN_GT10 0x0213  // Cold fluid in [GT10]
#define TRANSFER_FLUID_OUT_GT11 0x0214 // Cold fluid out [GT11]
#define HOT_WATER_EXTERNAL_GT3X 0x0215 // External hot water [GT3x]

// Device values
#define GROUND_LOOP_PUMP_P3 0x01FF	  // Ground loop pump [P3]
#define COMPRESSOR 0x0200			  // Compressor
#define ADDITIONAL_HEAT_STEP_1 0x0201 // Additional heat 3kW
#define ADDITIONAL_HEAT_STEP_2 0x0202 // Additional heat 6kW
#define RADIATOR_PUMP_P1 0x0205		  // Radiator pump [P1]
#define HEAT_CARRIER_PUMP_P2 0x0206	  // Heat carrier pump [P2]
#define THREE_WAY_VALVE 0x0207		  // Tree-way valve [VXV]
#define ALARM 0x0208				  // Alarm

// Control data
#define ADDED_HEAT_POWER 0x006C // Add heat power in %
#define GT4_TARGET_VALUE 0x006D // GT4 Target value
#define GT1_TARGET_VALUE 0x006E // GT1 Target value
#define GT1_ON_VALUE 0x006F		// GT1 On value
#define GT1_OFF_VALUE 0x0070	// GT1 Off value
#define GT3_TARGET_VALUE 0x002B // GT3 Target value
#define GT3_ON_VALUE 0x0073		// GT3 On value
#define GT3_OFF_VALUE 0x0074	// GT3 Off value

// Settings
#define HEAT_CURVE 0x0000				// Heat curve
#define HEAT_CURVE_FINE 0x0001			// Heat curve fine adj.
#define HEAT_CURVE_COUPLING_DIFF 0x0002 // Heat curve coupling diff.
#define ADJ_CURVE_AT_N35_DGR_OUT 0x0008 // Adj. curve at -35C out
#define ADJ_CURVE_AT_N30_DGR_OUT 0x000A // Adj. curve at -30C out
#define ADJ_CURVE_AT_N25_DGR_OUT 0x000C // Adj. curve at -25C out
#define ADJ_CURVE_AT_N20_DGR_OUT 0x000E // Adj. curve at -20C out
#define ADJ_CURVE_AT_N15_DGR_OUT 0x0010 // Adj. curve at -15C out
#define ADJ_CURVE_AT_N10_DGR_OUT 0x0012 // Adj. curve at -10C out
#define ADJ_CURVE_AT_N5_DGR_OUT 0x0014	// Adj. curve at -5C out
#define ADJ_CURVE_AT_0_DGR_OUT 0x0016	// Adj. curve at 0C out
#define ADJ_CURVE_AT_5_DGR_OUT 0x0018	// Adj. curve at 5C out
#define ADJ_CURVE_AT_10_DGR_OUT 0x001A	// Adj. curve at 10C out
#define ADJ_CURVE_AT_15_DGR_OUT 0x001C	// Adj. curve at 15C out
#define ADJ_CURVE_AT_20_DGR_OUT 0x001E	// Adj. curve at 20C out
#define INDOOR_TEMP_SETTING 0x0021		// Indoor temp setting
#define CURVE_INFL_IN_TEMP 0x0022		// Curve infl. by in-temp.

static const char *TAG = "Rego600";

class Rego600 : public PollingComponent, public UARTDevice
{

protected:
	static Rego600 *instance;

	byte request[9];

	byte shortResult[5];
	byte longResult[42];
	byte textResult[20];

public:
	Sensor *radiatorReturn = new Sensor();
	Sensor *outdoor = new Sensor();
	Sensor *hotWater = new Sensor();
	Sensor *forward = new Sensor();
	Sensor *room = new Sensor();
	Sensor *compressorTemp = new Sensor();
	Sensor *heatFluidOut = new Sensor();
	Sensor *heatFluidIn = new Sensor();
	Sensor *coldFluidIn = new Sensor();
	Sensor *coldFluidOut = new Sensor();
	Sensor *externalHotWater = new Sensor();

	Sensor *heat_fluid_pump_control = new Sensor();

	BinarySensor *heat_carrier_pump = new BinarySensor();
	BinarySensor *heat_fluid_pump = new BinarySensor();
	BinarySensor *three_way_valve = new BinarySensor();
	BinarySensor *additional_heat = new BinarySensor();
	BinarySensor *compressor = new BinarySensor();
	BinarySensor *cold_fluid_pump = new BinarySensor();

	Rego600(UARTComponent *parent) : PollingComponent(30000), UARTDevice(parent) {}

	static Rego600 *getInstance()
	{
		return instance;
	}
	static Rego600 *createInstance(UARTComponent *parent)
	{
		if (!instance)
		{
			instance = new Rego600(parent);
		}
		return instance;
	}

	void setup() override
	{
		ESP_LOGD(TAG, "Starting Rego600\n");
	}

	void update() override
	{
		publish_deg_temperature(RADIATOR_RETURN_GT1, getRegisterValue(RADIATOR_RETURN_GT1), shortResult, radiatorReturn);
		publish_deg_temperature(OUTDOOR_TEMP_GT2, getRegisterValue(OUTDOOR_TEMP_GT2), shortResult, outdoor);
		publish_deg_temperature(HOT_WATER_GT3, getRegisterValue(HOT_WATER_GT3), shortResult, hotWater);
		publish_deg_temperature(FORWARD_GT4, getRegisterValue(FORWARD_GT4), shortResult, forward);
		publish_deg_temperature(ROOM_GT5, getRegisterValue(ROOM_GT5), shortResult, room);
		publish_deg_temperature(COMPRESSOR_GT6, getRegisterValue(COMPRESSOR_GT6), shortResult, compressorTemp);
		publish_deg_temperature(HEAT_FLUID_OUT_GT8, getRegisterValue(HEAT_FLUID_OUT_GT8), shortResult, heatFluidOut);
		publish_deg_temperature(HEAT_FLUID_IN_GT9, getRegisterValue(HEAT_FLUID_IN_GT9), shortResult, heatFluidIn);
		publish_deg_temperature(TRANSFER_FLUID_IN_GT10, getRegisterValue(TRANSFER_FLUID_IN_GT10), shortResult, coldFluidIn);
		publish_deg_temperature(TRANSFER_FLUID_OUT_GT11, getRegisterValue(TRANSFER_FLUID_OUT_GT11), shortResult, coldFluidOut);
		publish_deg_temperature(HOT_WATER_EXTERNAL_GT3X, getRegisterValue(HOT_WATER_EXTERNAL_GT3X), shortResult, externalHotWater);
	}

	void buildCmd(byte addr, byte cmd, int16_t reg, int16_t val, byte *request)
	{
		*request = addr;
		*(request + 1) = cmd;
		*(request + 8) = 0; // XOR
		int2write(reg, request + 2);
		int2write(val, request + 5);
		for (int i = 2; i < 8; i++)
		{
			*(request + 8) ^= *(request + i); // update XOR with data
		}
	}

	void sendCmd(byte *request)
	{
		for (int i = 0; i < 9; i++)
		{
			Serial.write(*(request + i)); // send data to port
		}
	}

	int getRegisterValue(uint16_t reg)
	{
		uint8_t *request = buildRequest(REGO_600, REGISTER_R, reg, 00);
		uint16_t res = sendRead(request, shortResult, 5);
		ESP_LOGD(TAG, "Got value from read %d", res);
		return res;
	}

	// char *getDisplayLine(uint8_t line)
	// {
	// 	return getText(REGO_600, DISPLAY_R, line);
	// }

	// char *getText(uint8_t address, uint8_t command, uint16_t reg)
	// {
	// 	char *result = getLongData(address, command, reg);
	// 	return result2text(result);
	// }

	// char *getLongData(uint8_t address, uint8_t command, uint16_t reg)
	// {
	// 	uint8_t *request = buildRequest(address, command, reg, 00);
	// 	sendRead(request, longResult);
	// 	return longResult;
	// }

	uint8_t *buildRequest(uint8_t address, uint8_t command, uint16_t reg, int value)
	{
		buildCmd(address, command, reg, value, request);
		return request;
	}

	int readResponse(int max, int timeout, byte *response)
	{
		int TO = 0;			  // timeout value
		int cnt = 0;		  // count of received characters
		boolean done = false; // are we done with reading?
		do
		{
			if (Serial.available() > 0)
			{
				// read the incoming byte:
				*(response + cnt) = Serial.read();
				if (++cnt == max)
				{
					done = true;
				}
			}
			else
			{
				delay(10);
				if (++TO > timeout)
				{
					done = true;
				}
			}
		} while (!done);
		return cnt;
	}

	int readResponse(byte *result, int resLen)
	{
		int len = readResponse(resLen, 4, result);
		if (len != resLen)
		{
			ESP_LOGE(TAG, "Read length are different from expected (%d), got %d", resLen, len);
			return 0x8001;
		}
		if (*result == 0x01)
		{
			ESP_LOGE(TAG, "Bad checksum");
			return 0x8002;
		}
		if ((result[4] != ((result[1]) ^ (result[2]) ^ (result[3]))))
		{
			ESP_LOGE(TAG, "Checksums dont match, expected %04X, clculated %04X");
			return 0x8003;
		}
		return len;
	}

	// char *result2text(char *result)
	// {
	// 	for (int i = 1; i <= 20; i++)
	// 	{
	// 		char c = (result[(i * 2)] << 4) + result[1 + (i * 2)];
	// 		textResult[i] = c;
	// 	}
	// 	return textResult;
	// }

	int sendRead(uint8_t *request, byte *result, int resLen)
	{
		sendRequest(request);
		return readResponse(result, resLen);
	}

	void cmdsend(byte *request)
	{
		for (int i = 0; i < 9; i++)
		{
			Serial.write(request[i]);
		}
	}
	void sendRequest(uint8_t *data)
	{
		Serial.write(data, 9);
	}

private:
	void publish_deg_temperature(uint32_t msg_id, int res, byte *data, Sensor *sensor)
	{
		ESP_LOGD(TAG, "Publish temperature reg 0x%04X, result 0x%04X", msg_id, res);
		if (res >= 0x8000 && res < 0x8008)
		{
			ESP_LOGE(TAG, "Read error 0x%04X, for reg 0x%04X", res, msg_id);
			return;
		}
		int16_t temperature_val = read2int(data);
		float temperature = (float)temperature_val / 10.f;
		sensor->publish_state(temperature);
	}

	void publish_u8(uint32_t msg_id, int res, uint8_t *data, Sensor *sensor)
	{
		ESP_LOGD(TAG, "Publish u8 reg 0x%04X, result 0x%04X", msg_id, res);
		if (res >= 0x8000 && res < 0x8008)
		{
			ESP_LOGE(TAG, "Read error 0x%04X, for reg 0x%04X", res, msg_id);
			return;
		}
		sensor->publish_state(data[0]);
	}

	void publish_binary(uint32_t msg_id, int res, uint8_t *data, BinarySensor *sensor)
	{
		ESP_LOGD(TAG, "Publish binary reg 0x%04X, result 0x%04X", msg_id, res);
		if (res >= 0x8000 && res < 0x8008)
		{
			ESP_LOGE(TAG, "Read error 0x%04X, for reg 0x%04X", res, msg_id);
			return;
		}
		sensor->publish_state(!!data[0]);
	}

	int16_t read2int(byte *array) // convert received values (array) to integer
	{
		return ((int16_t) * (array + 1) << 14) | ((int16_t) * (array + 2) << 7) | (int16_t) * (array + 3);
	}

	void int2write(int16_t val, byte *array) // convert int (16 bit) to array for sending
	{
		*(array + 2) = (byte)val & 0x7F;
		*(array + 1) = (byte)(val >> 7) & 0x7F;
		*array = (byte)(val >> 14) & 0x03;
	}
};

Rego600 *Rego600::instance = 0;
