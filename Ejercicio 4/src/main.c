#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "fsm.h"

#define PERIOD_TICK 100/portTICK_RATE_MS
#define REBOUND_TICK 200/portTICK_RATE_MS
#define TIMER_PERIOD 60000/portTICK_RATE_MS
#define ETS_GPIO_INTR_ENABLE() _xt_isr_unmask(1 << ETS_GPIO_INUM)
#define ETS_GPIO_INTR_DISABLE() _xt_isr_mask(1 << ETS_GPIO_INUM)


enum fsm_state {
  LED_ON,
  LED_OFF,
};

static portTickType timer1;
volatile int done0 = 0;

void isr_gpio(void* arg) {
  static portTickType xLastISRTick0 = 0;

  uint32 status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);          //READ STATUS OF INTERRUPT

  portTickType now = xTaskGetTickCount ();
  portTickType timer1 = xTaskGetTickCount ();
  if (status & BIT(0)) {
    if (now > xLastISRTick0) {
      done0 = 1;
    }
    xLastISRTick0 = now + REBOUND_TICK;
  }

  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status);       //CLEAR THE STATUS IN THE W1 INTERRUPT REGISTER
}

static int boton_active (fsm_t* this) {return done0; }

static void encender (fsm_t* this) { done0 = 0; GPIO_OUTPUT_SET(2, 0); }
static void apagar   (fsm_t* this) { done0 = 0; GPIO_OUTPUT_SET(2, 1); }



/*
 * Máquina de estados
 */
static fsm_trans_t interruptor[] = {
  { LED_OFF,   boton_active, LED_ON , encender },
  { LED_ON, boton_active, LED_ON, encender},
  {-1, NULL, -1, NULL },
};

/*
 * wait until next_activation (absolute time)
 */
void delay_until (unsigned int next)
{
  unsigned int now = millis();
  delay (next - now);
}

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }
    return rf_cal_sec;
}

void inter(void* ignore)
{
    fsm_t* fsm = fsm_new(interruptor);
    apagar(fsm);
    portTickType xLastWakeTime;

    PIN_FUNC_SELECT(GPIO_PIN_REG_0, FUNC_GPIO0);
    PIN_FUNC_SELECT(GPIO_PIN_REG_15, FUNC_GPIO15);
    
    gpio_intr_handler_register((void*)isr_gpio, NULL);
    gpio_pin_intr_state_set(0, GPIO_PIN_INTR_NEGEDGE);
    gpio_pin_intr_state_set(15, GPIO_PIN_INTR_POSEDGE);
    ETS_GPIO_INTR_ENABLE();

    xLastWakeTime = xTaskGetTickCount ();

    while(true) {
    	fsm_fire(fsm);
        vTaskDelayUntil(&xLastWakeTime, PERIOD_TICK);
    }

    vTaskDelete(NULL);
}



/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    xTaskCreate(&inter, "startup", 2048, NULL, 1, NULL);
}

