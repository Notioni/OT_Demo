#ifndef __IRDA_H
#define __IRDA_H

void light_ir(int enable);
int irda_loopback_test(void);
int irda_send_code(void);
int irda_study_code(void);
int irda_init(void);

#endif /* __IRDA_H */
