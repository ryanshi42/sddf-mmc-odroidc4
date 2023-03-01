#include "timer_driver.h"

/* Used for transporting chars from `timer_driver` to `serial_client`. */
uintptr_t timer_driver_to_serial_client_putchar_buf;

uintptr_t timer_base_vaddr;

uintptr_t shared_dma;

/* `rx_` stands for "Receive". The following two Receive buffers are to assist
 * with transporting data from `timer_driver` to `mmc_driver`. */

/* In the SDDF's design-documentation, `rx_avail_ring_buf` is referred to as the
 * "Transmit-Free" (TxF) ring buffer. `rx_avail_ring_buf` holds all the buffers
 * in the `shared_dma` data region that are ready to be reused for transporting
 * new data from `timer_driver` to `mmc_driver`. */
uintptr_t rx_avail_ring_buf;

/* In the SDDF's design-documentation, `rx_used_ring_buf` is referred to as the
 * "Transmit-Available" (TxA) ring buffer. `rx_used_ring_buf` holds all the
 * buffers in the `shared_dma` data region that currently hold data sent by
 * `timer_driver` for the `mmc_driver` PD to process.  */
uintptr_t rx_used_ring_buf;

/* Global `timer_driver`. */
timer_driver_t global_timer_driver = {0};

result_t timer_driver_init(timer_driver_t *timer_driver, uintptr_t base_vaddr) {
    /* Initialise the timer. */
    result_t res = bcm_timer_init(
            &timer_driver->bcm_timer,
            base_vaddr
    );
    if (result_is_err(res)) {
        return result_ok_or(res, "Failed to initialise timer in timer_driver_init().");
    }
    /* Initialise our `rx_ring_buf_handle`, which is just a convenience struct
     * where all relevant Receive ring buffers are located. */
    ring_init(
            &timer_driver->rx_ring_buf_handle,
            (ring_buffer_t *) rx_avail_ring_buf,
            (ring_buffer_t *) rx_used_ring_buf,
            NULL,
            0
    );
    return result_ok();
}

void init(void) {
    /* Initialise `printf`. */
    printf_init(
            timer_driver_to_serial_client_putchar_buf,
            TIMER_DRIVER_TO_SERIAL_CLIENT_PUTCHAR_CHANNEL
    );
    /* Initialise the `timer_driver`. */
    result_t result = timer_driver_init(
            &global_timer_driver,
            timer_base_vaddr
    );
    /* Print out any errors. */
    if (result_is_err(result)) {
        result_printf(result);
        return;
    }
}

seL4_MessageInfo_t protected(sel4cp_channel ch, sel4cp_msginfo msginfo) {
    return sel4cp_msginfo_new(0, 0);
}

void notified(sel4cp_channel channel) {
    timer_driver_t *timer_driver = &global_timer_driver; /* Local reference to global serial driver for our convenience. */
    (void) timer_driver; /* Suppress unused variable warning. */
    switch(channel) {
        /* If MMC has asked the serial client to `putchar`, then print out a character. */
        case TIMER_DRIVER_TO_MMC_DRIVER_GET_NUM_TICKS_CHANNEL: {
            uint64_t num_ticks;
            result_t res = bcm_timer_get_num_ticks(&timer_driver->bcm_timer, &num_ticks);
            if (result_is_err(res)) {
                result_printf(res);
                return;
            }
            uintptr_t buf_addr; /* The dequeued buffer's address will be stored in `buf_addr`. */
            unsigned int buf_len; /* The dequeued buffer's length will be stored in `buf_len`. */
            /* We don't use the `cookie` but the `dequeue_avail` function call requires
             * a valid pointer for the `cookie` param, so we provide one to it anyway. */
            void *unused_cookie;
            /* Dequeue an available buffer from the Receive-available ring. */
            int ret_dequeue_avail = dequeue_avail(
                    &timer_driver->rx_ring_buf_handle,
                    &buf_addr,
                    &buf_len,
                    &unused_cookie
            );
            if (ret_dequeue_avail < 0) {
                printf("Failed to dequeue buffer from Receive available queue in notified().\n");
                return;
            }
            if (buf_len < sizeof(num_ticks)) {
                printf("Buffer too small to hold a uint64_t in notified().\n");
                return;
            }
            /* Save `num_ticks` into the buffer. */
            *((uint64_t *) buf_addr) = num_ticks;
            /* Enqueue our now-modified buffer onto the Receive-used ring. */
            int ret_enqueue_used = enqueue_used(
                    &timer_driver->rx_ring_buf_handle,
                    buf_addr, /* This is the buffer that contains our num_ticks value. */
                    sizeof(num_ticks),
                    unused_cookie
            );
            if (ret_enqueue_used < 0) {
                printf("Received-used ring is full in notified().\n");
                return;
            }
            break;
        }
        default:
            printf("Serial client: received notification on unexpected channel\n");
            break;
    }
}
