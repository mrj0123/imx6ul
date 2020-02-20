#ifndef BASE64_H
#define BASE64_H

int base64_decode(const char * base64, unsigned char * bindata);
char *base64_encode(unsigned char *bindata, char * base64, int binlength);

#endif // BASE64_H
