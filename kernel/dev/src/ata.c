#define HW_IO_IPL
#define DEV_CONSOLE_IPL

#include <hw/mod.h>
#include <dev/mod.h>
#include <lib/mod.h>

#include "ata.h"

uint8_t ata_wait_bsy(void) {
    uint32_t timeout = ATA_WAIT_TIMEOUT;
    while(ATA_READ_PRIMARY_STATUS_R & 0x80) {
        if (ATA_READ_PRIMARY_ERROR_R != 0x00) return FALSE;
        if (--timeout == 0) return FALSE;
    }
    return TRUE;
}

int ata_wait_drq(void) {
    uint32_t timeout = ATA_WAIT_TIMEOUT;
    while(!(ATA_READ_PRIMARY_STATUS_R & 0x08)) {
        if (ATA_READ_PRIMARY_ERROR_R != 0x00) {
            return FALSE;
        }
        if (--timeout == 0) return FALSE;
    }
    return TRUE;
}

uint8_t ata_get_error(void) {
    return ATA_READ_PRIMARY_ERROR_R;
}


int ata_identify(uint16_t *buf) {
    if(!buf) {
        return -1;
    }

    if (!ata_wait_bsy()) return -1;

    //set to master drive
    outb(ATA_PRIMARY_DRIVE_HEAD_REGISTER_RW, 0xE0); // 0b11100000
    outb(ATA_PRIMARY_COMMAND_REGISTER_W, 0xEC);

    int drq_done = ata_wait_drq();
    if (drq_done == FALSE) {
        serial_printf("%s: failed, ata_error: %x\n", __func__, ata_get_error());
        return -1;
    }

    for (uint32_t i = 0; i < 256; ++i) {
        uint16_t bytes = inw(ATA_PRIMARY_DATA_REGISTER_RW);
        buf[i] = bytes;
    }
    return 0;
}

uint8_t ata_read(uint32_t lba, uint8_t sectors_count, uint16_t *buf) {
    if (!buf) {
        return FALSE;
    }

    if (!ata_wait_bsy()) {
        SERIAL_ERROR("ATA driver wait for business exceeded timeout, could not read anything\n");
        return FALSE;
    }

    uint8_t lba_low  = (uint8_t)(lba);
    uint8_t lba_mid  = (uint8_t)(lba >> 8);
    uint8_t lba_high = (uint8_t)(lba >> 16);

    outb(ATA_PRIMARY_SECTOR_COUNT_REGISTER_RW, sectors_count);
    outb(ATA_PRIMARY_LBA_LOW_REGISTER_RW, lba_low);
    outb(ATA_PRIMARY_LBA_MID_REGISTER_RW, lba_mid);
    outb(ATA_PRIMARY_LBA_HIGH_REGISTER_RW, lba_high);
    outb(ATA_PRIMARY_DRIVE_HEAD_REGISTER_RW, (uint8_t)(0xE0 | ((lba >> 24) & 0xf)));

    outb(ATA_PRIMARY_COMMAND_REGISTER_W, 0x20); //READ_SECTORS command

    uint32_t word_count = 0;
    for (uint32_t i = 0; i < sectors_count; ++i) {
        int drq_done = ata_wait_drq();
        if (!drq_done) {
            serial_printf("%s: failed for lba: %x, %d sectors, with ata_error: %x\n", __func__, lba, (uint32_t)sectors_count, ata_get_error());
            return FALSE;
        }
        for (uint32_t j = 0; j < 256; ++j) {
            buf[word_count++] = inw(ATA_PRIMARY_DATA_REGISTER_RW);
        }
    }
    return TRUE;
}


uint8_t ata_write(uint32_t lba, uint8_t count, uint16_t *buf) {
    if (!buf) {
        return FALSE;
    }

    if (!ata_wait_bsy()) {
        SERIAL_ERROR("ATA driver wait for business exceeded timeout, could not write anything\n");
        return FALSE;
    }

    uint8_t lba_low  = (uint8_t)(lba);
    uint8_t lba_mid  = (uint8_t)(lba >> 8);
    uint8_t lba_high = (uint8_t)(lba >> 16);

    outb(ATA_PRIMARY_SECTOR_COUNT_REGISTER_RW, count);
    outb(ATA_PRIMARY_LBA_LOW_REGISTER_RW, lba_low);
    outb(ATA_PRIMARY_LBA_MID_REGISTER_RW, lba_mid);
    outb(ATA_PRIMARY_LBA_HIGH_REGISTER_RW, lba_high);
    outb(ATA_PRIMARY_DRIVE_HEAD_REGISTER_RW, (uint8_t)(0xE0 | ((lba >> 24) & 0xf)));

    outb(ATA_PRIMARY_COMMAND_REGISTER_W, 0x30); //WRITE_SECTORS command

    uint32_t word_count = 0;
    for (uint32_t i = 0; i < count; ++i) {
        int drq_done = ata_wait_drq();
        if (!drq_done) {
            serial_printf("%s: failed for lba: %x, %d sectors, with ata_error: %x\n", __func__ ,lba, (uint32_t)count, ata_get_error());
            return FALSE;
        }
        for (uint32_t j = 0; j < 256; ++j) {
            outw(ATA_PRIMARY_DATA_REGISTER_RW, buf[word_count++]);
            ATA_WRITE_WAIT();
        }
    }

    outb(ATA_PRIMARY_COMMAND_REGISTER_W, 0xE7); //FLUSH
    if (!ata_wait_bsy()) {
        SERIAL_WARN("ATA driver wait for business exceeded timeout, driver may not have flushed after operation\n");
    }
    return TRUE;
}
