#ifndef PE_PDF_H
#define PE_PDF_H
#include <stddef.h>
#include <stdint.h>
struct pe_pdf;
struct pe_pdf *pdf_create(const char *path, int compress);
/* w100/h100: page size in 1/100 pt.  The file is created on first page and
 * is a complete, valid PDF after every call. */
int pdf_add_page(struct pe_pdf *p, int32_t w100, int32_t h100,
                 const uint8_t *content, size_t len);
void pdf_close(struct pe_pdf *p);
void pdf_fmt100(char *out, int32_t v); /* 1/100 -> "12.34" */
#endif
