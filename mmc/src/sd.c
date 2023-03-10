#include "sd.h"

unsigned long sd_scr[2], sd_ocr, sd_rca, sd_err, sd_hv;

bcm_emmc_regs_t *global_regs;
sdcard_t *sdcard;

/**
 * Wait for data or command ready
 */
int sd_status(unsigned int mask) {
    if (mask & SR_CMD_INHIBIT) {
        sdhci_result_t sdhci_result;
        result_t res = sdhci_wait_for_cmd_in_progress(
                global_regs,
                &sdhci_result
        );
        if (result_is_err(res)) {
            return SD_ERROR;
        }
        return SD_OK;
    }
    if (mask & SR_DAT_INHIBIT) {
        sdhci_result_t sdhci_result;
        result_t res = sdhci_wait_for_data_in_progress(
                global_regs,
                &sdhci_result
        );
        if (result_is_err(res)) {
            return SD_ERROR;
        }
        return SD_OK;
    }
    int cnt = 1000000;
    while ((*EMMC_STATUS & mask) && !(*EMMC_INTERRUPT & INT_ERROR_MASK) && cnt--) wait_msec(1);
    return (cnt <= 0 || (*EMMC_INTERRUPT & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

/**
 * Wait for interrupt
 */
int sd_int(unsigned int mask) {
    sdhci_result_t sdhci_result;
    result_t res = sdhci_wait_for_interrupt(
            global_regs,
            mask,
            &sdhci_result
    );
    if (result_is_err(res)) {
        return SD_ERROR;
    }
//    unsigned int r, m = mask | INT_ERROR_MASK;
//    int cnt = 1000000;
//    while (!(*EMMC_INTERRUPT & m) && cnt--) {
//        wait_msec(1);
//    }
//    r = *EMMC_INTERRUPT;
//    if ((r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT)) {
//        *EMMC_INTERRUPT = r;
//        return SD_TIMEOUT;
//    } else if (r & INT_ERROR_MASK) {
//        *EMMC_INTERRUPT = r;
//        return SD_ERROR;
//    }
//    *EMMC_INTERRUPT = mask;
    return 0;
}

/**
 * Send a command
 */
int sd_cmd(unsigned int code, unsigned int arg) {
    int r = 0;
    sd_err = SD_OK;
    if (code & CMD_NEED_APP) {
        r = sd_cmd(CMD_APP_CMD | (sd_rca ? CMD_RSPNS_48 : 0), sd_rca);
        if (sd_rca && !r) {
            uart_puts("ERROR: failed to send SD APP command\n");
            sd_err = SD_ERROR;
            return 0;
        }
        code &= ~CMD_NEED_APP;
    }
    if (sd_status(SR_CMD_INHIBIT)) {
        uart_puts("ERROR: EMMC busy\n");
        sd_err = SD_TIMEOUT;
        return 0;
    }
    uart_puts("EMMC: Sending command ");
    uart_hex(code);
    uart_puts(" arg ");
    uart_hex(arg);
    uart_puts("\n");
    *EMMC_INTERRUPT = *EMMC_INTERRUPT;
    *EMMC_ARG1 = arg;
    *EMMC_CMDTM = code;
    if (code == CMD_SEND_OP_COND) wait_msec(1000);
    else if (code == CMD_SEND_IF_COND || code == CMD_APP_CMD)
        wait_msec(100);
    if ((r = sd_int(INT_CMD_DONE))) {
        uart_puts("ERROR: failed to send EMMC command\n");
        sd_err = r;
        return 0;
    }
    r = *EMMC_RESP0;
    if (code == CMD_GO_IDLE || code == CMD_APP_CMD) return 0;
    else if (code == (CMD_APP_CMD | CMD_RSPNS_48)) return r & SR_APP_CMD;
    else if (code == CMD_SEND_OP_COND) return r;
    else if (code == CMD_SEND_IF_COND) return r == arg ? SD_OK : SD_ERROR;
    else if (code == CMD_ALL_SEND_CID) {
        r |= *EMMC_RESP3;
        r |= *EMMC_RESP2;
        r |= *EMMC_RESP1;
        return r;
    } else if (code == CMD_SEND_REL_ADDR) {
        sd_err = (((r & 0x1fff)) | ((r & 0x2000) << 6) | ((r & 0x4000) << 8) | ((r & 0x8000) << 8)) & CMD_ERRORS_MASK;
        return r & CMD_RCA_MASK;
    }
    return r & CMD_ERRORS_MASK;
    // make gcc happy
    return 0;
}

/**
 * read a block from sd card and return the number of bytes read
 * returns 0 on error.
 */
bool sd_readblock(unsigned int lba, unsigned char *buffer, unsigned int num) {
    result_t res;
    sdhci_result_t sd_res;
    int r, c = 0, d;
    if (num < 1) num = 1;
    uart_puts("sd_readblock lba ");
    uart_hex(lba);
    uart_puts(" num ");
    uart_hex(num);
    uart_puts("\n");
    if (sd_status(SR_DAT_INHIBIT)) {
        sd_err = SD_TIMEOUT;
        return 0;
    }
    unsigned int *buf = (unsigned int *) buffer;
    if (sd_scr[0] & SCR_SUPP_CCS) {
        if (num > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT)) {
//            sd_cmd(CMD_SET_BLOCKCNT, num);
            res = sdhci_send_cmd(
                    global_regs,
                    IX_SET_BLOCKCNT,
                    num,
                    sdcard,
                    &sd_res
            );
            if (result_is_err(res)) {
                result_printf(res);
                return -1;
            }
            if (sd_err) return 0;
        }
        *EMMC_BLKSIZECNT = (num << 16) | 512;
//        sd_cmd(num == 1 ? CMD_READ_SINGLE : CMD_READ_MULTI, lba);
        res = sdhci_send_cmd(
                global_regs,
                num == 1 ? IX_READ_SINGLE : IX_READ_MULTI,
                lba,
                sdcard,
                &sd_res
        );
        if (result_is_err(res)) {
            result_printf(res);
            return -1;
        }
        if (sd_err) return 0;
    } else {
        *EMMC_BLKSIZECNT = (1 << 16) | 512;
    }
    while (c < num) {
        if (!(sd_scr[0] & SCR_SUPP_CCS)) {
//            sd_cmd(CMD_READ_SINGLE, (lba + c) * 512);
            res = sdhci_send_cmd(
                    global_regs,
                    IX_READ_SINGLE,
                    (lba + c) * 512,
                    sdcard,
                    &sd_res
            );
            if (result_is_err(res)) {
                result_printf(res);
                return -1;
            }
            if (sd_err) return 0;
        }
        if ((r = sd_int(INT_READ_RDY))) {
            uart_puts("\rERROR: Timeout waiting for ready to read\n");
            sd_err = r;
            return 0;
        }
        for (d = 0; d < 128; d++) buf[d] = *EMMC_DATA;
        c++;
        buf += 128;
    }
    if (num > 1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (sd_scr[0] & SCR_SUPP_CCS)) {
//        sd_cmd(CMD_STOP_TRANS, 0);
        res = sdhci_send_cmd(
                global_regs,
                IX_STOP_TRANS,
                0,
                sdcard,
                &sd_res
        );
        if (result_is_err(res)) {
            result_printf(res);
            return -1;
        }
    }
    return sd_err != SD_OK || c != num ? 0 : num * 512;
}

/**
 * write a block to the sd card and return the number of bytes written
 * returns 0 on error.
 */
bool sd_writeblock(unsigned char *buffer, unsigned int lba, unsigned int num) {
    result_t res;
    sdhci_result_t sd_res;
    int r, c = 0, d;
    if (num < 1) num = 1;
    uart_puts("sd_writeblock lba ");
    uart_hex(lba);
    uart_puts(" num ");
    uart_hex(num);
    uart_puts("\n");
    if (sd_status(SR_DAT_INHIBIT | SR_WRITE_AVAILABLE)) {
        sd_err = SD_TIMEOUT;
        return 0;
    }
    unsigned int *buf = (unsigned int *) buffer;
    if (sd_scr[0] & SCR_SUPP_CCS) {
        if (num > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT)) {
//            sd_cmd(CMD_SET_BLOCKCNT, num);
            res = sdhci_send_cmd(
                    global_regs,
                    IX_SET_BLOCKCNT,
                    num,
                    sdcard,
                    &sd_res
            );
            if (result_is_err(res)) {
                result_printf(res);
                return -1;
            }
            if (sd_err) return 0;
        }
        *EMMC_BLKSIZECNT = (num << 16) | 512;
//        sd_cmd(num == 1 ? CMD_WRITE_SINGLE : CMD_WRITE_MULTI, lba);
        res = sdhci_send_cmd(
                global_regs,
                num == 1 ? IX_WRITE_SINGLE : IX_WRITE_MULTI,
                lba,
                sdcard,
                &sd_res
        );
        if (result_is_err(res)) {
            result_printf(res);
            return -1;
        }
        if (sd_err) return 0;
    } else {
        *EMMC_BLKSIZECNT = (1 << 16) | 512;
    }
    printf("sd_writeblock: num=%d\n", num);
    while (c < num) {
        if (!(sd_scr[0] & SCR_SUPP_CCS)) {
//            sd_cmd(CMD_WRITE_SINGLE, (lba + c) * 512);
            res = sdhci_send_cmd(
                    global_regs,
                    IX_WRITE_SINGLE,
                    (lba + c) * 512,
                    sdcard,
                    &sd_res
            );
            if (result_is_err(res)) {
                result_printf(res);
                return -1;
            }
            if (sd_err) return 0;
        }
        printf("About to wait for INT_WRITE_RDY\n");
        if ((r = sd_int(INT_WRITE_RDY))) {
            uart_puts("\rERROR: Timeout waiting for ready to write\n");
            sd_err = r;
            return 0;
        }
        printf("Finished waiting for INT_WRITE_RDY\n");
        for (d = 0; d < 128; d++) *EMMC_DATA = buf[d];
        c++;
        buf += 128;
    }
    if ((r = sd_int(INT_DATA_DONE))) {
        uart_puts("\rERROR: Timeout waiting for data done\n");
        sd_err = r;
        return 0;
    }
    if (num > 1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (sd_scr[0] & SCR_SUPP_CCS)) {
//        sd_cmd(CMD_STOP_TRANS, 0);
        res = sdhci_send_cmd(
                global_regs,
                IX_STOP_TRANS,
                0,
                sdcard,
                &sd_res
        );
        if (result_is_err(res)) {
            result_printf(res);
            return -1;
        }
    }
    return sd_err != SD_OK || c != num ? 0 : num * 512;
}

/**
 * set SD clock to frequency in Hz
 */
int sd_clk(unsigned int f) {
    unsigned int d, c = 41666666 / f, x, s = 32, h = 0;
    int cnt = 100000;
    while ((*EMMC_STATUS & (SR_CMD_INHIBIT | SR_DAT_INHIBIT)) && cnt--) wait_msec(1);
    if (cnt <= 0) {
        uart_puts("ERROR: timeout waiting for inhibit flag\n");
        return SD_ERROR;
    }

    *EMMC_CONTROL1 &= ~C1_CLK_EN;
    wait_msec(10);
    x = c - 1;
    if (!x) s = 0;
    else {
        if (!(x & 0xffff0000u)) {
            x <<= 16;
            s -= 16;
        }
        if (!(x & 0xff000000u)) {
            x <<= 8;
            s -= 8;
        }
        if (!(x & 0xf0000000u)) {
            x <<= 4;
            s -= 4;
        }
        if (!(x & 0xc0000000u)) {
            x <<= 2;
            s -= 2;
        }
        if (!(x & 0x80000000u)) {
            x <<= 1;
            s -= 1;
        }
        if (s > 0) s--;
        if (s > 7) s = 7;
    }
    if (sd_hv > HOST_SPEC_V2) d = c; else d = (1 << s);
    if (d <= 2) {
        d = 2;
        s = 0;
    }
    uart_puts("sd_clk divisor ");
    uart_hex(d);
    uart_puts(", shift ");
    uart_hex(s);
    uart_puts("\n");
    if (sd_hv > HOST_SPEC_V2) h = (d & 0x300) >> 2;
    d = (((d & 0x0ff) << 8) | h);
    *EMMC_CONTROL1 = (*EMMC_CONTROL1 & 0xffff003f) | d;
    wait_msec(10);
    *EMMC_CONTROL1 |= C1_CLK_EN;
    wait_msec(10);
    cnt = 10000;
    while (!(*EMMC_CONTROL1 & C1_CLK_STABLE) && cnt--) wait_msec(10);
    if (cnt <= 0) {
        uart_puts("ERROR: failed to get stable clock\n");
        return SD_ERROR;
    }
    return SD_OK;
}

/**
 * initialize EMMC to read SDHC card
 */
int sd_init(bcm_emmc_regs_t *regs, sdcard_t *sd) {
    global_regs = regs;
    sdcard = sd;
    long r, cnt, ccs = 0;
//    // GPIO_CD
//    r = *GPFSEL4;
//    r &= ~(7 << (7 * 3));
//    *GPFSEL4 = r;
//    *GPPUD = 2;
//    wait_cycles(150);
//    *GPPUDCLK1 = (1 << 15);
//    wait_cycles(150);
//    *GPPUD = 0;
//    *GPPUDCLK1 = 0;
//    r = *GPHEN1;
//    r |= 1 << 15;
//    *GPHEN1 = r;
//
//    // GPIO_CLK, GPIO_CMD
//    r = *GPFSEL4;
//    r |= (7 << (8 * 3)) | (7 << (9 * 3));
//    *GPFSEL4 = r;
//    *GPPUD = 2;
//    wait_cycles(150);
//    *GPPUDCLK1 = (1 << 16) | (1 << 17);
//    wait_cycles(150);
//    *GPPUD = 0;
//    *GPPUDCLK1 = 0;
//
//    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
//    r = *GPFSEL5;
//    r |= (7 << (0 * 3)) | (7 << (1 * 3)) | (7 << (2 * 3)) | (7 << (3 * 3));
//    *GPFSEL5 = r;
//    *GPPUD = 2;
//    wait_cycles(150);
//    *GPPUDCLK1 = (1 << 18) | (1 << 19) | (1 << 20) | (1 << 21);
//    wait_cycles(150);
//    *GPPUD = 0;
//    *GPPUDCLK1 = 0;

    sd_hv = (*EMMC_SLOTISR_VER & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;
    uart_puts("EMMC: GPIO set up\n");

    // Reset the card.
//    *EMMC_CONTROL0 = 0;
//    *EMMC_CONTROL1 |= C1_SRST_HC;
//    cnt = 10000;
//    do { wait_msec(10); } while ((*EMMC_CONTROL1 & C1_SRST_HC) && cnt--);
//    if (cnt <= 0) {
//        uart_puts("ERROR: failed to reset EMMC\n");
//        return SD_ERROR;
//    }
//    uart_puts("EMMC: reset OK\n");
//    *EMMC_CONTROL1 |= C1_CLK_INTLEN | C1_TOUNIT_MAX;
//    wait_msec(10);
    // Set clock to setup frequency.
//    if ((r = sd_clk(400000))) return r;
//    *EMMC_INT_EN = 0xffffffff;
//    *EMMC_INT_MASK = 0xffffffff;
    sd_scr[0] = sd_scr[1] = sd_rca = sd_err = 0;
    sdhci_result_t sd_res;
//    sdhci_send_cmd(
//            global_regs,
//            IX_GO_IDLE_STATE,
//            0,
//            sdcard,
//            &sd_res
//    );
//    sd_cmd(CMD_GO_IDLE, 0);
    if (sd_err) return sd_err;

//    sd_cmd(CMD_SEND_IF_COND, 0x000001AA);
//    result_t res_if_cond = sdhci_send_cmd(
//            global_regs,
//            IX_SEND_IF_COND,
//            0x000001AA,
//            sdcard,
//            &sd_res
//    );
//    if (result_is_err(res_if_cond)) {
//        result_printf(res_if_cond);
//        return SD_ERROR;
//    }
    if (sd_err) return sd_err;
    cnt = 6;
    r = 0;
//    while (!(r & ACMD41_CMD_COMPLETE) && cnt--) {
//        wait_cycles(400);
//        r = sd_cmd(CMD_SEND_OP_COND, ACMD41_ARG_HC);
//        uart_puts("EMMC: CMD_SEND_OP_COND returned ");
//        if (r & ACMD41_CMD_COMPLETE)
//            uart_puts("COMPLETE ");
//        if (r & ACMD41_VOLTAGE)
//            uart_puts("VOLTAGE ");
//        if (r & ACMD41_CMD_CCS)
//            uart_puts("CCS ");
//        uart_hex(r >> 32);
//        uart_hex(r);
//        uart_puts("\n");
//        if (sd_err != SD_TIMEOUT && sd_err != SD_OK) {
//            uart_puts("ERROR: EMMC ACMD41 returned error\n");
//            return sd_err;
//        }
//        log_trace("EMMC: ACMD41 returned %x\n", r);
//    }
//    if (!(r & ACMD41_CMD_COMPLETE) || !cnt) return SD_TIMEOUT;
//    if (!(r & ACMD41_VOLTAGE)) return SD_ERROR;
//    if (r & ACMD41_CMD_CCS) ccs = SCR_SUPP_CCS;

    result_t res;
//    size_t retries = 7;
//    bool has_powered_up = false;
//    do {
//        usleep(400000);
//        res = sdhci_send_cmd(
//                global_regs,
//                IX_APP_SEND_OP_COND,
//                ACMD41_ARG_HC,
//                sdcard,
//                &sd_res
//        );
//        if (result_is_err(res)) {
//            result_printf(res);
//            return -1;
//        }
//        res = sdcard_has_powered_up(sdcard, &has_powered_up);
//        if (result_is_err(res)) {
//            result_printf(res);
//            return -1;
//        }
//    } while (!has_powered_up && (retries-- > 0));
//    if (!has_powered_up) {
//        printf("ERROR: EMMC card did not power up\n");
//        return SD_TIMEOUT;
//    }
//    /* Check voltage */
//    bool has_correct_voltage = false;
//    res = sdcard_is_voltage_3v3(sdcard, &has_correct_voltage);
//    if (!has_correct_voltage) {
//        return SD_ERROR_VOLTAGE;
//    }
//    /* Check card capacity. */
//    bool is_high_capacity = false;
//    res = sdcard_is_high_capacity(sdcard, &is_high_capacity);
//    if (result_is_err(res)) {
//        result_printf(res);
//        return -1;
//    }
//    if (is_high_capacity) {
//        log_trace("Card capacity: SDHC\n");
//    }

//    res = sdhci_send_cmd(
//            global_regs,
//            IX_ALL_SEND_CID,
//            0,
//            sdcard,
//            &sd_res
//    );
//    if (result_is_err(res)) {
//        result_printf(res);
//        return -1;
//    }
//    sd_cmd(CMD_ALL_SEND_CID, 0);

//    sd_rca = sd_cmd(CMD_SEND_REL_ADDR, 0);
//    res = sdhci_send_cmd(
//            global_regs,
//            IX_SEND_REL_ADDR,
//            0,
//            sdcard,
//            &sd_res
//    );
//    if (result_is_err(res)) {
//        result_printf(res);
//        return -1;
//    }
    sd_rca = sdcard->rca;

    uart_puts("EMMC: CMD_SEND_REL_ADDR returned ");
    uart_hex(sd_rca >> 32);
    uart_hex(sd_rca);
    uart_puts("\n");
    if (sd_err) return sd_err;

//    if ((r = sd_clk(25000000))) return r;
//    res = sdhci_set_sd_clock(global_regs, 25000000);
//    if (result_is_err(res)) {
//        result_printf(res);
//        return -1;
//    }

//    sd_cmd(CMD_CARD_SELECT, sd_rca);
//    if (sd_err) return sd_err;
//    res = sdhci_send_cmd(
//            global_regs,
//            IX_CARD_SELECT,
//            sd_rca,
//            sdcard,
//            &sd_res
//    );
//    if (result_is_err(res)) {
//        result_printf(res);
//        return -1;
//    }

    if (sd_status(SR_DAT_INHIBIT)) return SD_TIMEOUT;
    *EMMC_BLKSIZECNT = (1 << 16) | 8;

//    sd_cmd(CMD_SEND_SCR, 0);
    res = sdhci_send_cmd(
            global_regs,
            IX_SEND_SCR,
            0,
            sdcard,
            &sd_res
    );
    if (result_is_err(res)) {
        result_printf(res);
        return -1;
    }

    if (sd_err) return sd_err;
    if (sd_int(INT_READ_RDY)) return SD_TIMEOUT;

    r = 0;
    cnt = 100000;
    while (r < 2 && cnt) {
        if (*EMMC_STATUS & SR_READ_AVAILABLE)
            sd_scr[r++] = *EMMC_DATA;
        else
            wait_msec(1);
    }
    if (r != 2) return SD_TIMEOUT;
    if (sd_scr[0] & SCR_SD_BUS_WIDTH_4) {
//        sd_cmd(CMD_SET_BUS_WIDTH, sd_rca | 2);
        res = sdhci_send_cmd(
                global_regs,
                IX_SET_BUS_WIDTH,
                sd_rca | 2,
                sdcard,
                &sd_res
        );
        if (result_is_err(res)) {
            result_printf(res);
            return -1;
        }
        if (sd_err) return sd_err;
        *EMMC_CONTROL0 |= C0_HCTL_DWITDH;
    }
    // add software flag
    uart_puts("EMMC: supports ");
    if (sd_scr[0] & SCR_SUPP_SET_BLKCNT)
        uart_puts("SET_BLKCNT ");
    if (ccs)
        uart_puts("CCS ");
    uart_puts("\n");
    sd_scr[0] &= ~SCR_SUPP_CCS;
    sd_scr[0] |= ccs;
    return SD_OK;
}
