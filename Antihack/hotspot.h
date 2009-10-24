#ifndef INC_HOTSPOT
#define INC_HOTSPOT

struct HOTSPOT {
	explicit HOTSPOT(char* a, unsigned int s, char* d) : address(a), size(s), real_data(d) {}

	char* address;
	unsigned int size;
	char* real_data;

	private:
		HOTSPOT() {} // must be created with correct parameters
};

#endif
