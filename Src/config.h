#ifndef _CONFIG_H
#define _CONFIG_H

#define R2          33000
#define R4          10000
#define R3          33000
#define R5          560
#define R_SH        0.03333333

#define V_REF       1.077
#define V_SHOTTKY   0.20 // ������� ���������� �� ����� ������

// ����������� �������� ������� � BRIGHT_UHI �� BRIGHT_HI
// ��� (TEMP_OFF_DEF_SET + 10) - ����������
// �������� �� ��������� (�� ����������)
#define TEMP_OFF_DEF_SET 65

#define SYS_CLK     2000000L // 1000000L

#endif /* _CONFIG_H */
