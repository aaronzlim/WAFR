/*
 * Author: Aaron Lim
 * Project: WAFR
 * Filename: max30102_wire_driver.h
 * Description: Adapting the original max30102 driver for the Wire library
 * 
 * Revision History:
 * 2-7-17 Rev 1.00 BETA
 */

#ifndef max30102_wire_driver_H
#define max30102_wire_driver_H

// The Wire library uses 7-bit addresses, and appends the Write (0) / Read (1) bit.
// The Max30102 has write adderss 0xAE and read address 0xAF
#define I2C_MAX30102_ADDR 0x57

//register addresses
#define REG_INTR_STATUS_1 0x00
#define REG_INTR_STATUS_2 0x01
#define REG_INTR_ENABLE_1 0x02
#define REG_INTR_ENABLE_2 0x03
#define REG_FIFO_WR_PTR 0x04
#define REG_OVF_COUNTER 0x05
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D
#define REG_PILOT_PA 0x10
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TEMP_INTR 0x1F
#define REG_TEMP_FRAC 0x20
#define REG_TEMP_CONFIG 0x21
#define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

byte max30102_read_reg(byte reg);
void max30102_write_reg(uint8_t addr, uint8_t data);
void max30102_set_reg_ptr(uint8_t addr);
bool max30102_read_fifo(uint32_t *red_buffer, uint32_t *ir_buffer, uint8_t idx);
void max30102_init(void);
void max30102_clear_interrupt_status_regs(void);
void max30102_reset(void);


#endif // max30102_wire_driver_H
