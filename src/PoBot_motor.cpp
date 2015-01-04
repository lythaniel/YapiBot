#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>

#define MAIN_TEXTURE_WIDTH 512
#define MAIN_TEXTURE_HEIGHT 512

#define PWM_GPIO_R_PWM 23
#define PWM_GPIO_R_FW 24
#define	PWM_GPIO_R_RE 25

#define PWM_GPIO_L_PWM 14
#define PWM_GPIO_L_FW 15
#define	PWM_GPIO_L_RE 18


//entry point
int main(int argc, const char **argv)
{
	gpioInitialise();
	gpioSetMode (PWM_GPIO_R_PWM,PI_OUTPUT);
	gpioSetMode (PWM_GPIO_R_FW,PI_OUTPUT);
	gpioSetMode (PWM_GPIO_R_RE,PI_OUTPUT);

	gpioSetMode (PWM_GPIO_L_PWM,PI_OUTPUT);
	gpioSetMode (PWM_GPIO_L_FW,PI_OUTPUT);
	gpioSetMode (PWM_GPIO_L_RE,PI_OUTPUT);

	gpioWrite(PWM_GPIO_R_FW, 0);
	gpioWrite(PWM_GPIO_R_RE, 1);
	gpioSetPWMrange (PWM_GPIO_R_PWM,100);
	gpioPWM (PWM_GPIO_R_PWM,100);

	gpioWrite(PWM_GPIO_L_FW, 0);
	gpioWrite(PWM_GPIO_L_RE, 1);
	gpioSetPWMrange (PWM_GPIO_L_PWM,100);
	gpioPWM (PWM_GPIO_L_PWM,100);


	//while (1);
	char c;
	c = getchar();
	gpioWrite(PWM_GPIO_R_FW, 0);
	gpioWrite(PWM_GPIO_R_RE, 0);
	gpioWrite(PWM_GPIO_L_FW, 0);
	gpioWrite(PWM_GPIO_L_RE, 0);
	gpioPWM (PWM_GPIO_R_PWM,100);
	gpioPWM (PWM_GPIO_L_PWM,100);
	
	gpioTerminate();
}
