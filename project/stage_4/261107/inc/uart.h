#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>

#define UART4 ((uart_t *)0x40010000U)
#define UART_ISR_TXNF (1U << 7U)
#define UART_ISR_TC  (1U << 6U)
#define UART_ISR_RXNE (1U << 5U)

#define UART_ISR_RXFNE (1U << 5U)
#define UART_ISR_TXFNF (1U << 7U)

typedef enum
{
    UART_WORDLENGTH_8BIT = 0U,
    UART_WORDLENGTH_9BIT,
    UART_WORDLENGTH_7BIT,
}uart_wordlength_e;

typedef enum
{
    UART_PARITY_ODD = 0U,
    UART_PARITY_EVEN,
    UART_PARITY_NONE,
}uart_parity_e;

typedef enum
{
    UART_STOP_1BIT = 0U,
    UART_STOP_0_5BIT,
    UART_STOP_2BIT,
    UART_STOP_1_5BIT,
}uart_stopbits_e;

typedef enum
{
    UART_FIFO_DISABLE = 0U,
    UART_FIFO_ENABLE ,
} uart_fifo_mode_e;

typedef struct 
{
    uint32_t BaudRate;
    uart_wordlength_e WordLength;
    uart_parity_e Paritysel;
    uart_stopbits_e Stopbits;
    uart_fifo_mode_e  FifoMode;
}uart_init_t;

typedef struct
{
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
    volatile uint32_t PRESC;
} uart_t;

int UART4_DeInit(void);
int UART4_Init(const uart_init_t *config, uint32_t uart_clock_hz);
void HAL_UART4_PutChar(uint8_t data);
int HAL_UART4_GetChar(uint8_t *data_out);
int HAL_UART4_PutString(const char *str);
int HAL_UART4_GetString(char *buf, uint8_t buf_size);
int HAL_UART4_ReadAvailable(uint8_t *buf, size_t capacity, size_t *received_out);
int HAL_UART4_WriteAvailable(const uint8_t *buf, size_t capacity, size_t *sent_out);

#endif
