#include "core/memory.h"
#include "core/cpu.h"
#include "core/emu.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Global MEMORY state
mem_state_t mem;

static const uint32_t ram_size = 0x65800;
static const uint32_t flash_size = 0x400000;
static const uint32_t flash_sector_size = 0x10000;
static const uint32_t flash_sectors = 64;

void mem_init(void) {
    unsigned int i;

    mem.flash.block = (uint8_t*)malloc(flash_size);               /* allocate Flash memory */
    memset(mem.flash.block, 0xFF, flash_size);

    for (i = 0; i < flash_sectors; i++) {
        mem.flash.sector[i].ptr = mem.flash.block + i;
        mem.flash.sector[i].locked = false;
    }

    mem.ram.block = (uint8_t*)calloc(ram_size, sizeof(uint8_t));  /* allocate RAM */

    mem.flash.mapped = false;
    mem.flash.locked = false;
    mem.flash.status = 0;
    gui_console_printf("Initialized memory...\n");
}

void mem_free(void) {
    free(mem.ram.block);
    free(mem.flash.block);
    gui_console_printf("Freed memory...\n");
}

uint8_t* phys_mem_ptr(uint32_t addr, uint32_t size) {
    if (addr < 0xD00000) {
        return mem.flash.block+addr;
    }
    addr -= 0xD00000;
    return mem.ram.block+addr;
}

static void flash_reset(uint32_t addr, uint8_t byte) {
    (void)addr;
    (void)byte;
    mem.flash.write_index = 0;
}

static void flash_write(uint32_t addr, uint8_t byte) {
    mem.flash.block[addr] &= byte;
}

static void flash_erase(uint32_t addr, uint8_t byte) {
    (void)addr;
    (void)byte;

    memset(mem.flash.block, 0xFF, flash_size);
    gui_console_printf("Erased entire Flash chip.\n");
}

static void flash_erase_sector(uint32_t addr, uint8_t byte) {
    (void)byte;

    /* Reset sector */
    memset(mem.flash.sector[addr / flash_sector_size].ptr, 0xFF, flash_sector_size);
    gui_console_printf("Erased flash sector %02X.\n", addr);
}

struct flash_pattern {
    int length;
    const flash_write_t pattern[6];
    void (*handler)(uint32_t address, uint8_t value);
};
typedef struct flash_pattern flash_pattern_t;

static flash_pattern_t patterns[] = {
    {
        .length = 4,
        .pattern = {
            { .address = 0xAAA, .address_mask = 0xFFF, .value = 0xAA, .value_mask = 0xFF },
            { .address = 0x555, .address_mask = 0xFFF, .value = 0x55, .value_mask = 0xFF },
            { .address = 0xAAA, .address_mask = 0xFFF, .value = 0xA0, .value_mask = 0xFF },
            { .address = 0x000, .address_mask = 0x000, .value = 0x00, .value_mask = 0x00 },
        },
        .handler = flash_write
    },
    {
        .length = 6,
        .pattern = {
            { .address = 0xAAA, .address_mask = 0xFFF, .value = 0xAA, .value_mask = 0xFF },
            { .address = 0x555, .address_mask = 0xFFF, .value = 0x55, .value_mask = 0xFF },
            { .address = 0xAAA, .address_mask = 0xFFF, .value = 0x80, .value_mask = 0xFF },
            { .address = 0xAAA, .address_mask = 0xFFF, .value = 0xAA, .value_mask = 0xFF },
            { .address = 0x555, .address_mask = 0xFFF, .value = 0x55, .value_mask = 0xFF },
            { .address = 0x000, .address_mask = 0x000, .value = 0x00, .value_mask = 0x00 },
        },
        .handler = flash_erase_sector
    },
    {
        .length = 6,
        .pattern = {
            { .address = 0xAAA, .address_mask = 0xFFF, .value = 0xAA, .value_mask = 0xFF },
            { .address = 0x555, .address_mask = 0xFFF, .value = 0x55, .value_mask = 0xFF },
            { .address = 0xAAA, .address_mask = 0xFFF, .value = 0x80, .value_mask = 0xFF },
            { .address = 0xAAA, .address_mask = 0xFFF, .value = 0xAA, .value_mask = 0xFF },
            { .address = 0x555, .address_mask = 0xFFF, .value = 0x55, .value_mask = 0xFF },
            { .address = 0xAAA, .address_mask = 0xFFF, .value = 0x10, .value_mask = 0xFF },
        },
        .handler = flash_erase
    },
    {
        .length = 0
    }

    /* TODO: More flash patterns */

};

static uint8_t flash_read_handler(uint32_t address) {
    if(mem.flash.running_command == false) {
        return mem.flash.block[address];
    } else {
        return mem.flash.status;
    }
}

static void flash_write_handler(uint32_t address, uint8_t byte) {
    int i;
    int partial_match = 0;
    flash_write_t *w;
    struct flash_pattern *pattern;

    gui_console_printf("Flash Chip: Writting %02X -> %06X\t(Sector %02X)\n", byte, address, address / 0x10000);

    w = &mem.flash.writes[mem.flash.write_index++];
    w->address = address;
    w->value = byte;
    for (pattern = patterns; pattern->length; pattern++) {
        for (i = 0; i < mem.flash.write_index && i < pattern->length &&
            (mem.flash.writes[i].address & pattern->pattern[i].address_mask) == pattern->pattern[i].address &&
            (mem.flash.writes[i].value & pattern->pattern[i].value_mask) == pattern->pattern[i].value; i++) {
        }
        if (i == pattern->length) {
            pattern->handler(address, byte);
            partial_match = 0;
            break;
        } else if (i == mem.flash.write_index) {
            partial_match = 1;
        }
    }
    if (!partial_match) {
        flash_reset(address, byte);
    }
}

/* returns wait cycles */
uint8_t memory_read_byte(const uint32_t address)
{
    uint32_t addr;
    addr = address & 0xFFFFFF;

    switch(upperNibble24(addr)) {
        // FLASH
        case 0x0: case 0x1: case 0x2: case 0x3:
            cpu.cycles += 5;
            return flash_read_handler(address);

        // MAYBE FLASH
        case 0x4: case 0x5: case 0x6: case 0x7:
            addr -= 0x400000;
            if (mem.flash.mapped == true) {
                cpu.cycles += 5;
                return flash_read_handler(address);
            }

        // UNMAPPED
        case 0x8: case 0x9: case 0xA: case 0xB: case 0xC:
            cpu.cycles += 257;
            return 0;

        // RAM
        case 0xD:
            addr -= 0xD00000;
            if (addr < 0x65800) {
                cpu.cycles += 3;
                return mem.ram.block[addr];
            }
        // UNMAPPED
            addr -= 0x65800;
            if (addr < 0x1A800) {
                cpu.cycles += 3;
                return 0;
            }
        // MIRRORED
            return memory_read_byte(addr - 0x80000);

        case 0xE: case 0xF:
            cpu.cycles += 2;
            return mmio_read_byte(addr);          // read byte from mmio

        default:
            cpu.cycles += 1;
            break;
    }
    return 0;
}

void memory_write_byte(const uint32_t address, const uint8_t byte) {
    uint32_t addr;
    addr = address & 0xFFFFFF;

    switch(upperNibble24(addr)) {
        // FLASH
        case 0x0: case 0x1: case 0x2: case 0x3:
            if (mem.flash.locked == false) {
                flash_write_handler(addr, byte);
            }
            cpu.cycles += 5;
            return;

        // MAYBE FLASH
        case 0x4: case 0x5: case 0x6: case 0x7:
            addr -= 0x400000;
            if (mem.flash.locked == false) {
                if (mem.flash.mapped == true) {
                    cpu.cycles += 5;
                    flash_write_handler(addr, byte);
                    return;
                }
            }
            cpu.cycles += 257;
            return;

        // UNMAPPED
        case 0x8: case 0x9: case 0xA: case 0xB: case 0xC:
            cpu.cycles += 5;
            return;

        // RAM
        case 0xD:
            addr -= 0xD00000;
            if (addr <= 0x657FF) {
                cpu.cycles += 2;
                mem.ram.block[addr] = byte;
                return;
            }
            // UNMAPPED
            addr -=  0x65800;
            if (addr <= 0x1A7FF) {
                cpu.cycles += 1;
                return;
            }
            // MIRRORED
            memory_write_byte(addr - 0x80000, byte);
            return;

        // MMIO <-> Advanced Perphrial Bus
        case 0xE: case 0xF:
            cpu.cycles += 2;
            mmio_write_byte(addr, byte);         // write byte to the mmio port
            return;

        default:
            cpu.cycles += 1;
            break;
    }
    return;
}
