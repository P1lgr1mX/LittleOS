struct example {
    unsigned char config;  
    unsigned short address : 16; 
    unsigned char index;     /* bit 24 - 31 */
} __attribute__((packed));
