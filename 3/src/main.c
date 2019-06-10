#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "string.h"

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

void task_blink(void* ignore)
{
    while(true) {
        char msgmorse[100];
        char* msgstr = "hola mundo";
        str2morse(msgmorse, strlen(msgstr), msgstr);
    	morse_send(msgmorse);
    }
    vTaskDelete(NULL);
}


/*Devolver la correspondencia en Morse de un caracter*/
const char* morse (char c) {
    static const char* morse_ch[] = {
        "._", /* A */
        "_...", /* B */
        "_._.", /* C */
        "_..", /* D */
        ".", /* E */
        ".._.", /* F */
        "__.", /* G */
        "....", /* H */
        "..", /* I */
        ".___", /* J */
        "_._", /* K */
        "._..", /* L */
        "__", /* M */
        "_.", /* N */
        "___", /* O */
        ".__.", /* P */
        "__._", /* Q */
        "._.", /* R */
        "...", /* S */
        "_", /* T */
        ".._", /* U */
        "..._", /* V */
        ".__", /* W */
        "_.._", /* X */
        "_.__", /* Y */
        "__.." /* Z */
    };
    return morse_ch[c - 'a'];
}

int str2morse(char*buf, int n, char*str){
    int i = 0;
    char dest[n];
    for(i = 0; i < n-1; i++) {
        if(*(str+i) != ' '){
            char *var = morse(*(str+i));
            strcat(dest, var);
            buf = dest;
            printf("%s \n", buf);
        }
        else {
            strcat(dest, " ");
            buf = dest;
            printf("%s \n", buf);
        }
    }
    return 0;        
}

void morse_send(const char* msg) {
    PIN_FUNC_SELECT(GPIO_PIN_REG_2, FUNC_GPIO2);
    switch (*msg)
    {
        case '.':
            GPIO_OUTPUT_SET(2, 0);
            vTaskDelay(250/portTICK_RATE_MS);
    	    GPIO_OUTPUT_SET(2, 1);
            vTaskDelay(250/portTICK_RATE_MS);
            break;
        case '_':
            GPIO_OUTPUT_SET(2, 0);
            vTaskDelay(750/portTICK_RATE_MS);
    	    GPIO_OUTPUT_SET(2, 1);
            vTaskDelay(250/portTICK_RATE_MS);
            break;
        case ' ':
            vTaskDelay(1000/portTICK_RATE_MS);
            break;
        case '\0':
            break;
        default :
            break;
    }
    morse_send(++msg);
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
    xTaskCreate(&task_blink, "startup", 2048, NULL, 1, NULL);
}

