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

#define I2C_VEML6070_ADDR 0x38 //0x38 and 0x39
#define I2C_VEML6070_ADDR1 0x39 //0x38 and 0x39
#define I2C_VEML6070_ADDR2 0x73 //0x38 and 0x39

//Integration Time
#define IT_1_2 0x0 //1/2T
#define IT_1   0x1 //1T
#define IT_2   0x2 //2T
#define IT_4   0x3 //4T


//#define DATA_LENGTH                        512              /*!<Data buffer length for test buffer*/
//#define RW_TEST_LENGTH                     129              /*!<Data length for r/w test, any value from 0-DATA_LENGTH*/
//#define DELAY_TIME_BETWEEN_ITEMS_MS        1234             /*!< delay time between different test items */

//#define I2C_EXAMPLE_MASTER_SCL_IO          19               /*!< gpio number for I2C master clock */
//#define I2C_EXAMPLE_MASTER_SDA_IO          18               /*!< gpio number for I2C master data  */
//#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0        /*!< I2C port number for master dev */
//#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
//#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
//#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000           /*!< I2C master clock frequency */

#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

//SemaphoreHandle_t print_mux = NULL;

/**
 * @brief i2c master initialization
 */

/*
static void i2c_example_master_init()
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                       I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
}

static esp_err_t i2c_veml6070_read(i2c_port_t i2c_num, uint8_t* data_h, uint8_t* data_l)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, I2C_VEML6070_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    ////i2c_master_write_byte(cmd, (IT_1<<2) | 0x02, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x02, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    if (ret != ESP_OK) {
        printf("Falla i2c_master_cmd_begin 1. %d",ret);
	return ret;
    }
    i2c_cmd_link_delete(cmd);
    
    vTaskDelay(500 / portTICK_RATE_MS);
    
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(I2C_VEML6070_ADDR1) << 1 | READ_BIT, ACK_CHECK_EN);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    i2c_master_read_byte(cmd, data_h, ACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    if (ret != ESP_OK) {
        printf("Falla i2c_master_cmd_begin 2. %d",ret);
	return ret;
    }
    i2c_cmd_link_delete(cmd);
    
    vTaskDelay(10 / portTICK_RATE_MS);
    
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_VEML6070_ADDR2) << 1 | READ_BIT, ACK_CHECK_EN);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    i2c_master_read_byte(cmd, data_l, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    if (ret != ESP_OK) {
        printf("Falla i2c_master_cmd_begin 3. %d",ret);
	return ret;
    }
    i2c_cmd_link_delete(cmd);
    return ret;
}


void Veml6070(void){
//  Wire.begin();
    //i2c_example_slave_init();
    i2c_example_master_init();
    //i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    //i2c_master_start(cmd);
    //i2c_master_write_byte(cmd, ( ESP_SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);
    //if (size > 1) {
  //Wire.beginTransmission(I2C_ADDR);
  //Wire.write((IT_1<<2) | 0x02);
  //Wire.endTransmission();
  //vTaskDelay(500 / portTICK_PERIOD_MS);
  uint8_t msb=0, lsb=0;
  uint16_t uv;
  esp_err_t ret;
  //Wire.requestFrom(I2C_ADDR+1, 1); //MSB
  //vTaskDelay(1 / portTICK_PERIOD_MS);
  //if(Wire.available())
  //  msb = Wire.read();

  //Wire.requestFrom(I2C_ADDR+0, 1); //LSB
  //vTaskDelay(1 / portTICK_PERIOD_MS);
  //if(Wire.available())
  //  lsb = Wire.read();
  while(1){
  ret=i2c_veml6070_read(I2C_NUM_0,msb,lsb);
  uv = (msb<<8) | lsb;
  printf("UV %d \n",uv); //output in steps (16bit)

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
*/

/*
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"

#define SDA_PIN 18
#define SCL_PIN 19

static char tag[] = "i2cscanner";

//void task_i2cscanner(void *ignore) {
void task_i2cscanner(void) {
	ESP_LOGD(tag, ">> i2cScanner");
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = SDA_PIN;
	conf.scl_io_num = SCL_PIN;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	i2c_param_config(I2C_NUM_0, &conf);

	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

	int i;
	esp_err_t espRc;
	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	printf("00:         ");
	//for (i=3; i< 0x78; i++) {
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (0x38 << 1) | I2C_MASTER_WRITE, 0);
    		i2c_master_write_byte(cmd, ((IT_1<<2) | 0x02), 1);
		i2c_master_stop(cmd);

		espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		//if (i%16 == 0) {
		//	printf("\n%.2x:", i);
		//}
		//if (espRc == 0) {
		//	printf(" %.2x", i);
		//} else {
		//	printf(" --");
		//}
		//ESP_LOGD(tag, "i=%d, rc=%d (0x%x)", i, espRc, espRc);
		ESP_LOGD(tag, "rc= %d (0x%x)\n", espRc, espRc);
		printf("rc= %d (0x%x)\n", espRc, espRc);

                vTaskDelay(500 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);
    		uint8_t data_h=0, lsb=0;
                cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (0x39 << 1) | I2C_MASTER_READ, 0);
    		//i2c_master_write_byte(cmd, 0x70, 1);
		i2c_master_read_byte(cmd, data_h, 0);
		i2c_master_stop(cmd);
		espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);
		ESP_LOGD(tag, "Datos:%d  rc=%d (0x%x)\n", data_h, espRc, espRc);
		printf("Datos:%d  rc=%d (0x%x)\n", data_h, espRc, espRc);
	//}
	printf("\n");
	vTaskDelete(NULL);
}
void Veml6070(void){
//  Wire.begin();
task_i2cscanner();

}
*/

#define SDA_PIN GPIO_NUM_18
#define SCL_PIN GPIO_NUM_19

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
		.master.clk_speed = 1000000
	};
	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

uint8_t I2C_bus_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
	//uint32_t iError = BME280_INIT_VALUE;

	esp_err_t espRc;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, reg_addr, true);
	i2c_master_write(cmd, reg_data, cnt, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		printf("write susses");
		//iError = SUCCESS;
	} else {
		printf("write fail");
		//iError = FAIL;
	}
	i2c_cmd_link_delete(cmd);

	return (uint8_t)espRc;
}

uint8_t I2C_bus_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
	//uint32_t iError = BME280_INIT_VALUE;
	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, reg_addr, true);

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

	if (cnt > 1) {
		i2c_master_read(cmd, reg_data, cnt-1, I2C_MASTER_ACK);
	}
	i2c_master_read_byte(cmd, reg_data+cnt-1, I2C_MASTER_NACK);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		printf("read susses");
		//iError = SUCCESS;
	} else {
		printf("read fail");
		//iError = FAIL;
	}

	i2c_cmd_link_delete(cmd);

	return (uint8_t)espRc;
}

void Veml6070(void){
//  Wire.begin();
i2c_master_init();
uint8_t sal=0;
uint8_t dat=0;
sal=I2C_bus_write(I2C_VEML6070_ADDR, 0x70, 0x02, 1);
printf("write %d",sal);
sal=I2C_bus_read(I2C_VEML6070_ADDR, 0x71, dat, 1);
printf("read %d  %d",sal, dat);
}

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
