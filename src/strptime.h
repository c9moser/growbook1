/***************************************************************************
 *            strptime.h
 *
 ****************************************************************************/

#ifndef __GROWBOOK_STRPTIME_H__
#define __GROWBOOK_STRPTIME_H__

#ifdef __cplusplus
extern "C" {
#endif /* C++ */
#include <time.h>
	
char * strptime(const char *s, const char *format, struct tm *tm);

#ifdef __cplusplus
}
#endif /* C++ */
		

#endif /* __GROWBOOK_STRPTIME_H__ */
