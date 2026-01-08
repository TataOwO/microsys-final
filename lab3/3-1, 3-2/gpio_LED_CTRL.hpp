#include "gpio_util.hpp"
#include <memory>

namespace gpio {

class LED_CTRL {
public:
	LED_CTRL(int gpio);

	~LED_CTRL();
	
	void enable();
	void disable();
private:
    int gpio_id = -1;
};

};
