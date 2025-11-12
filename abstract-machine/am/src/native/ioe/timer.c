#include <am.h>
#include <sys/time.h>
#include <time.h>

static struct timeval boot_time = {};

void __am_timer_config(AM_TIMER_CONFIG_T *cfg) {
  // 表示定时器设备可用,并支持实时时钟功能
  cfg->present = cfg->has_rtc = true;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  // time() 函数（来自 <time.h>）：
  // 返回自 Unix 纪元（1970-01-01 00:00:00 UTC）以来的秒数
  // NULL 参数表示不需要额外的输出参数
  time_t t = time(NULL);
  // localtime() 函数：将 Unix 时间戳转换为本地时间的分解表示
  struct tm *tm = localtime(&t);
  rtc->second = tm->tm_sec;
  rtc->minute = tm->tm_min;
  rtc->hour   = tm->tm_hour;
  rtc->day    = tm->tm_mday;
  rtc->month  = tm->tm_mon + 1; // tm_mon 表示月份，范围 0-11，需要加 1 转换为 1-12
  rtc->year   = tm->tm_year + 1900; // tm_year 表示年份，从 1900 年开始计算
}

// 计算系统自启动以来运行的时间（以微秒为单位）
// 高精度时间测量（微秒级）,用于性能分析、游戏计时等
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  struct timeval now;
  // &now - 输出参数，存储当前时间，NULL - 时区参数（已废弃，通常传 NULL）
  // 获取自 Unix 纪元（1970-01-01 00:00:00 UTC）以来的时间，包含秒和微秒两部分
  gettimeofday(&now, NULL);
  long seconds = now.tv_sec - boot_time.tv_sec;
  long useconds = now.tv_usec - boot_time.tv_usec;
  // 加500us可能是为了补偿测量误差
  uptime->us = seconds * 1000000 + (useconds + 500);
}

void __am_timer_init() {
  gettimeofday(&boot_time, NULL);
}
