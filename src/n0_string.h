/*
 * file name: n0_string.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-30
 * history  :
 *	 20160330 实现以非'\0'结尾的字符串处理
 */

#ifndef __N0_STRING_H__
#define __N0_STRING_H__

namespace n0_string {
	unsigned long int strtoul(const char* s, int len, int base);
}

#endif // __N0_STRING_H__
