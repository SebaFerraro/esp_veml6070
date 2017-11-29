/*
  Example sketch for VEML6070-Breakout (Digital UV Light Sensor).
  Rset=270k on breakout, UVA sensitivity: 5.625 uW/cm²/step
  Board           I2C/TWI Pins
                  SDA, SCL
  ----------------------------
  Uno, Ethernet    A4, A5
  Mega             20, 21
  Leonardo          2,  3
  Due              20, 21
  Integration Times and UVA Sensitivity:
    Rset=240k -> 1T=100.0ms ->   5.000 uW/cm²/step
    Rset=270k -> 1T=112.5ms ->   5.625 uW/cm²/step
    Rset=300k -> 1T=125.0ms ->   6.250 uW/cm²/step
    Rset=600k -> 1T=250.0ms ->  12.500 uW/cm²/step
*/

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c.h>
#include "esp_system.h"
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "rom/ets_sys.h"
#include "driver/gpio.h"

#define I2C_VEML6070_ADDR_CMD 0x38 //0x38 and 0x39
#define I2C_VEML6070_ADDR1 0x38 //0x38 and 0x39
#define I2C_VEML6070_ADDR2 0x39 //0x38 and 0x39

//Integration Time
#define IT_1_2 0x0 //1/2T
#define IT_1   0x1 //1T
#define IT_2   0x2 //2T
#define IT_4   0x3 //4T


#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

#define SDA_PIN 18
#define SCL_PIN 19

#define SDA_PIN GPIO_NUM_18
#define SCL_PIN GPIO_NUM_19
#define I2C_ADDR 0x38
#define I2C_MASTER_ACK 0
#define I2C_MASTER_NACK 1

static char tag[] = "i2cscanner";

//void task_i2cscanner(void *ignore) {
uint16_t i2c_veml6070_uv(uint8_t cmd_conf) {
	uint16_t uv=0;
	esp_err_t espRc;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (I2C_VEML6070_ADDR_CMD << 1) | I2C_MASTER_WRITE, 0);
    	i2c_master_write_byte(cmd, cmd_conf, 1);
	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
		
	printf("CMD rc= %d (0x%x)\n", espRc, espRc);

        vTaskDelay(500 / portTICK_RATE_MS);
		
    	uint8_t msb=0, lsb=0;
        cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (I2C_VEML6070_ADDR1 << 1) | I2C_MASTER_READ, 0);
	i2c_master_read_byte(cmd, &msb, 0);
	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	ESP_LOGD(tag, "Datos msb: %d  rc=%d (0x%x)\n", msb, espRc, espRc);
               
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (I2C_VEML6070_ADDR2 << 1) | I2C_MASTER_READ, 0);
	i2c_master_read_byte(cmd, &lsb, 0);
	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	ESP_LOGD(tag, "Datos lsb :%d  rc=%d (0x%x)\n", lsb, espRc, espRc);
	uv=((uint16_t) msb<<8) | lsb;
	printf("Datos msb lsb : %d  %d  UV: %d\n", msb, lsb,uv);
        return uv;

}
//void Veml6070(void){
//  Wire.begin();
//task_i2cscanner();

//}

void i2c_master_init()
{
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA_PIN,
		.scl_io_num = SCL_PIN,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 1000000
	};
	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
	
}

uint8_t I2C_VEML6070_CMD(uint8_t dev_addr, uint8_t reg_data)
{
	//uint32_t iError = BME280_INIT_VALUE;

	esp_err_t espRc;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, 0);

	i2c_master_write_byte(cmd, reg_data, 1);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		printf("write susses\n");
		//iError = SUCCESS;
	} else {
		printf("write faili\n");
		//iError = FAIL;
	}
	i2c_cmd_link_delete(cmd);

	return (uint8_t)espRc;
}

uint16_t I2C_VEML6070_READ(uint8_t dev_addr)
{
	//uint32_t iError = BME280_INIT_VALUE;
	esp_err_t espRc;
	uint8_t msb=0, lsb=0;
	uint16_t uv=0;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, 0);
	i2c_master_read_byte(cmd, &lsb, 1);
	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	
	if (espRc == ESP_OK) {
		printf("read lsb succes");
		//iError = SUCCESS;
	} else {
		printf("read lsb fail");
		//iError = FAIL;
	}

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ((dev_addr+1) << 1) | I2C_MASTER_READ, 0);
	i2c_master_read_byte(cmd, &msb, 1);
	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	if (espRc == ESP_OK) {
		printf("read msb succes");
		//iError = SUCCESS;
	} else {
		printf("read msb fail");
		//iError = FAIL;
	}

	printf("msb: %d lsb: %d\n",msb,lsb);
	uv=((uint16_t) msb<<8) | lsb;
	return uv;
}

void Veml6070(void){
//  Wire.begin();
//i2c_master_init();
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = SDA_PIN;
	conf.scl_io_num = SCL_PIN;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	i2c_param_config(I2C_NUM_0, &conf);

	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

uint8_t sal=0;
uint8_t dat=0;
uint16_t uv=0;
dat=((IT_1<<2) | 0x02);
//dat=((IT_1<<2) | 0x02);
//sal=I2C_VEML6070_CMD(I2C_VEML6070_ADDR_CMD,dat);
//printf("CMD %d\n",sal);
//vTaskDelay(500 / portTICK_RATE_MS);
while(1){
	uv=i2c_veml6070_uv(dat);
	//uv=I2C_VEML6070_READ(I2C_VEML6070_ADDR1);
	printf("READ %d UV: %d\n",sal,uv);
	vTaskDelay(2000 / portTICK_RATE_MS);
	}
}
////i2c_veml6070_init(SDA_PIN,SCL_PIN,I2C_ADDR,dat);
////sal=i2c_veml6070_index(I2C_ADDR);
//sal=I2C_bus_write(I2C_VEML6070_ADDR, 0x38, 0x02, 1);
//printf("write %d\n",sal);
//sal=I2C_bus_read(I2C_VEML6070_ADDR, 0x39, dat, 1);
//printf("read %d  %d\n",sal, dat);
//}

/*
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"

#include "sdkconfig.h" // generated by "make menuconfig"


#define SDA_PIN GPIO_NUM_18
#define SCL_PIN GPIO_NUM_19
#define tag "VEML6070"

#define VEML6070_ADDRESS1 0x38
#define VEML6070_ADDRESS2 0x39

#define I2C_MASTER_ACK 0
#define I2C_MASTER_NACK 1

void i2c_master_init()
{
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA_PIN,
		.scl_io_num = SCL_PIN,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 100000
	};
	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void task_max44009_read_ambient_light(void)
{
	uint8_t lux_h;
	uint8_t lux_l;

	esp_err_t espErr;
	i2c_cmd_handle_t cmd;

	while (true) {
		vTaskDelay(800/portTICK_PERIOD_MS);

		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (VEML6070_ADDRESS1 << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, 0x02, true);
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (VEML6070_ADDRESS1 << 1) | I2C_MASTER_READ, true);
		i2c_master_read_byte(cmd, &lux_h, I2C_MASTER_NACK);
		i2c_master_stop(cmd);
		espErr = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		// According to datasheet (p17), we can read two registers in one transmission
		// by repeated start signal. But unfortunately it timeouts.
		// So we re-create or I2C link for to get lux low-byte.
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (VEML6070_ADDRESS1 << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, 0x02, true);
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (VEML6070_ADDRESS2 << 1) | I2C_MASTER_READ, true);
		i2c_master_read_byte(cmd, &lux_l, I2C_MASTER_NACK);
		i2c_master_stop(cmd);
		espErr = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		if (espErr == ESP_OK) {
			int exponent = (lux_h & 0xf0) >> 4;
			int mant = (lux_h & 0x0f) << 4 | lux_l;
			float lux = (float)(((0x00000001 << exponent) * (float)mant) * 0.045);
			ESP_LOGI(tag, "%.3f lux", lux);
		} else {
			ESP_LOGE(tag, "fail to read from sensor. code: %.2X", espErr);
		}

		i2c_cmd_link_delete(cmd);

	}
	//vTaskDelete(NULL);
}

//void app_main(void)
//{
//	i2c_master_init();
//	xTaskCreate(&task_max44009_read_ambient_light, "max44009_read_ambient_light",  2048, NULL, 6, NULL);
//}
void Veml6070(void){
i2c_master_init();
task_max44009_read_ambient_light(); 

}
*/
