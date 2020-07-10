#include "main.h"
#include "gpio.h"
#include "spi.h"

#include "w25qxx.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/**
 * Description :
 * Initilize the MCU Clock, the GPIO Pins corresponding to the
 * device and initilize the FSMC with the chosen configuration
 * Inputs    :
 *      None
 * outputs   :
 *      R0             : "1" 			: Operation succeeded
 * 			  "0" 			: Operation failure
 * Note: Mandatory for all types of device
 */
int Init(void) {
    SystemInit();

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_SPI2_Init();

    LL_SPI_SetRxFIFOThreshold(SPI2, LL_SPI_RX_FIFO_TH_QUARTER);
    LL_SPI_Enable(SPI2);

    LED_OFF(LED_PWR_GPIO_Port, LED_PWR_Pin);
    LED_OFF(LED_ERR_GPIO_Port, LED_ERR_Pin);

    /* Initialize W25Q16 */
    if (w25qxx.ID != W25Q16) {
        SET_DO(BUZER_GPIO_Port, BUZER_Pin);
        if (!W25qxx_Init()) {
            LED_ON(LED_ERR_GPIO_Port, LED_ERR_Pin);
        }
        RESET_DO(BUZER_GPIO_Port, BUZER_Pin);
    }

    /* Power LED */
    LED_ON(LED_PWR_GPIO_Port, LED_PWR_Pin);
    return w25qxx.ID != 0;
}

/**
 * Description :
 * Read data from the device
 * Inputs:
 *      Address : Write location
 *      Size    : Length in bytes
 *      buffer  : Address where to get the data to write
 * Outputs:
 *      R0: "1" : Operation succeeded
 * 			"0" : Operation failure
 * Note:Mandatory for all types except SRAM and PSRAM
 */
int Read(uint32_t Address, uint32_t Size, uint8_t *buffer) {
    if (!w25qxx.ID || (Address + Size) > w25qxx.CapacityInKiloByte * 1024) {
        LED_ON(LED_ERR_GPIO_Port, LED_ERR_Pin);
        return 0;
    }

    LED_OFF(LED_PWR_GPIO_Port, LED_PWR_Pin);
    LED_OFF(LED_ERR_GPIO_Port, LED_ERR_Pin);

    W25qxx_ReadBytes(buffer, Address, Size);

    LED_ON(LED_PWR_GPIO_Port, LED_PWR_Pin);
    return 1;
}

/**
 * Description :
 * Write data to the device
 * Inputs:
 *      Address : Write location
 *      Size    : Length in bytes
 *      buffer  : Address where to get the data to write
 * Outputs:
 *      R0: "1" : Operation succeeded
 *          "0" : Operation failure
 * Note: Mandatory for all types except SRAM and PSRAM
 */
int Write(uint32_t Address, uint32_t Size, uint8_t *buffer) {
    if (!w25qxx.ID || (Address + Size) > w25qxx.CapacityInKiloByte * 1024) {
        LED_ON(LED_ERR_GPIO_Port, LED_ERR_Pin);
        return 0;
    }

    LED_OFF(LED_PWR_GPIO_Port, LED_PWR_Pin);
    LED_OFF(LED_ERR_GPIO_Port, LED_ERR_Pin);

    uint32_t written_bytes = 0;
    uint32_t page = Address / w25qxx.PageSize;
    uint16_t offset = Address % w25qxx.PageSize;
    uint16_t chunk;

    while (written_bytes < Size) {
        chunk = (w25qxx.PageSize - offset > Size - written_bytes) ?
                Size - written_bytes : w25qxx.PageSize - offset;

        W25qxx_WritePage(buffer + written_bytes, page, offset, chunk);
        LL_GPIO_TogglePin(LED_PWR_GPIO_Port, LED_PWR_Pin);

        written_bytes += chunk;
        offset = 0;
        page++;
    }

    LED_ON(LED_PWR_GPIO_Port, LED_PWR_Pin);
    return 1;
}

/**
 * Description :
 * Erase all sectors in the device
 * Inputs:
 *     None
 * Outputs:
 *      R0: "1" : Operation succeeded
 *          "0" : Operation failure
 * Note: Not Mandatory for SRAM PSRAM and NOR_FLASH
 */
int MassErase(void) {
    if (!w25qxx.ID) {
        LED_ON(LED_ERR_GPIO_Port, LED_ERR_Pin);
        return 0;
    }

    LED_OFF(LED_PWR_GPIO_Port, LED_PWR_Pin);
    LED_OFF(LED_ERR_GPIO_Port, LED_ERR_Pin);

    W25qxx_EraseChip();

    LED_ON(LED_PWR_GPIO_Port, LED_PWR_Pin);
    return 1;
}

/**
 * Description :
 * Erase a number of sectors in the device
 * Inputs:
 *      EraseStartAddress	: Start address
 *      EraseEndAddress     : End address
 * Outputs:
 *      R0: "1" : Operation succeeded
 *          "0" : Operation failure
 * Note: Not Mandatory for SRAM PSRAM and NOR_FLASH
 */
int SectorErase(uint32_t EraseStartAddress, uint32_t EraseEndAddress) {
    if (!w25qxx.ID || EraseEndAddress < EraseStartAddress
            || EraseEndAddress > w25qxx.CapacityInKiloByte * 1024) {
        LED_ON(LED_ERR_GPIO_Port, LED_ERR_Pin);
        return 0;
    }

    LED_OFF(LED_PWR_GPIO_Port, LED_PWR_Pin);
    LED_OFF(LED_ERR_GPIO_Port, LED_ERR_Pin);

    EraseStartAddress = EraseStartAddress
            - EraseStartAddress % w25qxx.SectorSize;
    while (EraseEndAddress >= EraseStartAddress) {
        W25qxx_EraseSector(EraseStartAddress);
        LL_GPIO_TogglePin(LED_PWR_GPIO_Port, LED_PWR_Pin);

        EraseStartAddress += w25qxx.SectorSize;
    }

    LED_ON(LED_PWR_GPIO_Port, LED_PWR_Pin);
    return 1;
}
