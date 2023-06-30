#pragma once

#define CONFIG_ARM_HIKEY_OUTSTANDING_PREFETCHERS  0
#define CONFIG_ARM_HIKEY_PREFETCHER_STRIDE  0
#define CONFIG_ARM_HIKEY_PREFETCHER_NPFSTRM  0
/* disabled: CONFIG_ARM_HIKEY_PREFETCHER_STBPFDIS */
/* disabled: CONFIG_ARM_HIKEY_PREFETCHER_STBPFRS */
/* disabled: CONFIG_PLAT_IMX7 */
#define CONFIG_USER_TOP  0xa0000000
/* disabled: CONFIG_ARCH_AARCH32 */
#define CONFIG_ARCH_AARCH64  1
/* disabled: CONFIG_ARCH_ARM_HYP */
/* disabled: CONFIG_ARCH_RISCV32 */
/* disabled: CONFIG_ARCH_RISCV64 */
/* disabled: CONFIG_ARCH_X86_64 */
/* disabled: CONFIG_ARCH_IA32 */
#define CONFIG_SEL4_ARCH  aarch64
#define CONFIG_ARCH_ARM  1
#define CONFIG_ARCH  arm
#define CONFIG_WORD_SIZE  64
#define CONFIG_ARM_PLAT  zcu102
#define CONFIG_PLAT_ZYNQMP_ZCU102  1
#define CONFIG_MACH  zynq
/* disabled: CONFIG_PLAT_BCM2711 */
/* disabled: CONFIG_PLAT_BCM2837 */
/* disabled: CONFIG_PLAT_FVP */
/* disabled: CONFIG_PLAT_HIKEY */
/* disabled: CONFIG_PLAT_IMX8MQ_EVK */
/* disabled: CONFIG_PLAT_IMX8MM_EVK */
/* disabled: CONFIG_PLAT_MAAXBOARD */
/* disabled: CONFIG_PLAT_ODROIDC2 */
/* disabled: CONFIG_PLAT_ODROIDC4 */
/* disabled: CONFIG_PLAT_QEMU_ARM_VIRT */
/* disabled: CONFIG_PLAT_QUARTZ64 */
/* disabled: CONFIG_PLAT_ROCKPRO64 */
/* disabled: CONFIG_PLAT_TQMA8XQP1GB */
/* disabled: CONFIG_PLAT_TX1 */
/* disabled: CONFIG_PLAT_TX2 */
#define CONFIG_PLAT_ZYNQMP  1
#define CONFIG_PLAT  zynqmp
/* disabled: CONFIG_ARM_CORTEX_A7 */
/* disabled: CONFIG_ARM_CORTEX_A8 */
/* disabled: CONFIG_ARM_CORTEX_A9 */
/* disabled: CONFIG_ARM_CORTEX_A15 */
/* disabled: CONFIG_ARM_CORTEX_A35 */
#define CONFIG_ARM_CORTEX_A53  1
/* disabled: CONFIG_ARM_CORTEX_A55 */
/* disabled: CONFIG_ARM_CORTEX_A57 */
/* disabled: CONFIG_ARM_CORTEX_A72 */
/* disabled: CONFIG_ARCH_ARM_V7A */
/* disabled: CONFIG_ARCH_ARM_V7VE */
#define CONFIG_ARCH_ARM_V8A  1
/* disabled: CONFIG_AARCH64_SERROR_IGNORE */
#define CONFIG_ARM_MACH  zynq
#define CONFIG_KERNEL_MCS  1
#define CONFIG_ARM_PA_SIZE_BITS_40  1
/* disabled: CONFIG_ARM_PA_SIZE_BITS_44 */
#define CONFIG_ARM_ICACHE_VIPT  1
/* disabled: CONFIG_DEBUG_DISABLE_L2_CACHE */
/* disabled: CONFIG_DEBUG_DISABLE_L1_ICACHE */
/* disabled: CONFIG_DEBUG_DISABLE_L1_DCACHE */
/* disabled: CONFIG_DEBUG_DISABLE_BRANCH_PREDICTION */
/* disabled: CONFIG_ARM_HYPERVISOR_SUPPORT */
/* disabled: CONFIG_ARM_GIC_V3_SUPPORT */
/* disabled: CONFIG_AARCH64_VSPACE_S2_START_L1 */
/* disabled: CONFIG_ARM_HYP_ENABLE_VCPU_CP14_SAVE_AND_RESTORE */
/* disabled: CONFIG_ARM_ERRATA_430973 */
/* disabled: CONFIG_ARM_ERRATA_773022 */
/* disabled: CONFIG_ARM_SMMU */
/* disabled: CONFIG_TK1_SMMU */
/* disabled: CONFIG_ENABLE_A9_PREFETCHER */
/* disabled: CONFIG_EXPORT_PMU_USER */
/* disabled: CONFIG_DISABLE_WFI_WFE_TRAPS */
/* disabled: CONFIG_SMMU_INTERRUPT_ENABLE */
/* disabled: CONFIG_AARCH32_FPU_ENABLE_CONTEXT_SWITCH */
#define CONFIG_AARCH64_USER_CACHE_ENABLE  1
#define CONFIG_L1_CACHE_LINE_SIZE_BITS  6
#define CONFIG_EXPORT_PCNT_USER  1
/* disabled: CONFIG_EXPORT_VCNT_USER */
/* disabled: CONFIG_EXPORT_PTMR_USER */
/* disabled: CONFIG_EXPORT_VTMR_USER */
#define CONFIG_VTIMER_UPDATE_VOFFSET  1
#define CONFIG_HAVE_FPU  1
#define CONFIG_PADDR_USER_DEVICE_TOP  1099511627776
#define CONFIG_ROOT_CNODE_SIZE_BITS  12
#define CONFIG_BOOT_THREAD_TIME_SLICE  5
#define CONFIG_RETYPE_FAN_OUT_LIMIT  256
#define CONFIG_MAX_NUM_WORK_UNITS_PER_PREEMPTION  100
#define CONFIG_RESET_CHUNK_BITS  8
#define CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS  230
#define CONFIG_FASTPATH  1
/* disabled: CONFIG_EXCEPTION_FASTPATH */
#define CONFIG_NUM_DOMAINS  1
/* disabled: CONFIG_SIGNAL_FASTPATH */
#define CONFIG_NUM_PRIORITIES  256
#define CONFIG_MAX_NUM_NODES  1
/* disabled: CONFIG_ENABLE_SMP_SUPPORT */
#define CONFIG_KERNEL_STACK_BITS  12
#define CONFIG_FPU_MAX_RESTORES_SINCE_SWITCH  64
/* disabled: CONFIG_VERIFICATION_BUILD */
/* disabled: CONFIG_BINARY_VERIFICATION_BUILD */
#define CONFIG_DEBUG_BUILD  1
/* disabled: CONFIG_HARDWARE_DEBUG_API */
#define CONFIG_PRINTING  1
/* disabled: CONFIG_KERNEL_INVOCATION_REPORT_ERROR_IPC */
#define CONFIG_NO_BENCHMARKS  1
/* disabled: CONFIG_BENCHMARK_GENERIC */
/* disabled: CONFIG_BENCHMARK_TRACK_KERNEL_ENTRIES */
/* disabled: CONFIG_BENCHMARK_TRACEPOINTS */
/* disabled: CONFIG_BENCHMARK_TRACK_UTILISATION */
#define CONFIG_KERNEL_BENCHMARK  none
/* disabled: CONFIG_ENABLE_BENCHMARKS */
/* disabled: CONFIG_KERNEL_LOG_BUFFER */
#define CONFIG_MAX_NUM_TRACE_POINTS  0
#define CONFIG_IRQ_REPORTING  1
#define CONFIG_COLOUR_PRINTING  1
#define CONFIG_USER_STACK_TRACE_LENGTH  16
#define CONFIG_KERNEL_OPT_LEVEL_O2  1
/* disabled: CONFIG_KERNEL_OPT_LEVEL_OS */
/* disabled: CONFIG_KERNEL_OPT_LEVEL_O0 */
/* disabled: CONFIG_KERNEL_OPT_LEVEL_O1 */
/* disabled: CONFIG_KERNEL_OPT_LEVEL_O3 */
#define CONFIG_KERNEL_OPT_LEVEL  -O2
#define CONFIG_KERNEL_OPTIMISATION_CLONE_FUNCTIONS  1
/* disabled: CONFIG_KERNEL_FWHOLE_PROGRAM */
/* disabled: CONFIG_DANGEROUS_CODE_INJECTION */
/* disabled: CONFIG_DEBUG_DISABLE_PREFETCHERS */
/* disabled: CONFIG_SET_TLS_BASE_SELF */
#define CONFIG_KERNEL_WCET_SCALE  1
#define CONFIG_KERNEL_STATIC_MAX_PERIOD_US  0
/* disabled: CONFIG_CLZ_32 */
/* disabled: CONFIG_CLZ_64 */
/* disabled: CONFIG_CTZ_32 */
/* disabled: CONFIG_CTZ_64 */
/* disabled: CONFIG_CLZ_NO_BUILTIN */
/* disabled: CONFIG_CTZ_NO_BUILTIN */
