#include <common.h>
#include <asm/io.h>
#include <asm-generic/errno.h>
#include <intel_scu_ipc.h>
#include <intel_scu_ipcutil.h>

#define WATCHDOG_IPC_CMD 0xF8

#define IPC_STATUS_ADDR         0X04
#define IPC_SPTR_ADDR           0x08
#define IPC_DPTR_ADDR           0x0C
#define IPC_READ_BUFFER         0x90
#define IPC_WRITE_BUFFER        0x80
#define IPC_IOC			0x100

struct intel_ipc_controller {
	u32 ipc_base;
	u32 i2c_base;
};

static struct intel_ipc_controller  ipcdev; /* Only one for now */

static int platform; /* Platform type */
static u8  system_state; /* SYSTEM status */

u8 intel_readb_scu_register(u32 module_addr, u32 offset)
{
	return readb(CONFIG_SCU_BASE_ADDR + module_addr + offset);
}

struct vrtc_info read_vrtc_info(void)
{
	struct vrtc_info vinfo;

	vinfo.vrtc1_sec  = intel_readb_scu_register(VRTC_REG, VRTC1_SEC);
	vinfo.vrtc1_sec_alarm = intel_readb_scu_register(VRTC_REG, VRTC1_SEC_A);
	vinfo.vrtc1_min = intel_readb_scu_register(VRTC_REG, VRTC1_MIN);
	vinfo.vrtc1_min_alarm = intel_readb_scu_register(VRTC_REG, VRTC1_MIN_A);
	vinfo.vrtc1_hour = intel_readb_scu_register(VRTC_REG, VRTC1_HOU);
	vinfo.vrtc1_hour_alarm = intel_readb_scu_register(VRTC_REG, VRTC1_HOU_A);
	vinfo.vrtc1_day_week = intel_readb_scu_register(VRTC_REG, VRTC1_DAY_W);
	vinfo.vrtc1_day_month = intel_readb_scu_register(VRTC_REG, VRTC1_DAY_M);
	vinfo.vrtc1_month = intel_readb_scu_register(VRTC_REG, VRTC1_MON);
	vinfo.vrtc1_year = intel_readb_scu_register(VRTC_REG, VRTC1_YEA);

	return vinfo;
}

/*
 * Command Register (Write Only):
 * A write to this register results in an interrupt to the SCU core processor
 * Format:
 * |rfu2(8) | size(8) | command id(4) | rfu1(3) | ioc(1) | command(8)|
 */
void intel_scu_ipc_send_command(u32 cmd) /* Send ipc command */
{
#if 0
	ipcdev.cmd = cmd;
	INIT_COMPLETION(ipcdev.cmd_complete);

	/* Revert me:
	 * This is a workaround here for MRFLD, because IPC interrupt for MRFLD
	 * is still not supported on HVP. So we use polling mode to access PMIC
	 * registers. Will remove when MRFLD IPC interrupt is functional.
	 */
	if (platform == INTEL_MID_CPU_CHIP_TANGIER) {
		ipcdev.ioc = 0;
		writel(cmd, ipcdev.ipc_base);
		goto end;
	}
#endif

	if (system_state == 1) { // SYSTEM_RUNNING
		//ipcdev.ioc = 1;
		writel(cmd | IPC_IOC, ipcdev.ipc_base);
	} else {
		//ipcdev.ioc = 0;
		writel(cmd, ipcdev.ipc_base);
	}

//end:
	//pmu_log_ipc(cmd);
}

/*
 * IPC Write Buffer (Write Only):
 * 16-byte buffer for sending data associated with IPC command to
 * SCU. Size of the data is specified in the IPC_COMMAND_REG register
 */
void ipc_data_writel(u32 data, u32 offset) /* Write ipc data */
{
	writel(data, ipcdev.ipc_base + IPC_WRITE_BUFFER + offset);
}

/*
 * Status Register (Read Only):
 * Driver will read this register to get the ready/busy status of the IPC
 * block and error status of the IPC command that was just processed by SCU
 * Format:
 * |rfu3(8)|error code(8)|initiator id(8)|cmd id(4)|rfu1(2)|error(1)|busy(1)|
 */
int test;

u32 ipc_read_status(void)
{
	//return __raw_readl(ipcdev.ipc_base + IPC_STATUS_ADDR);
	test = __raw_readl(ipcdev.ipc_base + IPC_STATUS_ADDR);
	return test;
}

u8 ipc_data_readb(u32 offset) /* Read ipc byte data */
{
	return readb(ipcdev.ipc_base + IPC_READ_BUFFER + offset);
}

u32 ipc_data_readl(u32 offset) /* Read ipc u32 data */
{
	return readl(ipcdev.ipc_base + IPC_READ_BUFFER + offset);
}

int intel_scu_ipc_check_status(void)
{
	int ret = 0;
#if 1
	int status;
	int loop_count = 3000000;
	//int i;
#if 0
	if (ipcdev.ioc && (system_state == SYSTEM_RUNNING)) {
		if (0 == wait_for_completion_timeout(
				&ipcdev.cmd_complete, 3 * HZ))
			ret = -ETIMEDOUT;
	} else {
		while ((ipc_read_status() & 1) && --loop_count)
			udelay(1);
		if (loop_count == 0)
			ret = -ETIMEDOUT;
	}
#endif
	while ((ipc_read_status() & 1) && --loop_count)
		udelay(1);
	if (loop_count == 0)
		ret = -ETIMEDOUT;

	status = ipc_read_status();
	if (status & 0x2) {
		printf("%s() status=0x%08x\n", __func__, status);
		return -EIO;
	}
#if 0
	if (ret == -ETIMEDOUT)
		dev_err(&ipcdev.pdev->dev, "IPC timed out, IPC_STS=0x%x, IPC_CMD=0x%x",
			status, ipcdev.cmd);
	if (status & 0x2) {
		ret = -EIO;
		i = (status >> 16) & 0xFF;
		if (i < ARRAY_SIZE(ipc_err_sources))
			dev_err(&ipcdev.pdev->dev,
				"IPC failed: %s, IPC_STS=0x%x, IPC_CMD=0x%x",
				ipc_err_sources[i], status, ipcdev.cmd);
		else
			dev_err(&ipcdev.pdev->dev,
				"IPC failed: unknown error, IPC_STS=0x%x, IPC_CMD=0x%x",
				status, ipcdev.cmd);
	}
#endif
	status++;
#endif
	return ret;
}

/**
 *	intel_scu_ipc_simple_command - send a simple command
 *	@cmd: command
 *	@sub: sub type
 *
 *	Issue a simple command to the SCU. Do not use this interface if
 *	you must then access data as any data values may be overwritten
 *	by another SCU access by the time this function returns.
 *
 *	This function may sleep. Locking for SCU accesses is handled for
 *	the caller.
 */
int intel_scu_ipc_simple_command(int cmd, int sub)
{
	int err;

#if 0
	if (ipcdev.pdev == NULL)
		return -ENODEV;
#endif
	//intel_scu_ipc_lock();
	intel_scu_ipc_send_command(sub << 12 | cmd);
	err = intel_scu_ipc_check_status();
	//intel_scu_ipc_unlock();
	return err;
}
//EXPORT_SYMBOL(intel_scu_ipc_simple_command);

void intel_scu_ipc_lock(void)
{
#if 0
	atomic_inc(&ipcdev.pending);

	mutex_lock(&ipclock);

	/* Prevent C-states beyond C6 */
	pm_qos_update_request(qos, CSTATE_EXIT_LATENCY_S0i1 - 1);

	/* Prevent S3 */
	wake_lock(&ipc_wake_lock);
#endif 
}
//EXPORT_SYMBOL_GPL(intel_scu_ipc_lock);

void intel_scu_ipc_unlock(void)
{
#if 0
	/* Re-enable S3 */
	wake_unlock(&ipc_wake_lock);

	/* Re-enable Deeper C-states beyond C6 */
	pm_qos_update_request(qos, PM_QOS_DEFAULT_VALUE);

	mutex_unlock(&ipclock);

	if (!atomic_dec_and_test(&ipcdev.pending))
		schedule();
#endif
}
//EXPORT_SYMBOL_GPL(intel_scu_ipc_unlock);


int intel_scu_ipc_raw_cmd(u32 cmd, u32 sub, u8 *in, u8 inlen, u32 *out,
        u32 outlen, u32 dptr, u32 sptr)
{
    int i, err;
    u32 wbuf[4];

#if 0
	if (ipcdev.pdev == NULL)
		return -ENODEV;
#endif

    if (inlen > 16)
		return -EINVAL;

	memcpy(wbuf, in, inlen);

	writel(dptr, ipcdev.ipc_base + IPC_DPTR_ADDR);
	writel(sptr, ipcdev.ipc_base + IPC_SPTR_ADDR);

	/**
     * SRAM controller don't support 8bit write, it only supports
	 * 32bit write, so we have to write into the WBUF in 32bit,
	 * and SCU FW will use the inlen to determine the actual input
	 * data length in the WBUF.
	 */
	for (i = 0; i < ((inlen + 3) / 4); i++)
		ipc_data_writel(wbuf[i], 4 * i);

																		     /**
	* Watchdog IPC command is an exception here using double word
	* as the unit of input data size because of some historical
	* reasons and SCU FW is doing so.
	*/
	if ((cmd & 0xFF) == WATCHDOG_IPC_CMD)
	   inlen = (inlen + 3) / 4;

		intel_scu_ipc_send_command((inlen << 16) | (sub << 12) | cmd);
	    err = intel_scu_ipc_check_status();

    for (i = 0; i < outlen; i++)
		*out++ = ipc_data_readl(4 * i);

	return err;
}
//EXPORT_SYMBOL_GPL(intel_scu_ipc_raw_cmd);

int intel_scu_ipc_command(u32 cmd, u32 sub, u8 *in, u8 inlen,
        u32 *out, u32 outlen)
{
    int ret;
    intel_scu_ipc_lock();
    ret = intel_scu_ipc_raw_cmd(cmd, sub, in, inlen, out, outlen, 0, 0);
    intel_scu_ipc_unlock();
    return ret;
}
//EXPORT_SYMBOL_GPL(intel_scu_ipc_command);

int init_scu_ipc(void)
{
    if (system_state)
        return 0;

    ipcdev.ipc_base = CONFIG_SCU_IPC_BASE;
    ipcdev.i2c_base = CONFIG_SCU_I2C_BASE;

    platform = CONFIG_CPU_CHIP;
    system_state = 1; // SYSTEM_RUNNING 

    intel_scu_ipc_oshob_init();

    return 0;
}
