/*
 * AN2594.h
 *
 *  Created on: Nov 3, 2020
 *      Author: sergi
 */

#ifndef INC_AN2594_H_
#define INC_AN2594_H_

/*
 * File:   IntAN2594.h
 * Author: rudenkos
 *
 * Created on December 3, 2018, 4:51 PM
 */

#define AN2594_EEPROM_BASE_ADDRESS ((uint32_t)(FLASH_BASE + (Mcu.FLASH_PAGE_COUNT - 2) * FLASH_PAGE_SIZE))
#define AN2594_PAGE0_BASE_ADDRESS  ((uint32_t)(AN2594_EEPROM_BASE_ADDRESS))
#define AN2594_PAGE0_END_ADDRESS   ((uint32_t)(AN2594_PAGE0_BASE_ADDRESS + FLASH_PAGE_SIZE - 1))
#define AN2594_PAGE1_BASE_ADDRESS  ((uint32_t)(AN2594_PAGE0_BASE_ADDRESS + FLASH_PAGE_SIZE))
#define AN2594_PAGE1_END_ADDRESS   ((uint32_t)(AN2594_PAGE1_BASE_ADDRESS + FLASH_PAGE_SIZE - 1))

//    /* ctor:: */
//    IntAN2594(Byte_t eeprom_size = 128U,
//    FunctionalState fs = DISABLE)
//    {
//        debug_printf("EEPROM_START_ADDRESS: 0x%08X\n",
//        AN2594_EEPROM_BASE_ADDRESS);
//
//        ErrorStatus result;
//
//        /* Р’РёСЂС‚СѓР°Р»СЊРЅС‹Р№ Р°РґСЂРµСЃ 0xFFFF Р·Р°РїСЂРµС‰РµРЅ!
//         * Рў.Рµ. РјР°РєСЃРёРјР°Р»СЊРЅС‹Р№ Р°РґСЂРµСЃ <= 0xFFFE */
//
//        //m_eeprom_size = (eeprom_size <= 0xFFFE) ? eeprom_size : 0xFFFE;
//        m_eeprom_size = (eeprom_size >= FLASH_PAGE_SIZE) ?
//        FLASH_PAGE_SIZE - 1 : eeprom_size;
//
//        m_State = EEPROM::State::Initialization;
//
//        result = LowLevel::FLASH_Unlock();
//        if (result == SUCCESS)
//        {
//            result = AN2594_Init();
//            if (result == SUCCESS)
//            m_State = EEPROM::State::Ready;
//            else
//            m_State = EEPROM::State::Error;
//        }
//
//        if (fs == DISABLE) disable();
//    }
//
//    /* dtor:: */
//    ~IntAN2594()
//    {
//        disable();
//        Event.emit(EEPROM::Events::Destroyed, 0);
//    }
//
//    void
//    enable() override
//    {
//        //debug_printf("EepromIntFlash: enabled.\n");
//        LowLevel::FLASH_Unlock();
//        setEepromState(EEPROM::State::Ready);
//    }
//
//    void
//    disable() override
//    {
//        //debug_printf("EepromIntFlash: disabled.\n");
//        LowLevel::FLASH_Lock();
//        if (m_State != EEPROM::State::Error)
//        setEepromState(EEPROM::State::Disabled);
//    }

ErrorStatus
write(uint32_t eeprom_addr, void *src_ptr, size_t data_size)
{
    ErrorStatus result = ERROR;

    uint16_t AN2594_address;
    uint16_t current_data;

    if ((src_ptr != nullptr) && (data_size > 0))
    {
        for (size_t i = 0; i < data_size; i++)
        {
            m_eeprom_address = (eeprom_addr + i);

            if (m_eeprom_address < m_eeprom_size)
            {
                AN2594_address = m_eeprom_address & ~(0x01);
                AN2594_ReadVariable(AN2594_address, &current_data);
                m_data = current_data;

                if ((i + 1) < data_size)
                {
                    // odd & even
                    m_data &= 0x0000;
                    m_data |= *(uint8_t*) ((uint8_t*) src_ptr + i++);
                    m_data |= *(uint8_t*) ((uint8_t*) src_ptr + i) << 8;
                }
                else if (m_eeprom_address != AN2594_address)
                {
                    // odd
                    m_data &= 0xFF00;
                    m_data |= *(uint8_t*) ((uint8_t*) src_ptr + i);
                }
                else
                {
                    // even
                    m_data &= 0x00FF;
                    m_data |= *(uint8_t*) ((uint8_t*) src_ptr + i) << 8;
                }

                if (m_data != current_data)
                {
                    result = AN2594_WriteVariable(AN2594_address, m_data);
                    if (result != SUCCESS) break;
                }
                else
                    result = SUCCESS;
            }
        }
    }
    return result;
}

ErrorStatus
read(uint32_t eeprom_addr, void *dst_ptr, size_t data_size)
{
    ErrorStatus result = ERROR;

    uint16_t AN2594_address;

    if ((dst_ptr != nullptr) && (data_size > 0))
    {
        for (size_t i = 0; i < data_size; i++)
        {
            m_eeprom_address = (eeprom_addr + i);

            if (m_eeprom_address < m_eeprom_size)
            {
                AN2594_address = m_eeprom_address & ~(0x01);
                AN2594_ReadVariable(AN2594_address, &m_data);
            }
            else
                m_data = 0xFFFF;

            if ((i + 1) < data_size)
            {
                // odd & even
                *(uint8_t*) ((uint8_t*) dst_ptr + i++) = uint8_t(m_data);
                *(uint8_t*) ((uint8_t*) dst_ptr + i) = uint8_t(m_data >> 8);
            }
            else if (m_eeprom_address != AN2594_address)
                // odd
                *(uint8_t*) ((uint8_t*) dst_ptr + i) = uint8_t(m_data);
            else
                // even
                *(uint8_t*) ((uint8_t*) dst_ptr + i) = uint8_t(m_data >> 8);
        }
        result = SUCCESS;
    }
    return result;
}

uint8_t
readByte(uint32_t eeprom_addr)
{
    uint16_t AN2594_address = eeprom_addr & ~(0x01);

    if (AN2594_ReadVariable(AN2594_address, &m_data) == SUCCESS)
        return uint8_t((eeprom_addr == AN2594_address) ? m_data >> 8 : m_data);

    return (0xFF);
}

enum AN2594_Pages
{
    PAGE0 = 0x0000,
    PAGE1 = 0x0001,
    NO_VALID_PAGE = 0x00AB,
};

enum AN2594_Operations
{
    READ_FROM_VALID_PAGE = 0x00,
    WRITE_IN_VALID_PAGE = 0x01,
};

enum AN2594_Page_Status
{
    VALID_PAGE = ((uint16_t) 0x0000), /* PAGE containing valid data */
    RECEIVE_DATA = ((uint16_t) 0xEEEE), /* PAGE is marked to receive data */
    ERASED = ((uint16_t) 0xFFFF), /* PAGE is empty */
};

ErrorStatus
AN2594_Init()
{
    ErrorStatus FlashStatus;
    ErrorStatus EepromStatus;
    ErrorStatus ReadStatus;

    uint16_t VarIdx = 0;
    int16_t x = -1;

    /* Get Page0 status */
    uint16_t
    PageStatus0 = *(uint16_t*) AN2594_PAGE0_BASE_ADDRESS;
    /* Get Page1 status */
    uint16_t
    PageStatus1 = *(uint16_t*) AN2594_PAGE1_BASE_ADDRESS;

    switch (PageStatus0)
    {
        case ERASED:
            if (PageStatus1 == VALID_PAGE) /* Page0 erased, Page1 valid */
            {
                /* Erase Page0 (FIX: if necessary...) */
                FlashStatus = LowLevel_FLASH_PageErase(AN2594_PAGE0_BASE_ADDRESS);
                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
            }
            else if (PageStatus1 == RECEIVE_DATA) /* Page0 erased, Page1 receive */
            {
                /* Erase Page0 */
                FlashStatus = LowLevel_FLASH_PageErase(AN2594_PAGE0_BASE_ADDRESS);
                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
                /* Mark Page1 as valid */
                FlashStatus = LowLevel_FLASH_Program_HalfWord(AN2594_PAGE1_BASE_ADDRESS, VALID_PAGE);
                /* If program operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
            }
            else /* First EEPROM access (Page0&1 are erased) or invalid state -> format EEPROM */
            {
                /* Erase both Page0 and Page1 and set Page0 as valid page */
                FlashStatus = AN2594_Format();
                /* If erase/program operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
            }
            break;

        case RECEIVE_DATA:
            if (PageStatus1 == VALID_PAGE) /* Page0 receive, Page1 valid */
            {
                /* Transfer Data */
                FlashStatus = AN2594_PageTransfer(0, 0);
                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
            }
            else if (PageStatus1 == ERASED) /* Page0 receive, Page1 erased */
            {
                /* Erase Page1 */
                FlashStatus = LowLevel_FLASH_PageErase(AN2594_PAGE1_BASE_ADDRESS);
                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
                /* Mark Page0 as valid */
                FlashStatus = LowLevel_FLASH_Program_HalfWord(AN2594_PAGE0_BASE_ADDRESS, VALID_PAGE);
                /* If program operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
            }
            else /* Invalid state -> format eeprom */
            {
                /* Erase both Page0 and Page1 and set Page0 as valid page */
                FlashStatus = AN2594_Format();
                /* If erase/program operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
            }
            break;

        case VALID_PAGE:
            if (PageStatus1 == VALID_PAGE) /* Invalid state -> format eeprom */
            {
                /* Erase both Page0 and Page1 and set Page0 as valid page */
                FlashStatus = AN2594_Format();
                /* If erase/program operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
            }
            else if (PageStatus1 == ERASED) /* Page0 valid, Page1 erased */
            {
                /* Erase Page1 */
                FlashStatus = LowLevel
                ::FLASH_PageErase(AN2594_PAGE1_BASE_ADDRESS);
                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
            }
            else /* Page0 valid, Page1 receive */
            {
                /* Transfer Data */
                FlashStatus = AN2594_PageTransfer(0, 0);
                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != SUCCESS)
                {
                    return FlashStatus;
                }
            }
            break;

        default: /* Any other state -> format eeprom */
            /* Erase both Page0 and Page1 and set Page0 as valid page */
            FlashStatus = AN2594_Format();
            /* If erase/program operation was failed, a Flash error code is returned */
            if (FlashStatus != SUCCESS)
            {
                return FlashStatus;
            }
            break;
    }

    return SUCCESS;
}

/**
 * @brief  Erases PAGE0 and PAGE1 and writes VALID_PAGE header to PAGE0
 * @param  None
 * @retval Status of the last operation (Flash write or erase) done during
 *         EEPROM formating
 */
ErrorStatus
AN2594_Format(void)
{
    ErrorStatus FlashStatus;

    /* Erase Page0 */
    FlashStatus = LowLevel
    ::FLASH_PageErase(AN2594_PAGE0_BASE_ADDRESS);

    /* If erase operation was failed, a Flash error code is returned */
    if (FlashStatus != SUCCESS)
    {
        return FlashStatus;
    }
    /* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
    FlashStatus = LowLevel
    ::FLASH_Program_HalfWord(AN2594_PAGE0_BASE_ADDRESS, VALID_PAGE);

    /* If program operation was failed, a Flash error code is returned */
    if (FlashStatus != SUCCESS)
    {
        return FlashStatus;
    }
    /* Erase Page1 */
    FlashStatus = LowLevel
    ::FLASH_PageErase(AN2594_PAGE1_BASE_ADDRESS);

    /* Return Page1 erase operation status */
    return FlashStatus;
}

/**
 * @brief  Verify if active page is full and Writes variable in EEPROM.
 * @param  VirtAddress: 16 bit virtual address of the variable
 * @param  Data: 16 bit data to be written as variable value
 * @retval Success or error status:
 */
ErrorStatus
AN2594_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data)
{
    ErrorStatus FlashStatus;

    uint16_t ValidPage;
    uint32_t Address;
    uint32_t PageEndAddress;

    /* Get valid Page for write operation */
    ValidPage = AN2594_FindValidPage(WRITE_IN_VALID_PAGE);

    /* Check if there is no valid page */
    if (ValidPage == NO_VALID_PAGE)
    {
        return ERROR;
    }

    /* Get the valid Page start Address */
    Address =
    (uint32_t) (AN2594_EEPROM_BASE_ADDRESS +
        (uint32_t) (ValidPage * FLASH_PAGE_SIZE));

    /* Get the valid Page end Address */
    PageEndAddress =
    (uint32_t) ((AN2594_EEPROM_BASE_ADDRESS - 2) +
        (uint32_t) ((1 + ValidPage) * FLASH_PAGE_SIZE));

    /* Check each active page address starting from begining */
    while (Address < PageEndAddress)
    {
        /* Verify if Address and Address+2 contents are 0xFFFFFFFF */
        if ((*( __IO uint32_t* ) Address) == 0xFFFFFFFF)
        {
            /* Set variable data */
            FlashStatus = LowLevel
            ::FLASH_Program_HalfWord(Address, Data);
            /* If program operation was failed, a Flash error code is returned */
            if (FlashStatus != SUCCESS)
            {
                return FlashStatus;
            }
            /* Set variable virtual address */
            FlashStatus = LowLevel
            ::FLASH_Program_HalfWord(Address + 2, VirtAddress);
            /* Return program operation status */
            return FlashStatus;
        }
        else
        {
            /* Next address location */
            Address = Address + 4;
        }
    }

    /* Return PAGE_FULL in case the valid page is full */
    return ERROR;
}

/**
 * @brief  Transfers last updated variables data from the full Page to
 *   an empty one.
 * @param  VirtAddress: 16 bit virtual address of the variable
 * @param  Data: 16 bit data to be written as variable value
 * @retval Success or error status:
 */
ErrorStatus
AN2594_PageTransfer(uint16_t VirtAddress, uint16_t Data)
{
    ErrorStatus FlashStatus;
    ErrorStatus EepromStatus;
    ErrorStatus ReadStatus;

    uint32_t NewPageAddress;
    uint32_t OldPageAddress;
    uint16_t ValidPage;
    uint16_t VarIdx;

    /* Get active Page for read operation */
    ValidPage = AN2594_FindValidPage(READ_FROM_VALID_PAGE);

    if (ValidPage == PAGE1) /* Page1 valid */
    {
        /* New page address where variable will be moved to */
        NewPageAddress = AN2594_PAGE0_BASE_ADDRESS;

        /* Old page address where variable will be taken from */
        OldPageAddress = AN2594_PAGE1_BASE_ADDRESS;
    }
    else if (ValidPage == PAGE0) /* Page0 valid */
    {
        /* New page address where variable will be moved to */
        NewPageAddress = AN2594_PAGE1_BASE_ADDRESS;

        /* Old page address where variable will be taken from */
        OldPageAddress = AN2594_PAGE0_BASE_ADDRESS;
    }
    else
    {
        return ERROR; /* No valid Page */
    }

    /* Erase New Page */
    FlashStatus = LowLevel
    ::FLASH_PageErase(NewPageAddress);
    /* If erase operation was failed, a Flash error code is returned */
    if (FlashStatus != SUCCESS)
    {
        return FlashStatus;
    }

    /* Set the new Page status to RECEIVE_DATA status */
    FlashStatus = LowLevel
    ::FLASH_Program_HalfWord(NewPageAddress, RECEIVE_DATA);
    /* If program operation was failed, a Flash error code is returned */
    if (FlashStatus != SUCCESS)
    {
        return FlashStatus;
    }

    if ((VirtAddress != 0) && (Data != 0))
    {
        /* Write the variable passed as parameter in the new active page */
        EepromStatus = AN2594_VerifyPageFullWriteVariable(VirtAddress, Data);
        /* If program operation was failed, a Flash error code is returned */
        if (EepromStatus != SUCCESS)
        {
            return EepromStatus;
        }
    }

    /* Transfer process: transfer variables from old to the new active page */
    for (VarIdx = 0; VarIdx < m_eeprom_size; VarIdx++)
    {
        if (VarIdx != VirtAddress) /* Check each variable except the one passed as parameter */
        {
            /* Read the other last variable updates */
            ReadStatus = AN2594_ReadVariable(VarIdx, &m_data);
            /* In case variable corresponding to the virtual address was found */
            if ((ReadStatus != ERROR) && (m_data != 0))
            {
                /* Transfer the variable to the new active page */
                EepromStatus = AN2594_VerifyPageFullWriteVariable(VarIdx, m_data);
                /* If program operation was failed, a Flash error code is returned */
                if (EepromStatus != SUCCESS)
                {
                    return EepromStatus;
                }
            }
        }
    }

    /* Erase the old Page: Set old Page status to ERASED status */
    FlashStatus = LowLevel
    ::FLASH_PageErase(OldPageAddress);
    /* If erase operation was failed, a Flash error code is returned */
    if (FlashStatus != SUCCESS)
    {
        return FlashStatus;
    }

    /* Set new Page status to VALID_PAGE status */
    FlashStatus = LowLevel
    ::FLASH_Program_HalfWord(NewPageAddress, VALID_PAGE);

    /* Return last operation flash status */
    return FlashStatus;
}

/**
 * @brief  Returns the last stored variable data, if found, which correspond to
 *         the passed virtual address
 * @param  VirtAddress: Variable virtual address
 * @param  Data: Global variable contains the read variable value
 * @retval Success or error status:
 *           - SUCCESS: if variable was found
 *           - ERROR:   if the variable was not found
 */
ErrorStatus
AN2594_ReadVariable(uint16_t VirtAddress, uint16_t *Data)
{
    ErrorStatus status = ERROR;

    uint16_t ValidPage;
    uint16_t AddressValue;
    uint32_t Address;
    uint32_t PageStartAddress;

    /* Get active Page for read operation */
    ValidPage = AN2594_FindValidPage(READ_FROM_VALID_PAGE);

    /* Check if there is no valid page */
    if (ValidPage == NO_VALID_PAGE)
    {
        return ERROR;
    }

    /* Get the valid Page start Address */
    PageStartAddress =
    (uint32_t) (AN2594_EEPROM_BASE_ADDRESS +
        (uint32_t) (ValidPage * FLASH_PAGE_SIZE));

    /* Get the valid Page end Address */
    Address =
    (uint32_t) ((AN2594_EEPROM_BASE_ADDRESS - 2) +
        (uint32_t) ((1 + ValidPage) * FLASH_PAGE_SIZE));

    /* Check each active page address starting from end */
    while (Address > (PageStartAddress + 2))
    {
        /* Get the current location content to be compared with virtual address */
        AddressValue = (*( __IO uint16_t* ) Address);

        /* Compare the read address with the virtual address */
        if (AddressValue == VirtAddress)
        {
            /* Get content of Address-2 which is variable value */
            *Data = (*( __IO uint16_t* ) (Address - 2));

            /* In case variable value is read, reset status flag */
            status = SUCCESS;
            break;
        }
        else
        {
            /* Next address location */
            Address = Address - 4;
        }
    }

    /* Default value, if requested virtual address is not found */
    if (status != SUCCESS) *Data = 0xFFFFU;

    return SUCCESS;
}

/**
 * @brief  Writes/upadtes variable data in EEPROM.
 * @param  VirtAddress: Variable virtual address
 * @param  Data: 16 bit data to be written
 * @retval Success or error status:
 *           - FLASH_COMPLETE: on success
 *           - PAGE_FULL: if valid page is full
 *           - NO_VALID_PAGE: if no valid page was found
 *           - Flash error code: on write Flash error
 */
ErrorStatus
AN2594_WriteVariable(uint16_t VirtAddress, uint16_t Data)
{
    ErrorStatus status;

    /* Write the variable virtual address and value in the EEPROM */
    status = AN2594_VerifyPageFullWriteVariable(VirtAddress, Data);

    /* In case the EEPROM active page is full */
    if (status == ERROR)
    {
        /* Perform Page transfer */
        status = AN2594_PageTransfer(VirtAddress, Data);
    }

    /* Return last operation status */
    return status;
}

/**
 * @brief  Find valid Page for write or read operation
 * @param  Operation: operation to achieve on the valid page.
 *   This parameter can be one of the following values:
 *     @arg READ_FROM_VALID_PAGE: read operation from valid page
 *     @arg WRITE_IN_VALID_PAGE: write operation from valid page
 * @retval Valid page number (PAGE0 or PAGE1) or NO_VALID_PAGE in case
 *   of no valid page was found
 */
uint16_t
AN2594_FindValidPage(uint8_t Operation)
{
    uint16_t PageStatus0;
    uint16_t PageStatus1;

    /* Get Page0 actual status */
    PageStatus0 = (*( __IO uint16_t* ) AN2594_PAGE0_BASE_ADDRESS);

    /* Get Page1 actual status */
    PageStatus1 = (*( __IO uint16_t* ) AN2594_PAGE1_BASE_ADDRESS);

    /* Write or read operation */
    switch (Operation)
    {
        case WRITE_IN_VALID_PAGE: /* ---- Write operation ---- */
            if (PageStatus1 == VALID_PAGE)
            {
                /* Page0 receiving data */
                if (PageStatus0 == RECEIVE_DATA)
                {
                    return PAGE0; /* Page0 valid */
                }
                else
                {
                    return PAGE1; /* Page1 valid */
                }
            }
            else if (PageStatus0 == VALID_PAGE)
            {
                /* Page1 receiving data */
                if (PageStatus1 == RECEIVE_DATA)
                {
                    return PAGE1; /* Page1 valid */
                }
                else
                {
                    return PAGE0; /* Page0 valid */
                }
            }
            else
            {
                return NO_VALID_PAGE; /* No valid Page */
            }

        case READ_FROM_VALID_PAGE: /* ---- Read operation ---- */
            if (PageStatus0 == VALID_PAGE)
            {
                return PAGE0; /* Page0 valid */
            }
            else if (PageStatus1 == VALID_PAGE)
            {
                return PAGE1; /* Page1 valid */
            }
            else
            {
                return NO_VALID_PAGE; /* No valid Page */
            }

        default:
            return PAGE0; /* Page0 valid */
    }
}

};

#endif /* INC_AN2594_H_ */
