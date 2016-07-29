#ifndef _CachedDHT_H_
#define _CachedDHT_H_

#include <Arduino.h>
#include <DHT.h>	// USE https://github.com/adafruit/DHT-sensor-library
#include <SoftTimer.h>	// USE https://github.com/prampec/arduino-softtimer/

#define NORMALIZE_SIZE 10	//Temperature or humidity normalize vector size
#define MAX_DHT_COUNT 2		//Maximum count of attached DHTs

#define DHT_WARMUP_DELAY_MS 4000	//DHT warmup delay (ms) (Datasheet says not to get values from dht during some period)

#define DEFAULT_UPDATE_TIME_MS 2000	//DHT data default update time

//You may change update time by setting it in milliseconds
void SetUpdateTime(unsigned long periodMs);

class CachedDHT {
public:
	CachedDHT(uint8_t pin, uint8_t type);
	~CachedDHT();

	float GetTemperature() const;
	float GetHumidity() const;

	bool HasValues() const;

	void UpdateValues();
private:
	DHT m_dht_data;

	unsigned long m_dht_start;
	bool m_upstarted;
	bool m_is_normalized;
	byte m_current_normal;

	float m_current_temp;
	float m_current_hum;
	float m_temp_vector[NORMALIZE_SIZE];
	float m_hum_vector[NORMALIZE_SIZE];

	void NormalizeData(float ict, float ich);
};

#endif /* _CachedDHT_H_ */
