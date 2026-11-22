#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#define I2C5 ((i2c_t *)0x40015000U)

/* CR1 */
#define I2C_CR1_PE_Msk  (1U)

/* CR2 */
#define I2C_CR2_SADD_Msk        (0x3FFU)
#define I2C_CR2_WD_WRN_Pos      (10U)
#define I2C_CR2_WD_WRN_Msk      (1U << I2C_CR2_WD_WRN_Pos)
#define I2C_CR2_ADD10_Pos       (11U)
#define I2C_CR2_ADD10_Msk       (1U << I2C_CR2_ADD10_Pos)
#define I2C_CR2_START_Pos       (13U)
#define I2C_CR2_START_Msk       (1U << I2C_CR2_START_Pos)
#define I2C_CR2_STOP_Pos        (14U)
#define I2C_CR2_STOP_Msk        (1U << I2C_CR2_STOP_Pos)
#define I2C_CR2_NBYTES_Pos      (16U)
#define I2C_CR2_NBYTES_Msk      (0xFFU << I2C_CR2_NBYTES_Pos)
#define I2C_CR2_RELOAD_Pos      (24U)
#define I2C_CR2_RELOAD_Msk      (1U << I2C_CR2_RELOAD_Pos)
#define I2C_CR2_AUTOEND_Pos     (25U)
#define I2C_CR2_AUTOEND_Msk     (1U << I2C_CR2_AUTOEND_Pos)
/* ISR */
#define I2C_ISR_TXIS_Pos        (1U)
#define I2C_ISR_TXIS_Msk        (1U << I2C_ISR_TXIS_Pos)
#define I2C_ISR_NACKF_Pos       (4U)
#define I2C_ISR_NACKF_Msk       (1U << I2C_ISR_NACKF_Pos)
#define I2C_ISR_STOPF_Pos       (5U)
#define I2C_ISR_STOPF_Msk       (1U << I2C_ISR_STOPF_Pos)
#define I2C_ISR_BUSY_Pos        (15U)
#define I2C_ISR_BUSY_Msk        (1U << I2C_ISR_BUSY_Pos)

/* ICR */   
#define I2C_ICR_ADDRCF_Pos      (3U)
#define I2C_ICR_ADDRCF_Msk      (1U << I2C_ICR_ADDRCF_Pos)
#define I2C_ICR_NACKCF_Pos      (4U)
#define I2C_ICR_NACKCF_Msk      (1U << I2C_ICR_NACKCF_Pos)
#define I2C_ICR_STOPCF_Pos      (5U)
#define I2C_ICR_STOPCF_Msk      (1U << I2C_ICR_STOPCF_Pos)

typedef struct
{
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t TIMINGR;
    volatile uint32_t TIMEOUTR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t PECR;
    volatile uint32_t RXDR;
    volatile uint32_t TXDR;
}i2c_t;

int I2C_Init(i2c_t *I2Cx, uint32_t timing);
int I2C_MasterWriteByte(i2c_t *I2Cx, uint8_t addr7, uint8_t data, uint32_t timeout_ticks);

#endif
