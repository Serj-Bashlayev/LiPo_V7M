#ifndef _CONFIG_H
#define _CONFIG_H

#define R2          33000
#define R4          10000
#define R3          33000
#define R5          560
#define R_SH        0.025

#define V_REF       1.077
#define V_SHOTTKY   0.20 // падение напр€жени€ на диоде Ўоттки

// температура снижени€ €ркости с BRIGHT_UHI до BRIGHT_HI
// при (TEMP_OFF_DEF_SET + 10) - отключение
// значение по умолчанию (до калибровки)
#define TEMP_OFF_DEF_SET 65

#endif /* _CONFIG_H */
