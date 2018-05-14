#include "isd9160.h"
#include "stm32l4xx_hal.h"
#include "k_api.h"
#include <string.h>

#define ISD9160_I2C_ADDR                (0x15 << 1)
#define ISD9160_I2C_TIMEOUT             AOS_WAIT_FOREVER
#define ISD9160_ROM_SIZE                0x24400
#define UPG_FRAME_HEAD_SIZE             sizeof(UPG_FRAME_HEAD)
#define UPG_PAYLOAD_HAED_SIZE           sizeof(UPG_PAYLOAD_HEAD)
#define UPG_PAYLOAD_DATA_SIZE           128
#define SLAVE_DATA_MAX                  (UPG_PAYLOAD_HAED_SIZE + UPG_PAYLOAD_DATA_SIZE)
#define UPG_FRAME_MAGIC                 0x18
#define UPG_HINT_GRANU                  5
#define UPG_HINT_DIV                    (100 / UPG_HINT_GRANU)

#define MCU_FLASH_START_ADDR            0x8000000
#define UPG_FLASH_BASE                  ((256 << 10) + MCU_FLASH_START_ADDR)
#define FLASH_MAGIC_HEAD                "9160"
#define FLASH_MAGIC_FILE                0x20180511

typedef enum {
	I2C_CMD_SLPRT = 0,
	I2C_CMD_UPGRADE,
	I2C_CMD_RESPONSE,
	I2C_CMD_RECORD,
	I2C_CMD_PLAYBACK,
	
	I2C_CMD_END
} I2C_PROC_CMD;

typedef enum {
	RESP_UPG_ALL_SUCCESS = 0xa8,
	RESP_UPG_ALL_FAILED,
	RESP_UPG_BINSIZE_SUCCESS,
	RESP_UPG_BINSIZE_FAILED,
	RESP_UPG_FRAMEHEAD_SUCCESS,
	RESP_UPG_FRAMEHEAD_FAILED,
	RESP_UPG_PAYLOAD_SUCCESS,
	RESP_UPG_PAYLOAD_FAILED,
	RESP_UPG_PAYLOAD_CRC,
	
	RESP_LB_OPENED,
	RESP_LB_CLOSED,
	RESP_LB_FAILED,
	
	RESP_FLAG_NORMAL
} RESPONSE_FLAG;

typedef enum {
	BINTYPE_LDROM = 0,
	BINTYPE_APROM,
	BINTYPE_DATAROM,

	BINTYPE_END
} BINTYPE_UPG;

typedef struct {
	uint32_t addr;
	uint32_t size;
} UPG_FLASH_MAP;

#pragma pack(1)
typedef struct {
	uint8_t magic;
	uint8_t payload_size;
} UPG_FRAME_HEAD;

typedef struct {
	uint16_t crc;
	uint32_t offset;
} UPG_PAYLOAD_HEAD;

typedef struct {
	uint32_t magic_head;
	uint32_t file_num;
	uint8_t version[8];
} FLASH_UPG_HEAD;

typedef struct {
	uint32_t magic_file;
	uint32_t bintype;
	uint32_t size;
	uint32_t reserve;
} FLASH_FILE_HEAD;
#pragma pack()

static const UPG_FLASH_MAP g_isd9160_map_table[BINTYPE_END] = {
	{0x00100000, 4  << 10},
	{0x00000000, 60 << 10},
	{0x0000f000, 81 << 10},
};

static FLASH_UPG_HEAD g_upg_head;
static FLASH_FILE_HEAD g_file_head[BINTYPE_END];
static uint32_t g_bin_addr[BINTYPE_END];
static aos_mutex_t isd9160_mutex;

static inline void hton_4(uint8_t *data, uint32_t value)
{
	data[0] = (value >> (8 * 3)) & 0xff;
	data[1] = (value >> (8 * 2)) & 0xff;
	data[2] = (value >> (8 * 1)) & 0xff;
	data[3] = (value >> (8 * 0)) & 0xff;
}

static inline void ntoh_4(const uint8_t *data, uint32_t *value)
{
	*value = 0;
	*value |= data[0] << (8 * 3);
	*value |= data[1] << (8 * 2);
	*value |= data[2] << (8 * 1);
	*value |= data[3] << (8 * 0);
}

static int upgf_init(void)
{
	uint32_t bin_size_total = 0;
	uint32_t flash_offset = 0;
	int i;

	g_upg_head = *(__IO FLASH_UPG_HEAD *)(UPG_FLASH_BASE + flash_offset);
	flash_offset += sizeof(FLASH_UPG_HEAD);
	if (memcmp(&g_upg_head.magic_head, FLASH_MAGIC_HEAD, 4)) {
		KIDS_A10_PRT("magic_head is invalid.\n");
		return -1;
	}
	if (g_upg_head.file_num > BINTYPE_END) {
		KIDS_A10_PRT("file_num is invalid.\n");
		return -1;
	}
	
	for (i = 0; i < g_upg_head.file_num; ++i) {
		g_file_head[i] = *(__IO FLASH_FILE_HEAD *)(UPG_FLASH_BASE + flash_offset);
		flash_offset += sizeof(FLASH_FILE_HEAD);
		if (g_file_head[i].magic_file != FLASH_MAGIC_FILE) {
			KIDS_A10_PRT("file %d magic_file is invalid.\n", i);
			return -1;
		}
		if (g_file_head[i].bintype >= BINTYPE_END) {
			KIDS_A10_PRT("file %d bintype is invalid.\n", i);
			return -1;
		}
		g_bin_addr[i] = UPG_FLASH_BASE + flash_offset;
		bin_size_total += g_file_head[i].size;
		flash_offset += g_file_head[i].size;
	}
	if (bin_size_total > ISD9160_ROM_SIZE) {
		KIDS_A10_PRT("bin_size_total exceeds ISD9160_ROM_SIZE.\n");
		return -1;
	}
	
	return 0;
}

static int isd9160_slprt_size(uint32_t *size)
{
	int ret = 0;
	uint8_t data = I2C_CMD_SLPRT;
	uint8_t size_data[4] = {0};
	
	ret = hal_i2c_master_send(&brd_i2c4_dev, ISD9160_I2C_ADDR, &data, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		return ret;
	}
	
	ret = hal_i2c_master_recv(&brd_i2c4_dev, ISD9160_I2C_ADDR, size_data, 4, HAL_MAX_DELAY);
	if (ret != 0) {
		return ret;
	}
	
	ntoh_4(size_data, size);
	
	return 0;
}

static int isd9160_slprt_data(uint8_t *data, uint32_t size)
{
	return hal_i2c_master_recv(&brd_i2c4_dev, ISD9160_I2C_ADDR, data, size, HAL_MAX_DELAY);
}

static int handle_slprt(void)
{
	int ret = 0;
	uint32_t size = 0;
	int slprt_num = 0;
	char buf[SLAVE_DATA_MAX] = {0};
	
	if (!aos_mutex_is_valid(&isd9160_mutex)) {
		KIDS_A10_PRT("isd9160_mutex is invalid.\n");
		return -1;
	}
	ret = aos_mutex_lock(&isd9160_mutex, ISD9160_I2C_TIMEOUT);
	if (ret != 0) {
		KIDS_A10_PRT("ISD9160 is very busy now.\n");
		return -1;
	}
	
	while (1) {
		ret = isd9160_slprt_size(&size);
		if (ret != 0) {
			KIDS_A10_PRT("isd9160_slprt_size return failed.\n");
			ret = -1;
			break;
		}
		if (size == 0) {
			ret = slprt_num;
			break;
		}
		memset(buf, 0, sizeof(buf));
		ret = isd9160_slprt_data((uint8_t *)buf, size);
		if (ret != 0) {
			KIDS_A10_PRT("isd9160_slprt_data return failed.\n");
			ret = -1;
			break;
		}
		printf("slave_print: ");
		printf("%s", buf);
		++slprt_num;
	}
	
	ret = aos_mutex_unlock(&isd9160_mutex);
	if (ret != 0) {
		KIDS_A10_PRT("ISD9160 release failed.\n");
	}
	
	return ret;
}

static int get_upgresp(uint8_t *resp)
{
	uint8_t data = I2C_CMD_RESPONSE;
	int ret = 0;
	
	ret = hal_i2c_master_send(&brd_i2c4_dev, ISD9160_I2C_ADDR, &data, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		return ret;
	}
	
	return hal_i2c_master_recv(&brd_i2c4_dev, ISD9160_I2C_ADDR, resp, 1, HAL_MAX_DELAY);
}

static int request_binsize(uint8_t *resp, uint32_t type, uint32_t size)
{
	uint8_t data[5] = {0};
	int ret = 0;
	
	data[0] = I2C_CMD_UPGRADE;
	ret = hal_i2c_master_send(&brd_i2c4_dev, ISD9160_I2C_ADDR, data, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		return ret;
	}
	data[0] = (uint8_t)type;
	hton_4(&data[1], size);
	ret = hal_i2c_master_send(&brd_i2c4_dev, ISD9160_I2C_ADDR, data, 5, HAL_MAX_DELAY);
	if (ret != 0) {
		return ret;
	}
	ret = get_upgresp(resp);
	if (ret != 0) {
		return ret;
	}
	
	return 0;
}

static int request_framehead(uint8_t *resp, uint8_t fsize)
{
	uint8_t data[2] = {0};
	int ret = 0;
	
	data[0] = UPG_FRAME_MAGIC;
	data[1] = fsize;
	ret = hal_i2c_master_send(&brd_i2c4_dev, ISD9160_I2C_ADDR, data, 2, HAL_MAX_DELAY);
	if (ret != 0) {
		return ret;
	}
	ret = get_upgresp(resp);
	if (ret != 0) {
		return ret;
	}
	
	return 0;
}

static int request_payload(uint8_t *resp, uint32_t bintype, uint32_t bin_addr, uint32_t offset, uint32_t size)
{
	uint32_t isd9160_addr = 0;
	uint8_t data[SLAVE_DATA_MAX] = {0};
	int ret = 0;
	
	if (size > UPG_PAYLOAD_DATA_SIZE || offset + size > g_isd9160_map_table[bintype].size) {
		return -1;
	}
	
	/* calc crc checksum */
	
	isd9160_addr = g_isd9160_map_table[bintype].addr + offset;
	hton_4(&data[2], isd9160_addr);
	memcpy(&data[UPG_PAYLOAD_HAED_SIZE], (void *)(bin_addr + offset), size);
	ret = hal_i2c_master_send(&brd_i2c4_dev, ISD9160_I2C_ADDR, data, UPG_PAYLOAD_HAED_SIZE + size, HAL_MAX_DELAY);
	if (ret != 0) {
		return ret;
	}
	ret = get_upgresp(resp);
	if (ret != 0) {
		return ret;
	}
	
	return 0;
}

static void hint_percent(uint32_t now_bytes, uint32_t total_bytes)
{
	static uint32_t last_quotient = 0;
	uint32_t now_quotient = 0;
	
	if (now_bytes == 0 || total_bytes == 0) {
		printf("isd9160 upgrade current progress is 0%%\n");
		last_quotient = 0;
		return;
	}
	now_quotient = now_bytes * UPG_HINT_DIV / total_bytes;
	if (now_quotient > last_quotient) {
		printf("isd9160 upgrade current progress is %u%%\n", now_quotient * UPG_HINT_GRANU);
		last_quotient = now_quotient;
	}
}

static int send_upgrade(FLASH_FILE_HEAD *file_head, uint32_t bin_addr)
{
	uint8_t frame_size = 0;
	uint32_t send_bytes = 0;
	uint32_t remain_bytes = 0;
	uint8_t resp = RESP_FLAG_NORMAL;
	int ret = 0;
	
	ret = request_binsize(&resp, file_head->bintype, file_head->size);
	if (ret != 0) {
		KIDS_A10_PRT("request_binsize return failed.\n");
		return -1;
	}
	if (resp != RESP_UPG_BINSIZE_SUCCESS) {
		KIDS_A10_PRT("request_binsize return response abnormal, resp = 0x%02x.\n", resp);
		return -1;
	}
	remain_bytes = file_head->size - send_bytes;
	hint_percent(0, 0);
	while (remain_bytes > 0) {
		frame_size = remain_bytes < UPG_PAYLOAD_DATA_SIZE ?
								 (uint8_t)remain_bytes + UPG_PAYLOAD_HAED_SIZE : SLAVE_DATA_MAX;
		ret = request_framehead(&resp, frame_size);
		if (ret != 0) {
			KIDS_A10_PRT("request_framehead return failed.\n");
			return -1;
		}
		if (resp != RESP_UPG_FRAMEHEAD_SUCCESS) {
			KIDS_A10_PRT("request_framehead return response abnormal, resp = 0x%02x.\n", resp);
			return -1;
		}
		ret = request_payload(&resp, file_head->bintype, bin_addr, send_bytes, frame_size - UPG_PAYLOAD_HAED_SIZE);
		if (ret != 0) {
			KIDS_A10_PRT("request_payload return failed.\n");
			return -1;
		}
		if (resp != RESP_UPG_PAYLOAD_SUCCESS) {
			if (resp == RESP_UPG_ALL_SUCCESS) {
				
			} else if (resp == RESP_UPG_PAYLOAD_CRC) {
				continue;
			} else {
				KIDS_A10_PRT("request_payload return response abnormal, resp = 0x%02x.\n", resp);
				return -1;
			}
		}
		send_bytes += frame_size - UPG_PAYLOAD_HAED_SIZE;
		remain_bytes = file_head->size - send_bytes;
		hint_percent(send_bytes, file_head->size);
	}
	
	if (resp != RESP_UPG_ALL_SUCCESS) {
		KIDS_A10_PRT("unknow error. resp = 0x%02x\n", resp);
		return -1;
	}
	
	return 0;
}

int handle_upgrade(void)
{
	int ret = 0;
	int uret = 0;
	int i;
	
	ret = upgf_init();
	if (ret != 0) {
		return -1;
	}
	
	if (!aos_mutex_is_valid(&isd9160_mutex)) {
		KIDS_A10_PRT("isd9160_mutex is invalid.\n");
		return -1;
	}
	ret = aos_mutex_lock(&isd9160_mutex, ISD9160_I2C_TIMEOUT);
	if (ret != 0) {
		KIDS_A10_PRT("ISD9160 is very busy now.\n");
		return -1;
	}
	
	for (i = 0; i < g_upg_head.file_num; ++i) {
		ret = send_upgrade(&g_file_head[i], g_bin_addr[i]);
		if (ret != 0) {
			goto END;
		}
	}
	
END:
	uret = aos_mutex_unlock(&isd9160_mutex);
	if (uret != 0) {
		KIDS_A10_PRT("ISD9160 release failed.\n");
	}
	
	return ret;
}

void isd9160_proc_loop()
{
	while (1) {
		handle_slprt();
		krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND);
	}
}

void isd9160_reset(void)
{
	hal_gpio_output_low(&brd_gpio_table[GPIO_AUDIO_RST]);
	krhino_task_sleep(krhino_ms_to_ticks(50));
	hal_gpio_output_high(&brd_gpio_table[GPIO_AUDIO_RST]);
}

int isd9160_i2c_init(void)
{
	int ret = 0;
	
	if (aos_mutex_is_valid(&isd9160_mutex)) {
		KIDS_A10_PRT("ISD9160 module initialization had completed before now.\n");
		return -1;
	}
	ret = aos_mutex_new(&isd9160_mutex);
	if (ret != 0) {
		KIDS_A10_PRT("aos_mutex_new return failed.\n");
		return -1;
	}
	isd9160_reset();
	
	return 0;
}
