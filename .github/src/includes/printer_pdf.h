/*
  Hatari - printer_pdf.h

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.

  Glue between Hatari's printer port and the "pe" printer emulation core,
  which renders the printer data stream to PDF files.
*/
#ifndef HATARI_PRINTER_PDF_H
#define HATARI_PRINTER_PDF_H

/* selectable printer models and paper types (for the GUI) */
extern int PrinterPdf_NumModels(void);
extern const char *PrinterPdf_ModelId(int idx);
extern const char *PrinterPdf_ModelName(int idx);
extern int PrinterPdf_FindModel(const char *id);   /* index, 0 if unknown */
extern int PrinterPdf_NumPapers(void);
extern const char *PrinterPdf_PaperName(int idx);

/* configured output directory usable? */
extern bool PrinterPdf_CheckOutputDir(void);

/* one print job = one time stamped PDF file */
extern bool PrinterPdf_IsOpen(void);
extern bool PrinterPdf_Open(void);
extern void PrinterPdf_Byte(uint8_t Byte);
extern void PrinterPdf_Close(void);

#endif
