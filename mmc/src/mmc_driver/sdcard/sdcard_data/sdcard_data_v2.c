#include "sdcard_data_v2.h"

result_t sdcard_data_v2_get_c_size(csd_t *csd, uint32_t *ret_val) {
    if (csd == NULL) {
        return result_err("NULL `csd` passed to sdcard_data_v2_get_c_size().");
    }
    if (ret_val == NULL) {
        return result_err("NULL `ret_val` passed to sdcard_data_v2_get_c_size().");
    }
    return csd_get_ver2_c_size(csd, ret_val);
}
