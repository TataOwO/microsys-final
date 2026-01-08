#include "gpio_LED_CTRL.hpp"

namespace gpio {

LED_CTRL::LED_CTRL(int gpio) {
	gpio_id = gpio;
}

LED_CTRL::~LED_CTRL() {}

void LED_CTRL::enable() {
	util::gpio_export(gpio_id);
	util::gpio_set_dir(gpio_id, "out");
	util::gpio_set_value(gpio_id, 1);
}

void LED_CTRL::disable() {
	util::gpio_set_value(gpio_id, 0);
	util::gpio_unexport(gpio_id);
}

};
