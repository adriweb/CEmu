#ifndef H_USB
#define H_USB

#ifdef __cplusplus
extern "C" {
#endif

#include "../port.h"
#include "fotg210.h"

/* Standard USB state */
typedef struct usb_state {
    struct fotg210_regs regs;
    uint8_t ep0_data[8]; /* 0x1d0: EP0 Setup Packet PIO Register */
    uint8_t ep0_idx;
    uint8_t state;
    uint8_t *data;
    uint8_t len;
} usb_state_t;

/* Global GPT state */
extern usb_state_t usb;

/* Available Functions */
eZ80portrange_t init_usb(void);
void usb_reset(void);

void usb_host_int(uint8_t);
void usb_otg_int(uint16_t);
void usb_grp0_int(uint8_t);
void usb_grp1_int(uint32_t);
void usb_grp2_int(uint16_t);
uint8_t usb_status(void);

/* Save/Restore */
#include "../emu.h"
bool usb_restore(const emu_image*);
bool usb_save(emu_image*);

#ifdef __cplusplus
}
#endif

#endif
