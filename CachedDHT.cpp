#include <CachedDHT.h>

#define SIZEOF_ARRAY( a ) (sizeof( a ) / sizeof( a[ 0 ] ))

static bool TimerRegistered = false;
static CachedDHT* dht_array[MAX_DHT_COUNT]; //initialized in Register

void UpdateAll(Task* me) {
	for (byte i = 0; i < SIZEOF_ARRAY(dht_array); ++i) {
		if (dht_array[i] != NULL)
			dht_array[i]->UpdateValues();
	}
}

static Task UpdateAllTask(DEFAULT_UPDATE_TIME_MS, UpdateAll);

void SetUpdateTime(unsigned long periodMs) {
	UpdateAllTask.setPeriodMs(periodMs);
}

void UnRegister(CachedDHT * to_unreg) {
	for (byte i = 0; i < SIZEOF_ARRAY(dht_array); ++i) {
		if (dht_array[i] == to_unreg)
			dht_array[i] = NULL;
	}
}

void Register(CachedDHT * to_reg) {
	if (!TimerRegistered) {
		SoftTimer.add(&UpdateAllTask);
		for (byte i = 0; i < SIZEOF_ARRAY(dht_array); ++i)
			dht_array[i] = NULL;

		TimerRegistered = true;
	}

	for (byte i = 0; i < SIZEOF_ARRAY(dht_array); ++i) {
		if (dht_array[i] == to_reg)
			return; //already registered
	}

	for (byte i = 0; i < SIZEOF_ARRAY(dht_array); ++i) {
		if (dht_array[i] == NULL) {
			dht_array[i] = to_reg;
			return;
		}
	}
}

CachedDHT::CachedDHT(uint8_t pin, uint8_t type)
	: m_dht_data(pin, type)
	, m_dht_start(millis() + DHT_WARMUP_DELAY_MS)
	, m_upstarted(false)
	, m_is_normalized(false)
	, m_current_normal(0)
	, m_current_temp(NAN)
	, m_current_hum(NAN) {
		m_dht_data.begin();
		Register(this);
	}

CachedDHT::~CachedDHT() {
		UnRegister(this);
	}

	float CachedDHT::GetTemperature() const {
		return m_current_temp;
	}

	float CachedDHT::GetHumidity() const {
		return m_current_hum;
	}

	bool CachedDHT::HasValues() const {
		return m_is_normalized;
	}

	void CachedDHT::UpdateValues() {
		if (!m_upstarted) {
			if (millis() < m_dht_start)
				return;
			m_upstarted = true;
		}

		float ict = m_dht_data.readTemperature();
		float ich = m_dht_data.readHumidity();

		if (!isnan(ict) && !isnan(ich))
			NormalizeData(ict, ich);
	}

	void CachedDHT::NormalizeData(float ict, float ich) {
		if (!m_is_normalized) {
			m_current_temp = ict;
			m_current_hum = ich;
			for (int i = 0; i < NORMALIZE_SIZE; ++i) {
				m_temp_vector[i] = ict;
				m_hum_vector[i] = ich;
			}
			m_current_normal = 0;
			m_is_normalized = true;
			return;
		}
		++m_current_normal;
		if (m_current_normal == NORMALIZE_SIZE)
			m_current_normal = 0;
		m_temp_vector[m_current_normal] = ict;
		m_hum_vector[m_current_normal] = ich;
		float temp_res = 0;
		float hum_res = 0;
		for (int i = 0; i < NORMALIZE_SIZE; ++i) {
			temp_res += m_temp_vector[i];
			hum_res += m_hum_vector[i];
		}

		m_current_temp = temp_res / NORMALIZE_SIZE;
		m_current_hum = hum_res / NORMALIZE_SIZE;
	}
