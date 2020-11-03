#include "main.h"
#include "gpio.h"
#include "spi.h"

#include "w25qxx.h"

/* External function prototypes -----------------------------------------------*/

void
SystemClock_Config(void);

/* --------------------------------------------------------------------------- */

/**
 * Description :
 * Initialize the MCU Clock, the GPIO Pins corresponding to the
 * device and initialize the FSMC with the chosen configuration
 * Inputs:
 *      None
 * outputs:
 *      R0: "1" - Operation succeeded
 * 			"0" - Operation failure
 * Note: Mandatory for all types of device
 */
int
Init(void)
{
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
    if (w25qxx.ID != W25Q16)
    {
        SET_DO(BUZER_GPIO_Port, BUZER_Pin);
        if (!W25qxx_Init())
        {
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
 *      Address : Read location
 *      Size    : Length in bytes
 *      buffer  : Address where to put data
 * Outputs:
 *      R0: "1" - Operation succeeded
 * 			"0" - Operation failure
 * Note: Mandatory for all types except SRAM and PSRAM
 */
int
Read(uint32_t Address, uint32_t Size, uint8_t *buffer)
{
    if (!w25qxx.ID || (Address + Size) > w25qxx.CapacityInKiloByte * 1024)
    {
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
 *      R0: "1" - Operation succeeded
 *          "0" - Operation failure
 * Note: Mandatory for all types except SRAM and PSRAM
 */
int
Write(uint32_t Address, uint32_t Size, uint8_t *buffer)
{
    if (!w25qxx.ID || (Address + Size) > w25qxx.CapacityInKiloByte * 1024)
    {
        LED_ON(LED_ERR_GPIO_Port, LED_ERR_Pin);
        return 0;
    }

    LED_OFF(LED_PWR_GPIO_Port, LED_PWR_Pin);
    LED_OFF(LED_ERR_GPIO_Port, LED_ERR_Pin);

    uint32_t written_bytes = 0;
    uint32_t page = Address / w25qxx.PageSize;
    uint16_t offset = Address % w25qxx.PageSize;
    uint16_t chunk;

    while (written_bytes < Size)
    {
        chunk = (w25qxx.PageSize - offset > Size - written_bytes) ?
            Size - written_bytes : w25qxx.PageSize - offset;

        W25qxx_WritePage(buffer + written_bytes, page, offset, chunk);

        if (written_bytes % w25qxx.SectorSize == 0)
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
 *      R0: "1" - Operation succeeded
 *          "0" - Operation failure
 * Note: Not Mandatory for SRAM PSRAM and NOR_FLASH
 */
int
MassErase(void)
{
    if (!w25qxx.ID)
    {
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
 *      R0: "1" - Operation succeeded
 *          "0" - Operation failure
 * Note: Not Mandatory for SRAM PSRAM and NOR_FLASH
 */
int
SectorErase(uint32_t EraseStartAddress, uint32_t EraseEndAddress)
{
    if (!w25qxx.ID ||
        EraseEndAddress < EraseStartAddress ||
        EraseEndAddress > w25qxx.CapacityInKiloByte * 1024)
    {
        LED_ON(LED_ERR_GPIO_Port, LED_ERR_Pin);
        return 0;
    }

    LED_OFF(LED_PWR_GPIO_Port, LED_PWR_Pin);
    LED_OFF(LED_ERR_GPIO_Port, LED_ERR_Pin);

    EraseStartAddress -= EraseStartAddress % w25qxx.SectorSize;

    while (EraseEndAddress >= EraseStartAddress)
    {
        W25qxx_EraseSector(EraseStartAddress / w25qxx.SectorSize);
        LL_GPIO_TogglePin(LED_PWR_GPIO_Port, LED_PWR_Pin);

        EraseStartAddress += w25qxx.SectorSize;
    }

    LED_ON(LED_PWR_GPIO_Port, LED_PWR_Pin);
    return 1;
}

/**
 * Description :
 * Calculates checksum value of the memory zone
 * Inputs:
 *      StartAddress  : Flash start address
 *      Size          : Size (in WORD)
 *      InitVal       : Initial CRC value
 * Outputs:
 *     R0             : Checksum value
 * Note: Optional for all types of device
 */
uint32_t
CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal)
{
    uint8_t missalignementAddress = StartAddress % 4;
    uint8_t missalignementSize = Size;
    int cnt;
    uint32_t Val;
    uint8_t value;

    StartAddress -= StartAddress % 4;
    Size += (Size % 4 == 0) ? 0 : 4 - (Size % 4);

    for (cnt = 0; cnt < Size; cnt += 4)
    {
        W25qxx_ReadBytes(&value, StartAddress, 1);
        Val = value;
        W25qxx_ReadBytes(&value, StartAddress + 1, 1);
        Val += value << 8;
        W25qxx_ReadBytes(&value, StartAddress + 2, 1);
        Val += value << 16;
        W25qxx_ReadBytes(&value, StartAddress + 3, 1);
        Val += value << 24;
        if (missalignementAddress)
        {
            switch (missalignementAddress)
            {
                case 1:
                    InitVal += (uint8_t) (Val >> 8 & 0xff);
                    InitVal += (uint8_t) (Val >> 16 & 0xff);
                    InitVal += (uint8_t) (Val >> 24 & 0xff);
                    missalignementAddress -= 1;
                    break;
                case 2:
                    InitVal += (uint8_t) (Val >> 16 & 0xff);
                    InitVal += (uint8_t) (Val >> 24 & 0xff);
                    missalignementAddress -= 2;
                    break;
                case 3:
                    InitVal += (uint8_t) (Val >> 24 & 0xff);
                    missalignementAddress -= 3;
                    break;
            }
        }
        else if ((Size - missalignementSize) % 4 && (Size - cnt) <= 4)
        {
            switch (Size - missalignementSize)
            {
                case 1:
                    InitVal += (uint8_t) Val;
                    InitVal += (uint8_t) (Val >> 8 & 0xff);
                    InitVal += (uint8_t) (Val >> 16 & 0xff);
                    missalignementSize -= 1;
                    break;
                case 2:
                    InitVal += (uint8_t) Val;
                    InitVal += (uint8_t) (Val >> 8 & 0xff);
                    missalignementSize -= 2;
                    break;
                case 3:
                    InitVal += (uint8_t) Val;
                    missalignementSize -= 3;
                    break;
            }
        }
        else
        {
            InitVal += (uint8_t) Val;
            InitVal += (uint8_t) (Val >> 8 & 0xff);
            InitVal += (uint8_t) (Val >> 16 & 0xff);
            InitVal += (uint8_t) (Val >> 24 & 0xff);
        }
        StartAddress += 4;
    }
    return (InitVal);
}

/**
 * Description :
 * Verify flash memory with RAM buffer and calculates checksum value of
 * the programmed memory
 * Inputs:
 *      FlashAddr     : Flash address
 *      RAMBufferAddr : RAM buffer address
 *      Size          : Size (in WORD)..
 *      InitVal       : Initial CRC value
 * Outputs:
 *      R0            : Operation failed (address of failure)
 *      R1            : Checksum value
 * Note: Optional for all types of device
 */
uint64_t
Verify(uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size, uint32_t missalignement)
{
    uint32_t InitVal = 0;
    uint32_t VerifiedData = 0;
    uint8_t TmpBuffer = 0x00;
    uint64_t checksum;

    Size *= 4;
    checksum = CheckSum((uint32_t) MemoryAddr + (missalignement & 0xf),
                        Size - ((missalignement >> 16) & 0xF),
                        InitVal);

    while (Size > VerifiedData)
    {
        W25qxx_ReadBytes(&TmpBuffer, MemoryAddr + VerifiedData, 1);

        if (TmpBuffer != *((uint8_t*) RAMBufferAddr + VerifiedData))
            return ((checksum << 32) + MemoryAddr + VerifiedData);

        VerifiedData++;
    }

    return (checksum << 32);
}
