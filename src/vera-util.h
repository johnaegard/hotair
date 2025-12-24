#ifndef VERAUTIL_H
#define VERAUTIL_H

// Function declarations for VERA utility functions
void uppercase_petscii_40x30(void);
void load_into_vera(char* filename, unsigned long base_addr, char secondary_address);
void vera_loads(void);
void vera_screen_setup(void);

#endif // VERAUTIL_H
