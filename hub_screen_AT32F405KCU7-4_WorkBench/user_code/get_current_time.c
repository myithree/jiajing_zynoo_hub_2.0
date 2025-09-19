#include "get_current_time.h"

//提供微秒级时间戳
uint32_t getCurrentMicros(void)
{
  uint32_t primask = __get_PRIMASK();
  __disable_irq();

  uint32_t m = wk_timebase_get();
  __IO uint32_t v = SysTick->VAL;
  // If an overflow happened since we disabled irqs, it cannot have been
  // processed yet, so increment m and reload VAL to ensure we get the
  // post-overflow value.
  if (SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) {
    ++m;
    v = SysTick->VAL;

  }

  // Restore irq status
  __set_PRIMASK(primask);

  const uint32_t tms = SysTick->LOAD + 1;
  return (m * 1000 + ((tms - v) * 1000) / tms);
}

