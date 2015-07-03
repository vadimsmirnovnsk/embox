/**
 * @file
 * @brief
 *
 * @author  Alex Kalmuk
 * @date    31.05.2015
 */

#include <stdio.h>
#include <unistd.h>
#include <stm32f3xx.h>
#include <stm32f3_discovery.h>
#include <stm32f3xx_hal_gpio.h>
#include <stm32f3xx_hal_rcc.h>

#include <drivers/block_dev/flash/flash_dev.h>
#include <stm32f3_discovery_gyroscope.h>
#include <stm32f3_discovery_accelerometer.h>

#define ACC_TOTAL (512 * 4 + 128)
#define ACC_VAL_SIZE (2)
#define GYRO_TOTAL ACC_TOTAL
#define GYRO_VAL_SIZE (4)

extern struct flash_dev stm32f3_flash;
/* Offsets for arrays */
static const void *acc_x_values = (void*) 0x0;
static const void *acc_y_values = (void*) (ACC_TOTAL * ACC_VAL_SIZE);
static const void *acc_z_values = (void*) (2 * ACC_TOTAL * ACC_VAL_SIZE);
static const void *gyro_x_values = (void*) (3 * (ACC_TOTAL * ACC_VAL_SIZE));
static const void *gyro_y_values = (void*) (3 * (ACC_TOTAL * ACC_VAL_SIZE) + GYRO_TOTAL * GYRO_VAL_SIZE);
static const void *gyro_z_values = (void*) (3 * (ACC_TOTAL * ACC_VAL_SIZE) + 2 * GYRO_TOTAL * GYRO_VAL_SIZE);

/* motor 1*/
#define MOTOR_ENABLE1  GPIO_PIN_3
#define MOTOR_INPUT1   GPIO_PIN_5
#define MOTOR_INPUT2   GPIO_PIN_7
/* motor 2 */
#define MOTOR_ENABLE2  GPIO_PIN_2
#define MOTOR_INPUT3   GPIO_PIN_4
#define MOTOR_INPUT4   GPIO_PIN_6

struct motor {
	GPIO_TypeDef  *GPIOx;
	uint16_t enable;
	uint16_t input[2];
	/* 10, 01, 11 */
	uint8_t enabled_inputs:2;
};

enum motor_run_direction {
	MOTOR_RUN_LEFT,
	MOTOR_RUN_RIGHT
};

static struct motor motor1;
static struct motor motor2;

static void stm32f3_delay(uint32_t delay) {
	while(delay--)
		;
}

static void init_pins(GPIO_TypeDef  *GPIOx, uint16_t pins) {
	GPIO_InitTypeDef  GPIO_InitStruct;

	GPIO_InitStruct.Pin = pins;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

static void motor_init(struct motor *m, GPIO_TypeDef  *GPIOx,
		uint16_t enable, uint16_t in1, uint16_t in2, uint8_t mask) {
	m->GPIOx = GPIOx;
	m->enable = enable;
	m->input[0] = in1;
	m->input[1] = in2;
	m->enabled_inputs = mask;
}

static void motor_enable(struct motor *m) {
	HAL_GPIO_WritePin(GPIOD, m->enable, GPIO_PIN_SET);
}

#if 0
static void motor_disable(struct motor *m) {
	HAL_GPIO_WritePin(GPIOD, m->enable, GPIO_PIN_RESET);
}
#endif

static void motor_run(struct motor *m, enum motor_run_direction dir) {
	uint8_t input;

	if (dir == MOTOR_RUN_LEFT) {
		input = 0;
	} else if (dir == MOTOR_RUN_RIGHT) {
		input = 1;
	}
	HAL_GPIO_WritePin(GPIOD, m->input[!input], GPIO_PIN_RESET);
	stm32f3_delay(0xFF); /* FIXME may be it is not needed */
	HAL_GPIO_WritePin(GPIOD, m->input[input], GPIO_PIN_SET);
}

static void motor_stop(struct motor *m) {
	stm32f3_delay(1000);
	HAL_GPIO_WritePin(GPIOD, m->enable, GPIO_PIN_RESET);
	stm32f3_delay(1000);
	HAL_GPIO_WritePin(GPIOD, m->input[0], GPIO_PIN_RESET);
	stm32f3_delay(1000);
	HAL_GPIO_WritePin(GPIOD, m->input[1], GPIO_PIN_RESET);
	stm32f3_delay(1000);
}

static void store_val(const void *arr, int pos, void *data, int size) {
	flash_write(&stm32f3_flash,
			(unsigned long) arr + pos * size,
			data,
			size);
}

static void acc_get_store(int pos) {
	int16_t buf[3] = {0, 0, 0};
	BSP_ACCELERO_GetXYZ(buf);

	store_val(acc_x_values, pos, &buf[0], sizeof(buf[0]));
	store_val(acc_y_values, pos, &buf[1], sizeof(buf[1]));
	store_val(acc_z_values, pos, &buf[2], sizeof(buf[2]));
}

static void gyro_get_store(int pos) {
	float buf[3] = {0, 0, 0};
	BSP_GYRO_GetXYZ(buf);

	store_val(gyro_x_values, pos, &buf[0], sizeof(buf[0]));
	store_val(gyro_y_values, pos, &buf[1], sizeof(buf[1]));
	store_val(gyro_z_values, pos, &buf[2], sizeof(buf[2]));
}

static void acc_test(void) {
	for (size_t m = 0; m < ACC_TOTAL; m++)
		acc_get_store(m);
	motor_stop(&motor1);
}

static void gyro_test(void) {
	for (size_t m = 0; m < GYRO_TOTAL; m++)
		gyro_get_store(m);
	motor_stop(&motor1);
}

static void acc_gyro_test(void) {
	for (size_t m = 0; m < ACC_TOTAL; m++) {
		acc_get_store(m);
		gyro_get_store(m);
	}
	motor_stop(&motor1);
}


// --- fault detection code START

#define THRESHOLD       3000
#define INC_WINDOW_SIZE 50
#define EMA_ALPHA ((float) 2.0 / (float) (INC_WINDOW_SIZE + 1))

static float incs[INC_WINDOW_SIZE] = {0};
static int inc_index = 0;
static float average_inc = 0;
static float ema = 0;

#define FD_WIN_SIZE   200
#define FD_THRESHOLD  2600
static float f_dv[FD_WIN_SIZE] = {0};
static int index_l = 0;
static int index_h = FD_WIN_SIZE / 2;
static int index = 0;
static float diff = 0;

static inline float moving_average(float inc) {
	average_inc += (inc - incs[inc_index]) / INC_WINDOW_SIZE;
	incs[inc_index] = inc;
	inc_index = (inc_index + 1) % INC_WINDOW_SIZE;
	return average_inc;
}

static inline float filtered_derivative(float inc) {
	diff += ((inc - 2 * f_dv[index_h] + f_dv[index_l]) / FD_WIN_SIZE);
	f_dv[index] = inc;
	index = (index + 1) % FD_WIN_SIZE;
	index_h = (index_h + 1) % FD_WIN_SIZE;
	index_l = (index_l + 1) % FD_WIN_SIZE;
	return diff;
}

static inline float exp_moving_average(float inc) {
	ema += (inc - ema) * EMA_ALPHA;
	return ema;
}

static inline void fault_handle(void) {
	BSP_LED_On(LED3);
	motor_stop(&motor1);
}

static void fault_detect(void) {
	int16_t buf[3] = {0, 0, 0};
	float filtered_val;

	BSP_ACCELERO_GetXYZ(buf);
	ema = buf[0];

	while(1) {
		BSP_ACCELERO_GetXYZ(buf);
		//store_val(acc_x_values, m, &buf[0], sizeof(buf[0]));
		filtered_val = filtered_derivative(buf[0]);
		if (filtered_val < -FD_THRESHOLD || filtered_val > FD_THRESHOLD) {
			fault_handle();
		}
	}

	motor_stop(&motor1);
}

// --- fault detection code END

int main(void) {
	int res;

	HAL_Init();

	__GPIOD_CLK_ENABLE();

	res = BSP_ACCELERO_Init();
    if (res != HAL_OK) {
        printf("BSP_ACCLEERO_Init failed, returned %d\n", res);
        return -1;
    }

	res = BSP_GYRO_Init();
    if (res != HAL_OK) {
        printf("BSP_GYRO_Init failed, returned %d\n", res);
        return -1;
    }

	BSP_LED_Init(LED4);
	BSP_LED_Init(LED3);

	init_pins(GPIOD, MOTOR_ENABLE1 | MOTOR_INPUT1 | MOTOR_INPUT2 |
			MOTOR_ENABLE2 | MOTOR_INPUT3 | MOTOR_INPUT4);

	motor_init(&motor1, GPIOD, MOTOR_ENABLE1, MOTOR_INPUT1,
		MOTOR_INPUT2, 0x3);
	motor_init(&motor2, GPIOD, MOTOR_ENABLE2, MOTOR_INPUT3,
		MOTOR_INPUT4, 0x3);

	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

	motor_enable(&motor1);
	while(BSP_PB_GetState(BUTTON_USER) != SET)
		;
	for (size_t i = 0; i < 32; i++)
		flash_erase(&stm32f3_flash, i);
	motor_run(&motor1, MOTOR_RUN_RIGHT);

	fault_detect();

	return 0;
}
