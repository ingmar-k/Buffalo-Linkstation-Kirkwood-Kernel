#include "boardEnv/mvBoardEnvLib.h"
#include "BuffaloGpio.h"
#include "BuffaloUart.h"
#include "uart/mvUart.h"

#if defined(CONFIG_ARCH_FEROCEON_KW)

//#define MICONMSG

static volatile MV_UART_PORT* uartBase[MV_UART_MAX_CHAN]={mvUartBase(CONSOLEPORT),mvUartBase(MICONPORT)}; 
#elif defined(CONFIG_ARCH_FEROCEON_MV78XX0)
#include <linux/serial_core.h>
#include <linux/serial_reg.h>

struct uart_port *uart_ports[CONFIG_SERIAL_8250_NR_UARTS];

#if defined(CONFIG_BUFFALO_USE_UPS)
static unsigned int BuffaloUps_get_mctrl(struct uart_port *);
static void BuffaloUps_set_mctrl(struct uart_port *, unsigned int);
#endif
unsigned int BuffaloMctrlMode = BUFFALO_MODEM_CTRL_MODE_MPP;

static volatile MV_UART_PORT* uartBase[MV_UART_MAX_CHAN]={mvUartBase(CONSOLEPORT
),mvUartBase(MICONPORT),mvUartBase(UPSPORT)};
#elif defined(CONFIG_ARCH_ARMADA370)
static volatile MV_UART_PORT* uartBase[MV_UART_MAX_CHAN]={mvUartBase(CONSOLEPORT),mvUartBase(MICONPORT)}; 
#endif /* CONFIG_ARCH_FEROCEON_MV78XX0 */


static MV_VOID mvUartInit2(MV_U32 port, MV_U32 baudDivisor)
{
	volatile MV_UART_PORT *pUartPort=uartBase[port];
	unsigned char ier;
	
	//uartBase[port] = pUartPort = (volatile MV_UART_PORT *)base;
	
	ier = pUartPort->ier;
	
	pUartPort->ier = 0x00;
	pUartPort->lcr = LCR_DIVL_EN;           /* Access baud rate */
	pUartPort->dll = baudDivisor & 0xff;
	pUartPort->dlm = (baudDivisor >> 8) & 0xff;
	if (port==0){
		pUartPort->lcr = LCR_8N1;
	}else{
		pUartPort->lcr = LCR_WLS_8 | LCR_2_STB | LCR_PEN | LCR_EPS;
	}
	/* Clear & enable FIFOs */
	pUartPort->fcr = FCR_FIFO_EN | FCR_RXSR | FCR_TXSR;
	
	pUartPort->ier = ier;
		
	return;
}

static MV_VOID	mvUartPutc2(MV_U32 port, MV_U8 c)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	while ((pUartPort->lsr & LSR_THRE) == 0) ;
	pUartPort->thr = c;
	return;
}

static MV_U8	mvUartGetc2(MV_U32 port)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	while ((pUartPort->lsr & LSR_DR) == 0) ;
	return (pUartPort->rbr);
}

MV_BOOL mvUartTstc2(MV_U32 port)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	return ((pUartPort->lsr & LSR_DR) != 0);
}

static void output(int port, const unsigned char *buff, int len)
{
	int i=0;
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	unsigned char ier;

	ier = pUartPort->ier;
	pUartPort->ier = 0;
	
	while (len--){
		mvUartPutc2(port,buff[i++]);
	}
	pUartPort->ier = ier;
}

static int input(int port, unsigned char *ch, unsigned tmout_ms)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	unsigned char status=0xff;
	unsigned char ier;
	
	ier = pUartPort->ier;
	pUartPort->ier = 0;
	
	tmout_ms /= 10;
	if (tmout_ms==0){
		tmout_ms=1;
	}
	
	do{
		if (--tmout_ms == 0){
#ifdef MICONMSG
			printk("DATA_RDY : Time out.\n");
#endif
			break;
		}
		mvOsDelay(1);  /* ms */
		status = mvUartTstc2(port);
#ifdef MICONMSG
		//printk("status=%x\n",status);
#endif
	} while (!status);
	
#ifdef MICONMSG
	printk(">%s:port=%d: status=%x tmout=%x\n",__FUNCTION__,port,status,tmout_ms);
#endif
	if (tmout_ms == 0){
		return -1;
	}
	*ch = mvUartGetc2(port);
	pUartPort->ier = ier;
	return 0;
}

void BuffaloInitUpsUartPort(void)
{
#if defined(CONFIG_ARCH_FEROCEON_MV78XX0) && defined(CONFIG_BUFFALO_USE_UPS)
	// Overwrite uart's ops for UPS port.
	{
		struct uart_ops *upsport_ops;
		
		upsport_ops = kmalloc(sizeof(struct uart_ops), GFP_KERNEL);
		if (upsport_ops != NULL) {
			memcpy(upsport_ops, uart_ports[UPSPORT]->ops, sizeof(struct uart_ops));
			upsport_ops->set_mctrl = BuffaloUps_set_mctrl;
			upsport_ops->get_mctrl = BuffaloUps_get_mctrl;
			uart_ports[UPSPORT]->ops = upsport_ops;
		}
	}
#endif /* CONFIG_ARCH_FEROCEON_MV78XX0 && CONFIG_BUFFALO_USE_UPS */
}

//----------------------------------------------------------------------
// Initialize 
//----------------------------------------------------------------------
void BuffaloInitUart(void)
{
	unsigned baseclk=mvBoardTclkGet()/16;
	//unsigned char tmp;
	
	//printk("-- %s %x %x --\n",__FUNCTION__,baseclk/115200,baseclk/38400);
	
	// test : mvUartInit2(CONSOLEPORT, baseclk/115200);	/* 115200 */
	mvUartInit2(MICONPORT,   baseclk/38400);
	
	// test : BuffaloConsoleOutput("123456789123456789");
	
	return;
}

//----------------------------------------------------------------------
// for Console (port=0)
//----------------------------------------------------------------------
void BuffaloConsoleOutput(const unsigned char *buff)
{
	output(CONSOLEPORT,buff,strlen(buff));
}

int BuffaloConsoleInput(unsigned char *ch, unsigned tmout)
{
	return input(MICONPORT,ch,tmout);
}

//----------------------------------------------------------------------
// for Micon (port=1)
//----------------------------------------------------------------------
void BuffaloMiconOutput(const unsigned char *buff, int len)
{
	output(MICONPORT,buff,len);
}

int BuffaloMiconInput(unsigned char *ch, unsigned tmout)
{
	return input(MICONPORT,ch,tmout);
}

#if defined(CONFIG_ARCH_FEROCEON_MV78XX0) && defined(CONFIG_BUFFALO_USE_UPS)
//----------------------------------------------------------------------
// for UPS (port=2)
//----------------------------------------------------------------------
static unsigned int BuffaloUps_get_mctrl(struct uart_port *port)
{
	unsigned int status;
	unsigned int ret;

//printk("%s : ttyS2 mctrl get", __func__);
	ret = 0;

	status = BuffaloGpio_ReadMSR_UPSPort();
	if (status & UART_MSR_DCD)
		ret |= TIOCM_CAR;
	if (status & UART_MSR_RI)
		ret |= TIOCM_RNG;
	if (status & UART_MSR_DSR)
		ret |= TIOCM_DSR;

	if (BuffaloMctrlMode == BUFFALO_MODEM_CTRL_MODE_MPP)
		status = readb(port->membase + (UART_MSR << port->regshift));

	if (status & UART_MSR_CTS)
		ret |= TIOCM_CTS;
	if (status & BUFFALO_UART_MSR_SR)
		ret |= TIOCM_SR;

//printk(": 0x%x\n", ret);
	return ret;
}

static void BuffaloUps_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	unsigned char mcr = 0;
	extern int buffalo_booting;

//printk("%s : ttyS2 mctrl set: mctrl=0x%x\n", __func__, mctrl);

	if (mctrl & TIOCM_DTR)
		mcr |= UART_MCR_DTR;
	if (mctrl & TIOCM_RTS)
		mcr |= UART_MCR_RTS;
	if (mctrl & TIOCM_LOOP)
		mcr |= UART_MCR_LOOP;

	if(buffalo_booting == 1)
		mcr |= UART_MCR_DTR;

	writeb(mcr, port->membase + (UART_MCR << port->regshift));

#if defined(CONTROL_ST_IN_MCTRL)
	if (BuffaloMctrlMode == BUFFALO_MODEM_CTRL_MODE_GPIO)
		if(mctrl & TIOCM_ST)
			mcr |= BUFFALO_UART_MCR_ST;
#endif

	BuffaloGpio_WriteMCR_UPSPort(mcr);
}
#endif /* CONFIG_ARCH_FEROCEON_MV78XX0 */
