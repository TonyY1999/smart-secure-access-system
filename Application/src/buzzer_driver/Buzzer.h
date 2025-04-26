/**************************************************************************//**
 * @file      buzzer.h
 * @brief     alarming system buzzer driver
 * @author    Tony Yan & Yue Zhang
 * @date      2025-04-18
 ******************************************************************************/


extern struct tcc_module tcc_instance_tcc0;
void configure_tcc(void);
void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
