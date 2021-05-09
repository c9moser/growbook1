/***************************************************************************
 *            strptime.h
 *
 ****************************************************************************/

#ifndef __GROWBOOK_STRPTIME_H__
#define __GROWBOOK_STRPTIME_H__

#ifdef NAITVE_WIN32
#ifdef __cplusplus
extern "C" {
#endif /* C++ */
	
char * strptime(const char *s, const char *format, struct tm *tm);

#ifdef __cplusplus
}
#endif /* C++ */
		
#endif /* NATIVE_WIN32 */

#endif /* __GROWBOOK_STRPTIME_H__ */
