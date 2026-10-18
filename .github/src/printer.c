/*
  Hatari - printer.c

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.

  Printer communication. When bytes are sent from the ST they are sent to these
  functions via 'Printer_TransferByteTo()'. This will then open a file and
  direct the output to this. These bytes are buffered up (to improve speed) and
  this also allow us to detect when the stream goes into idle - at which point
  we close the file/printer.

  The data can go to a raw .prn file and/or (see printer_pdf.c) into a PDF
  file that shows what the emulated dot matrix printer would have printed.
*/
const char Printer_fileid[] = "Hatari printer.c";

#include "main.h"
#include "configuration.h"
#include "file.h"
#include "paths.h"
#include "printer.h"
#include "printer_pdf.h"
#include "log.h"

#define PRINTER_DEBUG 0
#if PRINTER_DEBUG
#define Dprintf(a) printf a
#else
#define Dprintf(a)
#endif

/* After ~4 seconds (4*50 VBLs), flush & close printer */
#define PRINTER_IDLE_CLOSE   (4*50)

static int nIdleCount;
static int bUnflushed;

static FILE *pPrinterHandle;


/*-----------------------------------------------------------------------*/
/**
 * Initialise Printer
 */
void Printer_Init(void)
{
	char *separator;
	Dprintf((stderr, "Printer_Init()\n"));

	/* disabled from config/command line? */
	if (!ConfigureParams.Printer.bEnablePrinting)
		return;

	/* raw .prn file output */
	if (ConfigureParams.Printer.bPrintToFile &&
	    ConfigureParams.Printer.szPrintToFileName[0])
	{
		/* printer name without path? */
		separator = strrchr(ConfigureParams.Printer.szPrintToFileName, PATHSEP);
		if (separator)
		{
			*separator = '\0';
			if (!File_DirExists(ConfigureParams.Printer.szPrintToFileName)) {
				Log_AlertDlg(LOG_ERROR, "Printer output file directory inaccessible. Printing to file disabled.");
				ConfigureParams.Printer.bPrintToFile = false;
			}
			*separator = PATHSEP;
		}
	}

	/* PDF output */
	if (ConfigureParams.Printer.bPrintToPdf && !PrinterPdf_CheckOutputDir())
	{
		Log_AlertDlg(LOG_ERROR, "PDF output directory inaccessible. Printing to PDF disabled.");
		ConfigureParams.Printer.bPrintToPdf = false;
	}

	if (!ConfigureParams.Printer.bPrintToFile && !ConfigureParams.Printer.bPrintToPdf)
		ConfigureParams.Printer.bEnablePrinting = false;

	Dprintf((stderr, "Filename for printing: %s \n", ConfigureParams.Printer.szPrintToFileName));
}


/*-----------------------------------------------------------------------*/
/**
 * Uninitialise Printer
 */
void Printer_UnInit(void)
{
	Dprintf((stderr, "Printer_UnInit()\n"));

	/* Close any open files, finish the PDF of the current print job */
	pPrinterHandle = File_Close(pPrinterHandle);
	PrinterPdf_Close();
	bUnflushed = false;
	nIdleCount = 0;
}


/*-----------------------------------------------------------------------*/
/**
 * Pass byte from emulator to printer.  Opens the printer file appending
 * if it isn't already open. Returns false if connection to "printer"
 * failed
 */
bool Printer_TransferByteTo(uint8_t Byte)
{
	bool bAccepted = false;

	/* Do we want to output to a printer/file? */
	if (!ConfigureParams.Printer.bEnablePrinting)
		return false;   /* Failed if printing disabled */

	/* Raw output into .prn file */
	if (ConfigureParams.Printer.bPrintToFile)
	{
		/* Have we made a connection to our printer/file? */
		if (!pPrinterHandle)
		{
			/* open printer file... */
			pPrinterHandle = File_Open(ConfigureParams.Printer.szPrintToFileName, "a+b");
			if (!pPrinterHandle)
			{
				Log_AlertDlg(LOG_ERROR, "Printer output file open failed. Printing to file disabled.");
				ConfigureParams.Printer.bPrintToFile = false;
			}
		}
		if (pPrinterHandle)
		{
			if (fputc(Byte, pPrinterHandle) != Byte)
				Log_Printf(LOG_ERROR, "Printer_TransferByteTo() writing failed!\n");
			else
				bAccepted = true;
		}
	}

	/* Rendering into a PDF file */
	if (ConfigureParams.Printer.bPrintToPdf)
	{
		if (!PrinterPdf_IsOpen() && !PrinterPdf_Open())
		{
			Log_AlertDlg(LOG_ERROR, "PDF output file open failed. Printing to PDF disabled.");
			ConfigureParams.Printer.bPrintToPdf = false;
		}
		else
		{
			PrinterPdf_Byte(Byte);
			bAccepted = true;
		}
	}

	if (!ConfigureParams.Printer.bPrintToFile && !ConfigureParams.Printer.bPrintToPdf)
		ConfigureParams.Printer.bEnablePrinting = false;

	if (bAccepted)
		bUnflushed = true;
	return bAccepted;
}


/*-----------------------------------------------------------------------*/
/**
 * Empty printer buffer, and if remains idle for set time close connection
 * (ie close file, stop printer)
 */
void Printer_CheckIdleStatus(void)
{
	/* Is anything waiting for printer? */
	if (bUnflushed)
	{
		if (pPrinterHandle)
			fflush(pPrinterHandle);
		bUnflushed = false;
		nIdleCount = 0;
	}
	else
	{
		nIdleCount++;
		/* Has printer been idle? */
		if (nIdleCount >= PRINTER_IDLE_CLOSE)
		{
			/* Close printer output */
			Printer_UnInit();
		}
	}
}
