#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_wifi.h"
#include "movement_states.h"

// PCA9685 I2C address
#define PCA9685_ADDR 0x40

// PCA9685 registers
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE

// I2C configuration
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

#define ang 30

int current_angles[12];
uint8_t origin_angles[12] = {45, 130 - ang, 155 - ang, 45, 160 - ang, 175 - ang, 45, 180 - ang, 180 - ang, 55, 160 - ang, 160 - ang};

extern MovementStates movement_states;

int legs[4] = {0, 1, 2, 3};
int fastDelay = 8;
int slowDelay = 40;


void i2c_master_init()
{
    // Initialize I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

// Function to write a value to a register
esp_err_t pca9685_write_register(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PCA9685_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Function to set PWM frequency
void pca9685_set_pwm_freq(float freq)
{
    uint8_t prescale_val = (uint8_t)(25000000.0 / (4096.0 * freq) - 1.0);
    pca9685_write_register(PCA9685_MODE1, 0x10); // Sleep mode
    pca9685_write_register(PCA9685_PRESCALE, prescale_val);
    pca9685_write_register(PCA9685_MODE1, 0x80); // Restart
    vTaskDelay(pdMS_TO_TICKS(5));
    pca9685_write_register(PCA9685_MODE1, 0x00); // Normal mode
}

// Function to set PWM value for a channel
void pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off)
{
    uint8_t reg = 0x06 + 4 * channel;
    pca9685_write_register(reg, on & 0xFF);
    pca9685_write_register(reg + 1, on >> 8);
    pca9685_write_register(reg + 2, off & 0xFF);
    pca9685_write_register(reg + 3, off >> 8);
}

void set_servo_angle(uint8_t channel, uint16_t angle)
{
    uint16_t pulse = 150 + angle * 2.275;
    pca9685_set_pwm(channel, 0, pulse);
}

void reset_servos()
{
    for (uint8_t i = 0; i < 12; i++)
    {
        set_servo_angle(i, origin_angles[i]);
        current_angles[i] = origin_angles[i];
    }
    printf("reset here\n");
}

void spider_init()
{
    i2c_master_init();
    pca9685_set_pwm_freq(50);
    reset_servos();
}


void angle_motion(int initialPin, int angles[3], int delay)
{
    // loops on every angle until it reaches zero while being applied to its corresponding servo
    int steps[3];
    for (int i = 0; i < 3; i++)
        steps[i] = (angles[i] > 0) ? 2 : -2; // sets the sign of angle change

    while (angles[0] != 0 || angles[1] != 0 || angles[2] != 0)
    {
        for (int i = 0; i < 3; i++)
        {
            if (angles[i] != 0)
            {
                set_servo_angle(initialPin + i, current_angles[initialPin + i] += steps[i]);
                angles[i] -= steps[i];
            }
        }

        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}


void move_spider_leg(int initialPin, int sign, int angleInc)
{
    if(sign == 0)
    {
        vTaskDelay(pdMS_TO_TICKS(500)); 
        return;
    }

    angle_motion(initialPin, (int[]){40 * sign, 32, 32 - angleInc}, fastDelay);
    angle_motion(initialPin, (int[]){0, -32, -32}, fastDelay);
    angle_motion(initialPin, (int[]){-40 * sign, 0, 20}, slowDelay);
}


void leg_task(int leg, int delay)
{
    int initialPin = leg * 3;
    int sign = 0;
    int angleInc = 0;
    uint8_t lastState = 0;

    //movement_states.forward = 1;

    while (1)
    {
        uint8_t currentState = movement_states.forward || movement_states.backward || movement_states.right || movement_states.left;

        if (currentState && !lastState)
        {
            (movement_states.forward)  ? angle_motion(initialPin, (int[]){-22 * (sign = (leg % 2 == 0) ? 1 : -1), 0, 0}, slowDelay) :
            (movement_states.backward) ? angle_motion(initialPin, (int[]){22 * (sign = (leg % 2 == 0) ? -1 : 1), 0, 0}, slowDelay) : (void) 0;

            vTaskDelay(pdMS_TO_TICKS(delay));
            angleInc = 0;
        }

        // if-else ternary structure, chooses one of the 4 movement states to set the corresponding leg direction
        (movement_states.forward)  ? (sign = (leg % 2 == 0) ? 1 : -1) :
        (movement_states.backward) ? (sign = (leg % 2 == 0) ? -1 : 1) :
        (movement_states.right)    ? sign = 1 :
        (movement_states.left)     ? sign = -1 : 0;

        move_spider_leg(initialPin, sign, angleInc);

        lastState = currentState;
        angleInc = 20;
    }
}


void Leg_1(void *pvParameters)
{
    leg_task(0, 0);
}

void Leg_2(void *pvParameters)
{
    leg_task(1, 600);
}

void Leg_3(void *pvParameters)
{
    leg_task(2, 900);
}

void Leg_4(void *pvParameters)
{
    leg_task(3, 300);
}


void spider()
{
    i2c_master_init();
    pca9685_set_pwm_freq(50);
    reset_servos();
    
    vTaskDelay(pdMS_TO_TICKS(3000));

    xTaskCreate(Leg_1, "Leg 1", 2048, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(Leg_2, "Leg 2", 2048, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(Leg_3, "Leg 3", 2048, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(Leg_4, "Leg 4", 2048, NULL, configMAX_PRIORITIES - 1, NULL);
}
