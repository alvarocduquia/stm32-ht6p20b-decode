#include <stm32f4xx.h>

typedef struct {

	uint32_t endereco;
	char botoes[2];

} CONTROLE_TypeDef;

char piloto = 0x0;
int lambda;
int contador;
uint32_t buffer;

typedef enum { SUBIDA, DESCIDA } BORDA_TypeDef;
int ultima_borda = 0;

int tick = 0;

#define REGISTRADOR		GPIOC
#define PINO			GPIO_ODR_ODR_1

void SysTick_Handler() {

	tick++;

}

void EXTI1_IRQHandler() {

	EXTI->PR |= EXTI_PR_PR1;

	BORDA_TypeDef borda = (REGISTRADOR->IDR & PINO) ? SUBIDA : DESCIDA;
	int duracao = (tick - ultima_borda) * 10;

	ultima_borda = tick;

	if(!piloto) {
		if(borda == SUBIDA) {
			if((duracao > 9200) && (duracao < 13800)) {
				piloto = 0x1;

				lambda = duracao / 23;

				contador = 0;
				buffer = 0;
			}
		}
	} else if(piloto) {
		if(borda == DESCIDA) {
			if((duracao > (0.5 * lambda)) && duracao < (1.5 * lambda)) {
				if(contador > 0) {
					buffer = (buffer << 1) + 1;
				}

				contador++;
			} else if((duracao > (1.5 * lambda)) && (duracao < (2.5 * lambda))) {
				if(contador > 0) {
					buffer = (buffer << 1);
				}

				contador++;
			} else {
				piloto = 0x0;
			}

			if(contador == 29) {
				if((buffer & 0xF) == 0b0101) {
					CONTROLE_TypeDef controle;

					controle.endereco = buffer >> 6;
					controle.botoes[0] = (buffer >> 4) & 0x1;
					controle.botoes[1] = (buffer >> 5) & 0x1;

					// Fim!
				}

				piloto = 0x0;
			}
		}
	}

}

int main(void) {

	SysTick_Config(160);	// 10uS (Frequência do ARM em 16MHz)

	// GPIO

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	GPIOC->MODER &= ~(GPIO_MODER_MODER10);

	// EXTI

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PC;

	EXTI->IMR |= EXTI_IMR_MR1;

	EXTI->RTSR |= EXTI_RTSR_TR1;
	EXTI->FTSR |= EXTI_FTSR_TR1;

	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_SetPriority(EXTI1_IRQn, 1);

	while(1) {

	}

}
