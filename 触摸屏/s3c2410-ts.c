
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <linux/delay.h>

#include <asm/hardware.h>


/* debug macros */
#undef DEBUG
#ifdef DEBUG
#define DPRINTK( x... )	printk("s3c2410-ts: " ##x)
#else
#define DPRINTK( x... )
#endif

#define PEN_UP	        0		
#define PEN_DOWN	1
#define PEN_FLEETING	2
#define MAX_TS_BUF	8	/* how many do we want to buffer */

#define DEVICE_NAME	"s3c2410-ts"
#define TSRAW_MINOR	1

typedef struct {
	unsigned int penStatus;		/* PEN_UP, PEN_DOWN, PEN_SAMPLE */
	TS_RET buf[MAX_TS_BUF];		/* protect against overrun */
	unsigned int head, tail;	/* head and tail for queued events */
	wait_queue_head_t wq;
	spinlock_t lock;
} TS_DEV;

static TS_DEV tsdev;

#define BUF_HEAD	(tsdev.buf[tsdev.head])
#define BUF_TAIL	(tsdev.buf[tsdev.tail])
#define INCBUF(x,mod) 	((++(x)) & ((mod) - 1))

static int tsMajor = 0;

static void (*tsEvent)(void);

#define HOOK_FOR_DRAG
#ifdef HOOK_FOR_DRAG
#define TS_TIMER_DELAY  (HZ/100) /* 10 ms */
static struct timer_list ts_timer;
#endif

#define wait_down_int()	{ ADCTSC = DOWN_INT | XP_PULL_UP_EN | \
				XP_AIN | XM_HIZ | YP_AIN | YM_GND | \
				XP_PST(WAIT_INT_MODE); }
#define wait_up_int()	{ ADCTSC = UP_INT | XP_PULL_UP_EN | XP_AIN | XM_HIZ | \
				YP_AIN | YM_GND | XP_PST(WAIT_INT_MODE); }
#define mode_x_axis()	{ ADCTSC = XP_EXTVLT | XM_GND | YP_AIN | YM_HIZ | \
				XP_PULL_UP_DIS | XP_PST(X_AXIS_MODE); }
#define mode_x_axis_n()	{ ADCTSC = XP_EXTVLT | XM_GND | YP_AIN | YM_HIZ | \
				XP_PULL_UP_DIS | XP_PST(NOP_MODE); }
#define mode_y_axis()	{ ADCTSC = XP_AIN | XM_HIZ | YP_EXTVLT | YM_GND | \
				XP_PULL_UP_DIS | XP_PST(Y_AXIS_MODE); }
#define start_adc_x()	{ ADCCON = PRESCALE_EN | PRSCVL(49) | \
				ADC_INPUT(ADC_IN5) | ADC_START_BY_RD_EN | \
				ADC_NORMAL_MODE; \
			  ADCDAT0; }
#define start_adc_y()	{ ADCCON = PRESCALE_EN | PRSCVL(49) | \
				ADC_INPUT(ADC_IN7) | ADC_START_BY_RD_EN | \
				ADC_NORMAL_MODE; \
			  ADCDAT1; }
#define disable_ts_adc()	{ ADCCON &= ~(ADCCON_READ_START); }

static int adc_state = 0;
static int x, y;	/* touch screen coorinates */

static void tsEvent_raw(void)
{
	if (tsdev.penStatus == PEN_DOWN) { /* 保存按下时的坐标 */
		BUF_HEAD.x = x;
		BUF_HEAD.y = y;
		BUF_HEAD.pressure = PEN_DOWN;

#ifdef HOOK_FOR_DRAG 
		ts_timer.expires = jiffies + TS_TIMER_DELAY;
		add_timer(&ts_timer); /* 对长时间按下键的处理 */
#endif
	} else {
#ifdef HOOK_FOR_DRAG 
		del_timer(&ts_timer);
#endif
		
		BUF_HEAD.x = 0;
		BUF_HEAD.y = 0;
		BUF_HEAD.pressure = PEN_UP; /* 保存弹起时的坐标 */
	}

	tsdev.head = INCBUF(tsdev.head, MAX_TS_BUF);
	wake_up_interruptible(&(tsdev.wq)); /* 唤醒进程 */

}

static int tsRead(TS_RET * ts_ret)
{
        spin_lock_irq(&(tsdev.lock));
	ts_ret->x = BUF_TAIL.x;
	ts_ret->y = BUF_TAIL.y;
	ts_ret->pressure = BUF_TAIL.pressure;
	tsdev.tail = INCBUF(tsdev.tail, MAX_TS_BUF);
	spin_unlock_irq(&(tsdev.lock));
        
	return sizeof(TS_RET);
}


static ssize_t s3c2410_ts_read(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
	TS_RET ts_ret;

retry: 
	if (tsdev.head != tsdev.tail) {
		int count;
		count = tsRead(&ts_ret);
		if (count) copy_to_user(buffer, (char *)&ts_ret, count);
		return count;
	} else {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		interruptible_sleep_on(&(tsdev.wq));
		if (signal_pending(current))
			return -ERESTARTSYS;
		goto retry;
	}

	return sizeof(TS_RET);
}

static unsigned int s3c2410_ts_poll(struct file *filp, struct poll_table_struct *wait)
{
	poll_wait(filp, &(tsdev.wq), wait);
	return (tsdev.head == tsdev.tail) ? 0 : (POLLIN | POLLRDNORM); 
}

static inline void start_ts_adc(void)
{
	adc_state = 0;
	mode_x_axis();
	start_adc_x();
}

static inline void s3c2410_get_XY(void)
{
	if (adc_state == 0) { /* 转换x */
		adc_state = 1;
		disable_ts_adc();
		y = (ADCDAT0 & 0x3ff); /* 获取x坐标 */
		mode_y_axis(); 
		start_adc_y(); /*启动y坐标转化*/
	} else if (adc_state == 1) {  /*转换y*/
		adc_state = 0;
		disable_ts_adc();
		x = (ADCDAT1 & 0x3ff); /* 获取y坐标 */
		tsdev.penStatus = PEN_DOWN; /* 改变屏状态 */
		DPRINTK("PEN DOWN: x: %08d, y: %08d\n", x, y);
		wait_up_int();   /* 等待弹起中断 */
		tsEvent();
	}
}

static void s3c2410_isr_adc(int irq, void *dev_id, struct pt_regs *reg)
{

	spin_lock_irq(&(tsdev.lock));
	if (tsdev.penStatus == PEN_UP)
	  s3c2410_get_XY();
#ifdef HOOK_FOR_DRAG
	else
	  s3c2410_get_XY();
#endif
	spin_unlock_irq(&(tsdev.lock));
}

/* 当按键按下时首先产生的中断 */
static void s3c2410_isr_tc(int irq, void *dev_id, struct pt_regs *reg)
{
	spin_lock_irq(&(tsdev.lock));
	if (tsdev.penStatus == PEN_UP) /*如果是按下中断*/
	{
	  start_ts_adc(); /* 开始AD转化 */
	} else 
	{
	  tsdev.penStatus = PEN_UP; /* 如果是弹起中断*/
	  DPRINTK("PEN UP: x: %08d, y: %08d\n", x, y);
	  wait_down_int(); /*等待按下中断*/
	  tsEvent(); /* 调用后续处理函数 */
	}
	spin_unlock_irq(&(tsdev.lock));
}

#ifdef HOOK_FOR_DRAG
static void ts_timer_handler(unsigned long data)
{
	spin_lock_irq(&(tsdev.lock));
	if (tsdev.penStatus == PEN_DOWN) {
		start_ts_adc();
	}
	spin_unlock_irq(&(tsdev.lock));
}
#endif

static int s3c2410_ts_open(struct inode *inode, struct file *filp)
{
	tsdev.head = tsdev.tail = 0;
	tsdev.penStatus = PEN_UP;
#ifdef HOOK_FOR_DRAG 
	init_timer(&ts_timer);
	ts_timer.function = ts_timer_handler;
#endif
	tsEvent = tsEvent_raw;
	init_waitqueue_head(&(tsdev.wq));

	MOD_INC_USE_COUNT;
	return 0;
}

static int s3c2410_ts_release(struct inode *inode, struct file *filp)
{
#ifdef HOOK_FOR_DRAG
	del_timer(&ts_timer);
#endif
	MOD_DEC_USE_COUNT;
	return 0;
}

static struct file_operations s3c2410_fops = {
	owner:	THIS_MODULE,
	open:	s3c2410_ts_open,
	read:	s3c2410_ts_read,	
	release:	s3c2410_ts_release,
	poll:	s3c2410_ts_poll,
};

void tsEvent_dummy(void) {}


#ifdef CONFIG_DEVFS_FS
static devfs_handle_t devfs_ts_dir, devfs_tsraw;
#endif
static int __init s3c2410_ts_init(void)
{
	int ret;

	tsEvent = tsEvent_dummy;

	/* 注册字符设备 */
	ret = register_chrdev(0, DEVICE_NAME, &s3c2410_fops);
	if (ret < 0) {
	  printk(DEVICE_NAME " can't get major number\n");
	  return ret;
	}
	tsMajor = ret;

	/* set gpio to XP, YM, YP and  YM */
	set_gpio_ctrl(GPIO_YPON); 
	set_gpio_ctrl(GPIO_YMON);
	set_gpio_ctrl(GPIO_XPON);
	set_gpio_ctrl(GPIO_XMON);

	
	/* 注册中断 */
	ret = request_irq(IRQ_ADC_DONE, s3c2410_isr_adc, SA_INTERRUPT, 
			  DEVICE_NAME, s3c2410_isr_adc);
	if (ret) goto adc_failed;
	ret = request_irq(IRQ_TC, s3c2410_isr_tc, SA_INTERRUPT, 
			  DEVICE_NAME, s3c2410_isr_tc);
	if (ret) goto tc_failed;

	/* 等待触摸屏被按下的中断 */
	wait_down_int();

#ifdef CONFIG_DEVFS_FS
	devfs_ts_dir = devfs_mk_dir(NULL, "touchscreen", NULL);
	devfs_tsraw = devfs_register(devfs_ts_dir, "0raw", DEVFS_FL_DEFAULT,
			tsMajor, TSRAW_MINOR, S_IFCHR | S_IRUSR | S_IWUSR,
			&s3c2410_fops, NULL);
#endif

	ADCDLY = 0xFFFF;
	printk(DEVICE_NAME " initialized\n");

	return 0;
 tc_failed:
	free_irq(IRQ_ADC_DONE, s3c2410_isr_adc);
 adc_failed:
	return ret;
}

static void __exit s3c2410_ts_exit(void)
{
#ifdef CONFIG_DEVFS_FS	
	devfs_unregister(devfs_tsraw);
	devfs_unregister(devfs_ts_dir);
#endif	
	unregister_chrdev(tsMajor, DEVICE_NAME);

	free_irq(IRQ_ADC_DONE, s3c2410_isr_adc);
	free_irq(IRQ_TC, s3c2410_isr_tc);
}

module_init(s3c2410_ts_init);
module_exit(s3c2410_ts_exit);
