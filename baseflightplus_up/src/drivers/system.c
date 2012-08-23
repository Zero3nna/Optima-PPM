/*
 * Copyright (c) 2012 Baseflight U.P.
 * Licensed under the MIT License
 * @author  Scott Driessens v0.1 (August 2012)
 *
 * August 2012 - Added Coarse Event Timers
 *
 * From Timecop's Original Baseflight
 *
 */

#include "board.h"

#include "actuator/mixer.h"

#include "core/printf_min.h"

///////////////////////////////////////////////////////////////////////////////

// Cycle counter stuff - these should be defined by CMSIS, but they aren't
#define DWT_CTRL    (*(volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT  ((volatile uint32_t *)0xE0001004)
#define CYCCNTENA   (1 << 0)

///////////////////////////////////////////////////////////////////////////////

typedef struct gpio_config_t {
    GPIO_TypeDef *gpio;
    uint16_t pin;
    GPIOMode_TypeDef mode;
} gpio_config_t;

///////////////////////////////////////////////////////////////////////////////

// Cycles per microsecond
static volatile uint32_t usTicks = 0;

///////////////////////////////////////////////////////////////////////////////

// Current uptime for 1kHz systick timer. will rollover after 49 days.
// Hopefully we won't care.
static volatile uint32_t sysTickUptime = 0;
static volatile uint32_t sysTickCycleCounter = 0;

///////////////////////////////////////////////////////////////////////////////
// Cycle Counter
///////////////////////////////////////////////////////////////////////////////

static void cycleCounterInit(void)
{
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    usTicks = clocks.SYSCLK_Frequency / 1000000;
    
    // enable DWT access
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // enable the CPU cycle counter
    DWT_CTRL |= CYCCNTENA;
}

///////////////////////////////////////////////////////////////////////////////
// Coarse Timer Utilities
///////////////////////////////////////////////////////////////////////////////

struct periodic_timer_event {
    uint32_t start;
    uint32_t period;
    uint32_t delta;
    event_callback callback;
};

struct single_timer_event {
    uint32_t start;
    uint32_t delay;
    event_callback callback;
};

static volatile struct single_timer_event singleEvents[TIMER_MAX_EVENTS];
static volatile struct periodic_timer_event periodicEvents[TIMER_MAX_EVENTS];

void eventCallbacks(void)
{
	uint8_t i;
    uint32_t temp;
    
	for (i = 0; i < TIMER_MAX_EVENTS; ++i) {
		if (singleEvents[i].callback && (micros() - singleEvents[i].start) > singleEvents[i].delay) {
			singleEvents[i].callback();
            singleEvents[i].callback = 0;
		} if (periodicEvents[i].callback && (micros() - periodicEvents[i].start) > periodicEvents[i].period) {
			periodicEvents[i].callback();
            temp = periodicEvents[i].start;
			periodicEvents[i].start = micros();
            periodicEvents[i].delta = periodicEvents[i].start - temp;
		}
	}
}

void singleEvent(event_callback callback, uint32_t delay)
{
	volatile struct single_timer_event *ev = singleEvents;
	while (ev->callback)
		++ev;
    ev->start = micros();
	ev->delay = delay;
	ev->callback = callback;
}

void periodicEvent(event_callback callback, uint32_t period)
{
	volatile struct periodic_timer_event *ev = periodicEvents;
	while (ev->callback)
		++ev;
	ev->start = micros();
	ev->period = period;
	ev->callback = callback;
}

void printEventDeltas(void)
{
    uint8_t i;
    
    for(i = 0; i < TIMER_MAX_EVENTS - 1; ++i) {
        printf_min("%u, ", periodicEvents[i].delta);
    }
    
    printf_min("%u\n", periodicEvents[TIMER_MAX_EVENTS - 1].delta);
}

///////////////////////////////////////////////////////////////////////////////
// SysTick
///////////////////////////////////////////////////////////////////////////////

void SysTick_Handler(void)
{
    sysTickUptime++;
    sysTickCycleCounter = *DWT_CYCCNT;
}

///////////////////////////////////////////////////////////////////////////////
// System Time in Microseconds
///////////////////////////////////////////////////////////////////////////////

uint32_t micros(void)
{
    register uint32_t oldCycle, cycle, timeMs;
    __disable_irq();
    cycle = *DWT_CYCCNT;
    oldCycle = sysTickCycleCounter;
    timeMs = sysTickUptime;
    __enable_irq();
    return (timeMs * 1000) + (cycle - oldCycle) / usTicks;
}

///////////////////////////////////////////////////////////////////////////////
// System Time in Milliseconds
///////////////////////////////////////////////////////////////////////////////

uint32_t millis(void)
{
    return sysTickUptime;
}

///////////////////////////////////////////////////////////////////////////////
// Delay Microseconds
///////////////////////////////////////////////////////////////////////////////

void delayMicroseconds(uint32_t us)
{
    uint32_t now = micros();
    while(micros() - now < us);
}

///////////////////////////////////////////////////////////////////////////////
// Delay Milliseconds
///////////////////////////////////////////////////////////////////////////////

void delay(uint32_t ms)
{
    while (ms--)
        delayMicroseconds(1000);
}

///////////////////////////////////////////////////////////////////////////////
// System Reset
///////////////////////////////////////////////////////////////////////////////

#define AIRCR_VECTKEY_MASK    ((uint32_t)0x05FA0000)

void systemReset(bool toBootloader)
{
    if (toBootloader) {
        // 1FFFF000 -> 20000200 -> SP
        // 1FFFF004 -> 1FFFF021 -> PC
        *((uint32_t *) 0x20004FF0) = 0xDEADBEEF;        // 20KB STM32F103
    }
    // Generate system reset
    SCB->AIRCR = AIRCR_VECTKEY_MASK | (uint32_t) 0x04;
}

///////////////////////////////////////////////////////////////////////////////
// System Initialization
///////////////////////////////////////////////////////////////////////////////

void systemInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    uint8_t i;

    gpio_config_t gpio_cfg[] = {
        {LED0_GPIO, LED0_PIN, GPIO_Mode_Out_PP},        // PB3 (LED)
        {LED1_GPIO, LED1_PIN, GPIO_Mode_Out_PP},        // PB4 (LED)
    };

    uint8_t gpio_count = sizeof(gpio_cfg) / sizeof(gpio_cfg[0]);

    // Turn on clocks for stuff we use
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    RCC_ClearFlag();

    // Make all GPIO in by default to save power and reduce noise
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Turn off JTAG port 'cause we're using the GPIO for leds
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    // Configure gpio
    for (i = 0; i < gpio_count; i++) {
        GPIO_InitStructure.GPIO_Pin = gpio_cfg[i].pin;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = gpio_cfg[i].mode;

        GPIO_Init(gpio_cfg[i].gpio, &GPIO_InitStructure);
    }
    
	/* set all events to disabled */
	for (i=0; i < TIMER_MAX_EVENTS; ++i) {
		singleEvents[i].callback = 0;
		periodicEvents[i].callback = 0;
	}

    // Init cycle counter
    cycleCounterInit();

    // SysTick
    SysTick_Config(SystemCoreClock / 1000);

    LED0_OFF();
    LED1_OFF();

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     // 2 bits for pre-emption priority, 2 bits for subpriority

    checkFirstTime(false);
    readEEPROM();
}

///////////////////////////////////////////////////////////////////////////////