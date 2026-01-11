#include <cstring>
#include <string>
#include <stdio.h>

namespace gpio {
namespace util {
    int gpio_export(unsigned int gpio);
    int gpio_unexport(unsigned int gpio);
    int gpio_set_dir(unsigned int gpio, std::string dirStatus);
    int gpio_set_value(unsigned int gpio, int value);

}
};
