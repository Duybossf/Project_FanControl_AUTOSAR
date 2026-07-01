
#include "IoHwAb.h"

// Hàm delay đơn giản (không dùng timer, không chính xác)
void Delay(uint32 nCount) {
    for(; nCount != 0; nCount--);
}

int main(void) {

    /* Gán nhiệt độ mặc định khi chưa đọc ADC*/
    uint8 nhietDo = 0U;
    
    /* Cấu hình Port, Dio, Adc, Pwm*/
    IoHwAb_Init();

    while (1) 
    {
        if (IoHwAb_ReadTemperature(&nhietDo) == E_OK)
        {
            if (nhietDo < 30U)
            {
                /* Tắt quạt*/
                IoHwAb_SetFanDuty(0U);

                /* tắt LED*/
                IoHwAb_SetLed(FALSE);
            }
            else if (nhietDo < 40U)
            {
                /* Bật quạt 50%*/
                IoHwAb_SetFanDuty(50U);

                /* Bật LED*/
                IoHwAb_SetLed(TRUE);
            }
            else
            {
                /* Bật quạt 100%*/
                IoHwAb_SetFanDuty(100U);

                /* Bật LED*/
                IoHwAb_SetLed(TRUE);
            }

            Delay(500U);
        }
    }
}
