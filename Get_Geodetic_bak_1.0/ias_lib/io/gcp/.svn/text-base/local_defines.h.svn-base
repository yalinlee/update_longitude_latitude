#ifndef LOCAL_DEFINES_H
#define LOCAL_DEFINES_H

/* Defining some macro routines to work with the data buffers regarding
   endianness.
   For GCPs, the chips will always be little endian format on disk, so need to
   byte-swap the data buffer read and for writing if on a big endian system. */

#ifndef HAVE_LITTLE_ENDIAN
#define SWAP_BYTES_16(x)                                \
{                                                       \
    unsigned short data_macro = *(unsigned short*)&(x); \
    data_macro = (((data_macro & 0xff00) >> 8)          \
               |  ((data_macro & 0x00ff) << 8));        \
    *(unsigned short*)&(x) = data_macro;                \
}
#define SWAP_BYTES_32(x)                                \
{                                                       \
    unsigned int data_macro = *(unsigned int*)&(x);     \
    data_macro = (((data_macro & 0xff000000) >> 24)     \
               |  ((data_macro & 0x00ff0000) >> 8)      \
               |  ((data_macro & 0x0000ff00) << 8)      \
               |  ((data_macro & 0x000000ff) << 24));   \
    *(unsigned int*)&(x) = data_macro;                  \
}
#else
#define SWAP_BYTES_16(x)
#define SWAP_BYTES_32(x)
#endif

#endif
