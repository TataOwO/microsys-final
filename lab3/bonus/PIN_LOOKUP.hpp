/** USAGE:
*   LOOKUP::PIN::P15 -> 255
*   LOOKUP::PIN::P18 -> 481
*   etc etc etc
*/

namespace LOOKUP {
	struct PIN {
		enum : int {
			P7 = 396,
			P13 = 397,
			P15 = 255,
			P16 = 296,
			P18 = 481,
			P22 = 254,
			P29 = 398,
			P31 = 298,
			P32 = 297,
			P33 = 389,
			P37 = 388,
		};
	};
}