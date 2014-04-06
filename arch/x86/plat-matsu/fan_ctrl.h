#if defined(__FAN_CTRL_H__)
#else

#define __FAN_CTRL_H__

//IOCTL interface command
#define GET_TEMP_CMD_BASE		0x100
#define MAX_GET_TEMP			3
#define GET_FAN_CMD_BASE		0x200
#define MAX_GET_FAN			4
#define SET_FAN_CMD_BASE		0x400
#define MAX_SET_FAN			3
#define IOCTL_CMD_MASK			0xFFFFFF00

#define GET_TEMP(x)		((x >= 0 && x < MAX_GET_TEMP)? (GET_TEMP_CMD_BASE + x):-1)
#define GET_FAN(x)		((x >= 0 && x < MAX_GET_FAN)? (GET_FAN_CMD_BASE + x):-1)
#define SET_FAN(x)		((x >= 0 && x < MAX_SET_FAN)? (SET_FAN_CMD_BASE + x):-1)

#endif
