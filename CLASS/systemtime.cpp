#include "systemtime.h"
#include <windows.h>
//在jwt.cpp中直接包含windows.h头文件，有冲突。
//所以才通过这种方式……
SystemTime::SystemTime()
{

}

void SystemTime::SetDate(const char *date)
{
    WinExec(date,SW_HIDE);
}
