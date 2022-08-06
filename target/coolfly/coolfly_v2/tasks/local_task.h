#ifndef __LOCAL_TASK_H__
#define __LOCAL_TASK_H__


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct {
	uint32_t ch[16];
	uint64_t timestamp;
} cf_rc_s;

typedef struct  {
	uint8_t sbus_buff[25];
	uint64_t timestamp;
} cf_sbus_s;


#ifdef __cplusplus
}
#endif

#endif

