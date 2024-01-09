#include <stdio.h>
#include "data_format.h"
#include "cmd_fun.h"
#include "xil_types.h"
#include "xil_io.h"
#define MYDEBUG  //printf
/*0xaa 0x2d 0x01 0x00 0x10 0x00 0x00 0x3e
aa 2e 02 12 34 56 78 01 02 03 04 4e
 * 0xAA
 * 0x55
 * */
/*
 * 0xaa 0x2d 0x00 0x2d
 * 0xaa 0x2d 0x01 0x00 0x10 0x00 0x00 0x3e
 * 0x55
 *
 *is_open_auto_gain_measure
 * set_auto_target_gray_range
 * set_frame_cycle
 * set_extrigger_input_mode
 * set_trigger_input_polar_delay
 * set_ttl_output_polar
 * is_use_white_balance_right
 * is_open_auto_white_balance_measure
 * set_white_balance_para
 * set_color_saturation
 * set_image_process_mode
 * set_HDR_level
 * set_electric_touwu_level
 * set_edge_enhanced_level
 * is_display_cross
 * set_auto_bright_refer_area
 * set_grad_cal_area
 * get_grad_cal_result

 	unsigned int mv_crossair;
	unsigned int imagedirection;
	unsigned int electroniczoom;
	unsigned int voerlayCharacterContent;
 */
#define CONFIGREG_BASEADDR 			0x43c00000
#define SYSTEM_STARUS_OFFSET 		0x20848		//0x8212<<2

#define  MV_CROSSAIR_CURSOR			0x204B8		//0x812E<<2
#define  IAMGE_DIRECTION			0x204C4		//0x8131<<2
#define  ELECTRIC_ZOOM			    0x204C0		//0x8130<<2
#define  OVERLAY_CHAR_CONTENT	    0x20640		//0x8190<<2

#define EXPOSURE_TIME_OFFSET 		0x204A0		//0x8128<<2
#define EXPOSURE_TIME_RANGE_MIN 	0x20474		//0x811D<<2
#define EXPOSURE_TIME_RANGE_MAX 	0x20478		//0x811E<<2
#define AUTO_EXPOSURE_TIME 			0x204E4		//0x8139<<2
#define CAMERA_GAIN_OFFSET 			0x204A4		//0x8129<<2
#define CAMERA_GAIN_RANGE_MIN 		0x2048C		//0x8123<<2
#define CAMERA_GAIN_RANGE_MAX 		0x20490		//0x8124<<2

#define ATUO_GAIN_MEASURE 			0x204E8		//0x813A<<2
#define AUTO_TARGET_GRAY_RANGE 		0x204EC 	//0x813B<<2
#define CAMERA_FRAME_CYCLE 			0x204CC		//0x8133<<2
#define EXTRIGGER_INPUT_MODE 		0x20508		//0x8142<<2
#define TTIGGER_INPUT_POLAR_DELAY 	0x2050C		//0x8143<<2
#define TTL_OUTPUT_POLAR 			0x20524		//0x8149<<2
#define WHITE_BALANCE_RIGHT 		0
#define AUTO_WHITE_BALANCE_MEASURE 	0x2000C 	//0x8003<<2
#define WHITE_BALANCE_PARA_R 		0x20020		//0x8008<<2
#define WHITE_BALANCE_PARA_B 		0x20024		//0x8009<<2
#define COLOR_SATURATION_1 			0x20494		//0x8125
#define COLOR_SATURATION_2 			0x204AC		//0x812B
#define IMAGE_PROCESS_MODE 			0x200A0		//0x8028<<2
#define CAMERA_HDR_LEVEL 			0
#define ELECTRIC_TOUWU_LEVEL 		0x204B4		//0x812D<<2
#define EDGE_ENHANCED_LEVEL 		0x20500		//0x8140<<2
#define	DISPLAY_CTOSS 				0x204BC		//0x812F<<2
#define AUTO_BRIGHT_REFER_X_AREA 	0x20498		//0x8126<<2
#define AUTO_BRIGHT_REFER_Y_AREA 	0x2049C		//0x8127<<2
#define GRAD_CAL_AREA_X 			0x20460		//0x8118<<2
#define GRAD_CAL_AREA_Y				0x20464		//0x8119<<2
#define GRAD_CAL_RESULT 			0x20838		//0x820E<<2
#define GRAD_CAL_RESULT_AVERAGE 	0x2083C		//0x820F<<2

#define CAM_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (u32)(RegOffset))

#define CAM_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	Xil_Out32((BaseAddress) + (u32)(RegOffset), (u32)(RegisterValue))

static camInfo st_camInfo = {0,0,0,400,0x70,0x88};

int system_reboot(void * ptr)
{
	int status = 0;

	MYDEBUG("system_reboot\r\n");
	return status;
}

int get_system_status(void *  ptr,unsigned char *buf, unsigned int len, unsigned char cmd)
{
	int status = 0;
	unsigned char i = 0;
	unsigned char sbuf[10];

	//MYDEBUG("get_system_status\r\n");
	sbuf[i] = 0XAA;
	i++;
	sbuf[i] = cmd;
	i++;
	sbuf[i] = 1;
	i++;

	//sbuf[i] = *((u32*)(0x43c00000+(0x8212<<2)));
	//sbuf[i] = *((u32*)(0x43c00000+SYSTEM_STARUS_OFFSET));
	sbuf[i]=CAM_ReadReg(CONFIGREG_BASEADDR,SYSTEM_STARUS_OFFSET);
	i++;
	sbuf[i] = 0;i++;
	sbuf[i] = 0;i++;
	sbuf[i] = 0;i++;
	sbuf[i] = check_sum(sbuf+1, i-1);
	i++;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, (i));

	return status;
}
void save_eep(unsigned char *buf, unsigned int bytes)
{
	eep_writes(buf, bytes);
}

void read_eep(unsigned char *buf, unsigned int bytes)
{
	eep_reads(buf, bytes);
}
void flush_eep(void)
{
	unsigned int buf[2] = {0,0};
	read_eep((unsigned char *)buf, 8);
	read_eep((unsigned char *)buf, 8);
	if(buf[0] == 0xffffffff)
		buf[0] = (0x3c0|(0x21c<<16));
	if(buf[1] <=3 || buf[1] >= 0)
	{}
	else
		buf[1] = 0;
	st_camInfo.mv_crossair = buf[0];
	st_camInfo.imagedirection = buf[1];

	CAM_WriteReg(CONFIGREG_BASEADDR,MV_CROSSAIR_CURSOR,st_camInfo.mv_crossair);
	CAM_WriteReg(CONFIGREG_BASEADDR,IAMGE_DIRECTION,st_camInfo.imagedirection);
}
int save_cfgpara_to_camera(void *  ptr)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];
	unsigned int buf[2] = {0,0};
/*
 * st_camInfo.mv_crossair
 * st_camInfo.imagedirection
 * */
	buf[0] = st_camInfo.mv_crossair;
	buf[1] = st_camInfo.imagedirection;
	save_eep((unsigned char *)buf, 8);
	/*read_eep((unsigned char *)buf, 8);
	 * st_camInfo.mv_crossair = buf[0];
	 * st_camInfo.imagedirection = buf[1];
	 * */

	//MYDEBUG("save_cfgpara_to_camera\r\n");
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}


int moving_the_crosshair_cursor(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	int val = 0;
	int tmp = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];
	u16 crosshairX_regValue=0;
	u16 crosshairY_regValue=0;
	
	tmp = ((unsigned int *)buf)[0];
	//st_camInfo.mv_crossair = ((unsigned int *)buf)[0];
	MYDEBUG("moving_the_crosshair_cursor %d\r\n",tmp);
	if(0==tmp)
	{

		//x���1 960-1
		
		val = st_camInfo.mv_crossair;//CAM_ReadReg(CONFIGREG_BASEADDR,MV_CROSSAIR_CURSOR);
		crosshairX_regValue=(val&0xffff)-1;
		//MYDEBUG("val  %x\r\n",val);
		crosshairY_regValue = (val>>16);
	}
	else if(1==tmp)
	{
		
		val = st_camInfo.mv_crossair;//CAM_ReadReg(CONFIGREG_BASEADDR,MV_CROSSAIR_CURSOR);
		crosshairX_regValue=(val&0xffff)+1;
		//MYDEBUG("val  %x\r\n",val);
		crosshairY_regValue = (val>>16);
	}
	else if(2==tmp)
	{
		
		val = st_camInfo.mv_crossair;//CAM_ReadReg(CONFIGREG_BASEADDR,MV_CROSSAIR_CURSOR);
		crosshairY_regValue=(val>>16)-1;
				//MYDEBUG("val  %x\r\n",val);
		crosshairX_regValue = (val&0xffff);
	}
	else
	{
		//crosshairY_regValue=0x21C+1;
		val = st_camInfo.mv_crossair;//CAM_ReadReg(CONFIGREG_BASEADDR,MV_CROSSAIR_CURSOR);
		crosshairY_regValue=(val>>16)+1;
				//MYDEBUG("val  %x\r\n",val);
		crosshairX_regValue = (val&0xffff);
	}

	MYDEBUG("crosshairX_regValue 0x%x\r\n",crosshairX_regValue);
	MYDEBUG("crosshairX_regValue 0x%x\r\n",crosshairX_regValue>>8);

	//st_camInfo.mv_crossair=((crosshairX_regValue&0x00ff)<<24)|((crosshairX_regValue>>8)<<16)|((crosshairY_regValue&0x00ff)<<8)|(crosshairY_regValue>>8);
	st_camInfo.mv_crossair=((crosshairX_regValue))|((crosshairY_regValue)<<16);
	MYDEBUG("st_camInfo.mv_crossair 0x%x\r\n",st_camInfo.mv_crossair);

	CAM_WriteReg(CONFIGREG_BASEADDR,MV_CROSSAIR_CURSOR,st_camInfo.mv_crossair);

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}

/*
#define  MV_CROSSAIR_CURSOR			0x204B8		//0x812E<<2
#define  IAMGE_DIRECTION			0x204C4		//0x8131<<2
#define  ELECTRIC_ZOOM			    0x204C0		//0x8130<<2
#define  OVERLAY_CHAR_CONTENT	    0x20640		//0x8190<<2

*/

int image_direction(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];
	unsigned int val = 0;
	
	val = ((unsigned int *)buf)[0];
	//st_camInfo.imagedirection = ((unsigned int *)buf)[0];
	MYDEBUG("image_direction %d\r\n",val);
	
	if(val==0||val==1||val==2||val==3)
	{
		devStatus =	1;
		st_camInfo.imagedirection = val;
		CAM_WriteReg(CONFIGREG_BASEADDR,IAMGE_DIRECTION,st_camInfo.imagedirection);
	}
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}


int electric_zoom(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];

	
	st_camInfo.electroniczoom = ((unsigned int *)buf)[0];
MYDEBUG("electroniczoom %d\r\n",st_camInfo.electroniczoom);
	CAM_WriteReg(CONFIGREG_BASEADDR,ELECTRIC_ZOOM,st_camInfo.electroniczoom);

	
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}


int overlay_char_content(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];

	
	st_camInfo.voerlayCharacterContent = ((unsigned int *)buf)[0];
	MYDEBUG("voerlayCharacterContent %d\r\n",st_camInfo.voerlayCharacterContent);
	CAM_WriteReg(CONFIGREG_BASEADDR,OVERLAY_CHAR_CONTENT,st_camInfo.voerlayCharacterContent);
	
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}


int set_exposure_time(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];



	st_camInfo.exposureTime = *((unsigned int *)buf);
#if 0
	if(st_camInfo.exposureTime>=st_camInfo.exposureTimeMin && st_camInfo.exposureTime<=st_camInfo.exposureTimeMax)
	{
		devStatus = 1;
		MYDEBUG("exposure time legal\r\n");
	}
#endif
	//*(u32*)(0x43c00000+(0x8128<<2)) =st_camInfo.exposureTime;

	MYDEBUG("set_exposure_time %d\r\n", st_camInfo.exposureTime);
	CAM_WriteReg(CONFIGREG_BASEADDR,EXPOSURE_TIME_OFFSET,st_camInfo.exposureTime);
	devStatus = 1;

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;
	
	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);
	return status;
}

int set_exposure_time_range(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];
	unsigned int exposure_time_rangeValue=0;

	/*	st_camInfo.exposureTimeMin = buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24);
	st_camInfo.exposureTimeMax = buf[4]|(buf[5]<<8)|(buf[6]<<16)|(buf[7]<<24);*/
	st_camInfo.exposureTimeMin = ((unsigned int *)buf)[0];
	st_camInfo.exposureTimeMax = ((unsigned int *)buf)[1];

	MYDEBUG("set_exposure_time_range %d %d\r\n", st_camInfo.exposureTimeMin, st_camInfo.exposureTimeMax);
	//exposure_time_rangeValue=(st_camInfo.exposureTimeMin&0xffff0000)|(st_camInfo.exposureTimeMax>>16);
	CAM_WriteReg(CONFIGREG_BASEADDR,EXPOSURE_TIME_RANGE_MIN,st_camInfo.exposureTimeMin);
	CAM_WriteReg(CONFIGREG_BASEADDR,EXPOSURE_TIME_RANGE_MAX,st_camInfo.exposureTimeMax);


	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
//AA 30 01 00 00 00 00 31   AA 30 01 01 00 00 00 32
int is_open_auto_exposure(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];
	unsigned int val = 0;
	//st_camInfo.openAutoExposure = ((unsigned int *)buf)[0];
	val = ((unsigned int *)buf)[0];
	MYDEBUG("is_open_auto_exposure %d\r\n",val);
	
	if(val==0||val==1)
	{
		
		devStatus =	1;
		st_camInfo.openAutoExposure = val;
		CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_EXPOSURE_TIME,st_camInfo.openAutoExposure);
	}
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}

int set_gain(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];
	st_camInfo.gainMin=0x0;
	st_camInfo.gainMax=0x190;


	st_camInfo.gain = ((unsigned int *)buf)[0];
//	if(st_camInfo.gain>=st_camInfo.gainMin && st_camInfo.gain<=st_camInfo.gainMax)
//		MYDEBUG("gain legal(range:0x0~0x190)\r\n");
//	else
//		devStatus=1;

	MYDEBUG("set_gain %d\r\n",st_camInfo.gain);
	devStatus =1;
	CAM_WriteReg(CONFIGREG_BASEADDR,CAMERA_GAIN_OFFSET,st_camInfo.gain);

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}

int set_gain_range(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];

	st_camInfo.gainMin = ((unsigned int *)buf)[0];
	st_camInfo.gainMax = ((unsigned int *)buf)[1];
	MYDEBUG("set_gain_range %d %d\r\n", st_camInfo.gainMin, st_camInfo.gainMax);
	//st_camInfo.gain=(st_camInfo.gainMin&0xffff0000)|(st_camInfo.gainMax>>16);
	CAM_WriteReg(CONFIGREG_BASEADDR,CAMERA_GAIN_RANGE_MIN,st_camInfo.gainMin);
	CAM_WriteReg(CONFIGREG_BASEADDR,CAMERA_GAIN_RANGE_MAX,st_camInfo.gainMax);

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}

int is_open_auto_gain_measure(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];
	unsigned char flag = 0;
	unsigned int val = 0;

	//st_camInfo.openAutoGainMeasure = ((unsigned int *)buf)[0];
	val = ((unsigned int *)buf)[0];
	MYDEBUG("is_open_auto_gain_measure %d\r\n",val);

	if((0==val)||(1==val))
	{
		devStatus=1;
		st_camInfo.openAutoGainMeasure = val;
		CAM_WriteReg(CONFIGREG_BASEADDR,ATUO_GAIN_MEASURE,st_camInfo.openAutoGainMeasure);
	}
	

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}

int set_auto_target_gray_range(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];

	st_camInfo.grayRangeMin = ((unsigned int *)buf)[0];
	st_camInfo.grayRangeMax = ((unsigned int *)buf)[1];
	MYDEBUG("set_gain_range min: %d max: %d\r\n", st_camInfo.grayRangeMin, st_camInfo.grayRangeMax);

	st_camInfo.grayRange=(((st_camInfo.grayRangeMin+st_camInfo.grayRangeMax)/2)&0xff|(0x20<<8));
	MYDEBUG("set_gain_range %d\r\n", st_camInfo.grayRange);
	CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_TARGET_GRAY_RANGE,st_camInfo.grayRange);


	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}




int set_frame_cycle(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];

	st_camInfo.frameCycle = ((unsigned int *)buf)[0];

	//CAM_WriteReg(CONFIGREG_BASEADDR,CAMERA_FRAME_CYCLE,st_camInfo.frameCycle);

	//MYDEBUG("set_frame_cycle %d\r\n", st_camInfo.frameCycle);
	//if(st_camInfo.frameCycle>10)
		devStatus =1;
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;


	//*(u32*)(0x43c00000+(0x8133<<2)) =st_camInfo.frameCycle;
	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}

int set_extrigger_input_mode(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];
	unsigned int val = 0;

	//st_camInfo.extriggerInputMode = ((unsigned int *)buf)[0];
	val = ((unsigned int *)buf)[0];
	
	MYDEBUG("set_extrigger_input_mode %d\r\n", val);
	if(val>=0&&val<4)
	{
		devStatus=1;
		st_camInfo.extriggerInputMode = val;
		CAM_WriteReg(CONFIGREG_BASEADDR,EXTRIGGER_INPUT_MODE,st_camInfo.extriggerInputMode);
	}
	else
	{
		devStatus=0;
		MYDEBUG("extriggerInputMode is invilid\r\n");
	}
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
int set_trigger_input_polar_delay(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];
	unsigned int val = 0;


	//st_camInfo.triggerInputPolarDelay = ((unsigned int *)buf)[0];
	val = ((unsigned int *)buf)[0];
	
	MYDEBUG("set_trigger_input_polar_delay %d\r\n", val);

	if(val==0||val==1)
	{
		devStatus=1;
		st_camInfo.triggerInputPolarDelay = val;
		CAM_WriteReg(CONFIGREG_BASEADDR,TTIGGER_INPUT_POLAR_DELAY,st_camInfo.triggerInputPolarDelay);
	}
		
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
int set_ttl_output_polar(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];
	unsigned int val = 0;

	//st_camInfo.ttlOutputPolar = ((unsigned int *)buf)[0];
	val = ((unsigned int *)buf)[0];
	
	MYDEBUG("set_ttl_output_polar %d\r\n", val);
	if(val==0||val==1)
	{
		devStatus=1;
		st_camInfo.ttlOutputPolar = val;
		CAM_WriteReg(CONFIGREG_BASEADDR,TTL_OUTPUT_POLAR,st_camInfo.ttlOutputPolar);
	}
			
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
//��ƽ��У��
int is_use_white_balance_right(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];

	st_camInfo.useWhiteBalanceRight = ((unsigned int *)buf)[0];
	MYDEBUG("is_use_white_balance_right %d\r\n", st_camInfo.useWhiteBalanceRight);

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
int is_open_auto_white_balance_measure(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];
	unsigned int val = 0;

	//st_camInfo.openAuWhBaMea = ((unsigned int *)buf)[0];
	val = ((unsigned int *)buf)[0];
	
	MYDEBUG("is_open_auto_white_balance_measure %d\r\n", val);
	if(val==0||val==1)
	{
		devStatus=1;
		st_camInfo.openAuWhBaMea = val;
		CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_WHITE_BALANCE_MEASURE,st_camInfo.openAuWhBaMea);
	}
		
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
int set_white_balance_para(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];

	st_camInfo.whBaPParaR = ((unsigned int *)buf)[0];
	st_camInfo.whBaPParaG = ((unsigned int *)buf)[1];
	st_camInfo.whBaPParaB = ((unsigned int *)buf)[2];
	MYDEBUG("set_white_balance_para r %d\r\n",st_camInfo.whBaPParaR);
	MYDEBUG("set_white_balance_para g %d\r\n",st_camInfo.whBaPParaG);
	MYDEBUG("set_white_balance_para b %d\r\n",st_camInfo.whBaPParaB);
	CAM_WriteReg(CONFIGREG_BASEADDR,WHITE_BALANCE_PARA_R,st_camInfo.whBaPParaR);
	CAM_WriteReg(CONFIGREG_BASEADDR,WHITE_BALANCE_PARA_B,st_camInfo.whBaPParaB);

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
//����ɫ�ʱ��Ͷ�
int set_color_saturation(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];

	st_camInfo.colorSaturation = ((unsigned int *)buf)[0];
	MYDEBUG("set_color_saturation r %d\r\n",st_camInfo.colorSaturation);
	if(st_camInfo.colorSaturation==0)
	{
		CAM_WriteReg(CONFIGREG_BASEADDR,COLOR_SATURATION_1,0x01);
		CAM_WriteReg(CONFIGREG_BASEADDR,COLOR_SATURATION_2,0x00000000);
	}

	if(st_camInfo.colorSaturation>=4096&&st_camInfo.colorSaturation<=(4096+511))
	{
		CAM_WriteReg(CONFIGREG_BASEADDR,COLOR_SATURATION_1,0x00000000);
		CAM_WriteReg(CONFIGREG_BASEADDR,COLOR_SATURATION_2,0x00000000);
	}

	if(st_camInfo.colorSaturation>=(4096+512)&&st_camInfo.colorSaturation<=(4096+1023))
	{
		CAM_WriteReg(CONFIGREG_BASEADDR,COLOR_SATURATION_1,0x00000000);
		CAM_WriteReg(CONFIGREG_BASEADDR,COLOR_SATURATION_2,0x01);
	}

	if(st_camInfo.colorSaturation>=(4096+1024)&&st_camInfo.colorSaturation<=(4096+1024+512))
	{
		CAM_WriteReg(CONFIGREG_BASEADDR,COLOR_SATURATION_1,0x00000000);
		CAM_WriteReg(CONFIGREG_BASEADDR,COLOR_SATURATION_2,0x02);
	}

	if(st_camInfo.colorSaturation>=(4096+1536)&&st_camInfo.colorSaturation<=(4096+2047))
	{
		CAM_WriteReg(CONFIGREG_BASEADDR,COLOR_SATURATION_1,0x00000000);
		CAM_WriteReg(CONFIGREG_BASEADDR,COLOR_SATURATION_2,0x03);
	}


	devStatus=1;

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}

int set_image_process_mode(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];

	st_camInfo.imageProcessMode = ((unsigned int *)buf)[0];
	//CAM_WriteReg(CONFIGREG_BASEADDR,IMAGE_PROCESS_MODE,st_camInfo.imageProcessMode);
	MYDEBUG("set_image_process_mode r %d\r\n",st_camInfo.imageProcessMode);
	if(st_camInfo.imageProcessMode>=0&&st_camInfo.imageProcessMode<5)
	{
		devStatus=1;
	}
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
//���ÿ���̬HDR��λ
int set_HDR_level(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];

	st_camInfo.HDRLevel = ((unsigned int *)buf)[0];
	MYDEBUG("set_HDR_level r %d\r\n",st_camInfo.HDRLevel);
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
//���õ���͸����λ
int set_electric_touwu_level(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];

	st_camInfo.electricTouwuLevel = ((unsigned int *)buf)[0];
	CAM_WriteReg(CONFIGREG_BASEADDR,ELECTRIC_TOUWU_LEVEL,st_camInfo.electricTouwuLevel);
	MYDEBUG("set_electric_touwu_level r %d\r\n",st_camInfo.electricTouwuLevel);
	//if(st_camInfo.electricTouwuLevel>=0&&st_camInfo.electricTouwuLevel<5)
	{
		devStatus=1;
	}

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
//���ñ�Ե��ǿ��λ
int set_edge_enhanced_level(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];

	st_camInfo.edgeEnhancedLevel = ((unsigned int *)buf)[0];
	MYDEBUG("set_edge_enhanced_level r %d\r\n",st_camInfo.edgeEnhancedLevel);
	CAM_WriteReg(CONFIGREG_BASEADDR,EDGE_ENHANCED_LEVEL,st_camInfo.edgeEnhancedLevel);
	//if(st_camInfo.edgeEnhancedLevel>=0&&st_camInfo.edgeEnhancedLevel<=5)
	{
		devStatus=1;
	}

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}

//�Ƿ���ʾʮ�ֹ��
int is_display_cross(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 0;
	unsigned char sbuf[1];

	st_camInfo.displayCross = ((unsigned int *)buf)[0];
	MYDEBUG("is_display_cross r %d\r\n",st_camInfo.displayCross);

	if(st_camInfo.displayCross==0)
		CAM_WriteReg(CONFIGREG_BASEADDR,DISPLAY_CTOSS,st_camInfo.displayCross);
	if(st_camInfo.displayCross==2)
			CAM_WriteReg(CONFIGREG_BASEADDR,DISPLAY_CTOSS,0x01);

	//if(st_camInfo.displayCross==0||st_camInfo.displayCross==2)
	{
		devStatus=1;
	}
	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
//�����Զ����ȵ����ο�����
int set_auto_bright_refer_area(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];
	unsigned int BtightArea_Xvalue=0;
	unsigned int BtightArea_Yvalue=0;

	st_camInfo.autoBrightReferArea = ((unsigned int *)buf)[0];
	MYDEBUG("set_auto_bright_refer_area r %d\r\n",st_camInfo.autoBrightReferArea);

	switch(st_camInfo.autoBrightReferArea)
	{
		case 0:
			BtightArea_Xvalue=0x071C0064;
			BtightArea_Yvalue=0x03D40064;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;

		case 1:
			BtightArea_Xvalue=0x05a001e0;// 0xE001A005;
			BtightArea_Yvalue=0x032a010e;//0x0E012A03;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 2:
			BtightArea_Xvalue=0x071c0064;//0x64001C07;
			BtightArea_Yvalue=0x021c0064;//0x64001C02;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 3:
			BtightArea_Xvalue=0x071c0064;//0x64001C07;
			BtightArea_Yvalue=0x03d4021c;//0x1C02D403;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 4:
			BtightArea_Xvalue=0x03c00064;//0x6400C003;
			BtightArea_Yvalue=0x03d40064;//0x6400D403;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 5:
			BtightArea_Xvalue=0x071c03c0;//0xC0031C07;
			BtightArea_Yvalue=0x03d40064;//0x6400D403;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 6:
			BtightArea_Xvalue=0x071c0064;//0x64001C07;
			BtightArea_Yvalue=0x01680064;//0x64006801;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 7:
			BtightArea_Xvalue=0x071C0064;//0x64001C07;
			BtightArea_Yvalue=0x03D402D0;//0xD002D403;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 8:
			BtightArea_Xvalue=0x02800064;//0x64008002;
			BtightArea_Yvalue=0x03D40064;//0x6400D403;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 9:
			BtightArea_Xvalue=0x071C0500;//0x00051C07;
			BtightArea_Yvalue=0x03D40064;//0x6400D403;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 10:
			BtightArea_Xvalue=0x071C0064;//0x64001C07;
			BtightArea_Yvalue=0x02D00064;//0x6400D002;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 11:
			BtightArea_Xvalue=0x071C0064;//0x64001C07;
			BtightArea_Yvalue=0x03D4022D;//0x2D02D403;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 12:
			BtightArea_Xvalue=0x50000064;//0x64000050;
			BtightArea_Yvalue=0x03D40064;//0x6400D403;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 13:
			BtightArea_Xvalue=0x071C02D0;//0xD0021C07;
			BtightArea_Yvalue=0x03D40064;//0x6400D403;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 14:
			BtightArea_Xvalue=0x04B0002D;//0x2D00B004;
			BtightArea_Yvalue=0x02A3022D;//0x2D02A302;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		case 15:
			BtightArea_Xvalue=0x042A0356;//0x56032A04;
			BtightArea_Yvalue=0x025801E0;//0xE0015802;
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_X_AREA,BtightArea_Xvalue);
			CAM_WriteReg(CONFIGREG_BASEADDR,AUTO_BRIGHT_REFER_Y_AREA,BtightArea_Yvalue);
			break;
		  default:
			  xil_printf("error\n");
			  break;
	}

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
//�����ݶȼ�������Χ
int set_grad_cal_area(void *  ptr,unsigned char *buf, unsigned int len)
{
	int status = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[1];

	unsigned int grad_area_Xvalue=0;
	unsigned int grad_area_XvalueStart=0;
	unsigned int grad_area_XvalueEND=0;
	unsigned int grad_area_Yvalue=0;
	unsigned int grad_area_YvalueStart=0;
	unsigned int grad_area_YvalueEND=0;


	st_camInfo.gradCalArea[0] = ((unsigned int *)buf)[0];
	st_camInfo.gradCalArea[1] = ((unsigned int *)buf)[1];
	st_camInfo.gradCalArea[2] = ((unsigned int *)buf)[2];
	st_camInfo.gradCalArea[3] = ((unsigned int *)buf)[3];
	MYDEBUG("set_grad_cal_area r %d\r\n",st_camInfo.gradCalArea[0]);
	MYDEBUG("set_grad_cal_area r %d\r\n",st_camInfo.gradCalArea[1]);
	MYDEBUG("set_grad_cal_area r %d\r\n",st_camInfo.gradCalArea[2]);
	MYDEBUG("set_grad_cal_area r %d\r\n",st_camInfo.gradCalArea[3]);

	grad_area_XvalueStart=st_camInfo.gradCalArea[0];
	grad_area_XvalueEND =(st_camInfo.gradCalArea[0]+st_camInfo.gradCalArea[2])&0xffff;
	grad_area_Xvalue=grad_area_XvalueStart|(grad_area_XvalueEND<<16);

	CAM_WriteReg(CONFIGREG_BASEADDR,GRAD_CAL_AREA_X,grad_area_Xvalue);


	grad_area_YvalueStart=st_camInfo.gradCalArea[1];
	grad_area_YvalueEND =(st_camInfo.gradCalArea[1]+st_camInfo.gradCalArea[3])&0xffff;
	grad_area_Yvalue=grad_area_YvalueStart|(grad_area_YvalueEND<<16);
	CAM_WriteReg(CONFIGREG_BASEADDR,GRAD_CAL_AREA_Y,grad_area_Yvalue);

	if(devStatus)
		sbuf[0] = MSG_HEADER;
	else
		sbuf[0] = MSG_HEADER_FAILED;
	fun_callbackInfoDT *pfun = ptr;

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, 1);

	return status;
}
static unsigned int serialQuriFlag = 0;

unsigned int get_self_data(unsigned char *buf)
{
	unsigned int bytes = 0;
	if(serialQuriFlag == 0x00)
	{
		bytes = 0;
	}
	else if(serialQuriFlag == 0x01)
	{
		memcpy(buf, "gvop 1", 6);
		bytes = 6;
	}
	else if(serialQuriFlag == 0x02)
	{
		memcpy(buf, "gvop 2", 6);
		bytes = 6;
	}
	else if(serialQuriFlag == 0x03)
	{
		memcpy(buf, "gvop 3", 6);
		bytes = 6;
	}
	else
		bytes = 0;
	return bytes;
}
//��ȡ�ݶȼ�����,���Ͳ�������
int get_grad_cal_result(void *  ptr,unsigned char *buf, unsigned int len, unsigned char cmd)
{
	int status = 0;
	unsigned char i = 0;
	unsigned char devStatus = 1;
	unsigned char sbuf[10];
	unsigned int val = 100;
	unsigned int val1 = 1000;
	fun_callbackInfoDT *pfun = ptr;


	MYDEBUG("get_grad_cal_result\r\n");
	if(devStatus)
		sbuf[i] = MSG_HEADER;
	else
	{
		sbuf[i] = MSG_HEADER_FAILED;
		i++;
		goto __next;
	}


	st_camInfo.gradCalResult = CAM_ReadReg(CONFIGREG_BASEADDR,GRAD_CAL_RESULT);//val;
	st_camInfo.averageGradVal = CAM_ReadReg(CONFIGREG_BASEADDR,GRAD_CAL_RESULT_AVERAGE);
	MYDEBUG("set_grad_cal_area r %d\r\n",st_camInfo.gradCalResult);
	i++;
	if(len == 1)
	{
		unsigned int dataInfo = 0;
		dataInfo = ((unsigned int *)buf)[0];
		if(dataInfo == 0x01)
		{
			serialQuriFlag = 0x01;
			sbuf[i] = (st_camInfo.gradCalResult &0xff);i++;
			sbuf[i] = ((st_camInfo.gradCalResult>>8) &0xff);i++;
			sbuf[i] = ((st_camInfo.gradCalResult>>16)  &0xff);i++;
			sbuf[i] = ((st_camInfo.gradCalResult>>24)  &0xff);i++;
			sbuf[i] = check_sum(sbuf+1, i-1);
			i++;
		}
		else if(dataInfo == 0x02)
		{
			serialQuriFlag = 0x02;
			sbuf[i] = (st_camInfo.averageGradVal&0xff);i++;
			sbuf[i] = check_sum(sbuf+1, i-1);
			i++;
		}
		else if(dataInfo == 0x03)
		{
			serialQuriFlag = 0x03;
			sbuf[i] = (st_camInfo.gradCalResult &0xff);i++;
			sbuf[i] = ((st_camInfo.gradCalResult>>8) &0xff);i++;
			sbuf[i] = ((st_camInfo.gradCalResult>>16)  &0xff);i++;
			sbuf[i] = ((st_camInfo.gradCalResult>>24)  &0xff);i++;
			sbuf[i] = (st_camInfo.averageGradVal&0xff);i++;
			sbuf[i] = check_sum(sbuf+1, i-1);
			i++;
		}
		else if(dataInfo == 0x00)
		{
			serialQuriFlag = 0x00;
			goto __next;
		}
		else
		{
			serialQuriFlag = 0x00;
			sbuf[0] = MSG_HEADER_FAILED;
			goto __next;
		}
			
	}
	else
	{

		
		sbuf[i] = cmd;i++;
		sbuf[i] = 2;i++;
		sbuf[i] = (st_camInfo.gradCalResult &0xff);i++;
		sbuf[i] = ((st_camInfo.gradCalResult>>8) &0xff);i++;
		sbuf[i] = ((st_camInfo.gradCalResult>>16)  &0xff);i++;
		sbuf[i] = ((st_camInfo.gradCalResult>>24)  &0xff);i++;
		sbuf[i] = (st_camInfo.averageGradVal &0xff);i++;
		sbuf[i] = ((st_camInfo.averageGradVal>>8) &0xff);i++;
		sbuf[i] = ((st_camInfo.averageGradVal>>16)  &0xff);i++;
		sbuf[i] = ((st_camInfo.averageGradVal>>24)  &0xff);i++;
		sbuf[i] = check_sum(sbuf+1, i-1);
		i++;
	}
	
__next:

	if(pfun != 0 && pfun->func != 0)
		status = pfun->func(pfun->para[0], sbuf, (i));

	return status;
}

static cmdFunHandle cmdFunHandleTable[MSG_CMD_SUM];


unsigned char cmd_deal(unsigned char cmd, unsigned char * buf, unsigned int len, void *  ptr)
{
	unsigned char res;

	if (0 == cmdFunHandleTable[cmd])
		return ERROR_FUN_NULL;

	res = cmdFunHandleTable[cmd](ptr,buf,len,cmd);

	return res;
} 
unsigned char cmd_fun_init(void *ptr)
{
	unsigned char res;
	int i = 0;

	for(i = 0; i < MSG_CMD_SUM; i++)
	cmdFunHandleTable[i] = 0;
	cmdFunHandleTable[0x01] = system_reboot;
	cmdFunHandleTable[0x0a] = get_system_status;
	cmdFunHandleTable[0x10] = save_cfgpara_to_camera;
	cmdFunHandleTable[0x6B] = moving_the_crosshair_cursor;//image_direction   electric_zoom
	cmdFunHandleTable[0x20] = image_direction;
	cmdFunHandleTable[0x60] = electric_zoom;
	cmdFunHandleTable[0x2d] = set_exposure_time;
	cmdFunHandleTable[0x2e] = set_exposure_time_range;
	cmdFunHandleTable[0x30] = is_open_auto_exposure;
	cmdFunHandleTable[0x32] = set_gain;
	cmdFunHandleTable[0x33] = set_gain_range;
	cmdFunHandleTable[0x34] = is_open_auto_gain_measure;
	cmdFunHandleTable[0x2f] = set_auto_target_gray_range;
	cmdFunHandleTable[0x42] = set_frame_cycle;
	cmdFunHandleTable[0x50] = set_extrigger_input_mode;
	cmdFunHandleTable[0x51] = set_trigger_input_polar_delay;
	cmdFunHandleTable[0x52] = set_ttl_output_polar;
	cmdFunHandleTable[0x1a] = is_use_white_balance_right;
	cmdFunHandleTable[0x19] = is_open_auto_white_balance_measure;
	cmdFunHandleTable[0x17] = set_white_balance_para;
	cmdFunHandleTable[0x74] = set_color_saturation;
	cmdFunHandleTable[0x4c] = set_image_process_mode;
	cmdFunHandleTable[0x5c] = set_HDR_level;
	cmdFunHandleTable[0x73] = set_electric_touwu_level;
	cmdFunHandleTable[0x25] = set_edge_enhanced_level;
	cmdFunHandleTable[0x6a] = is_display_cross;
	cmdFunHandleTable[0x4f] = set_auto_bright_refer_area;
	cmdFunHandleTable[0x70] = set_grad_cal_area;
	cmdFunHandleTable[0x6f] = get_grad_cal_result;
	return res;
}
