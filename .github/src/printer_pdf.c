/*
  Hatari - printer_pdf.c

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.

  Renders the data stream sent to the emulated printer port as PDF, using
  the emulation of a dot matrix printer (currently NEC Pinwriter P6).
  Every print job (ended by the printer idle timeout, see printer.c) creates
  a new file "<base name>_YYYYMMDD_HHMMSS.pdf".
*/
const char PrinterPdf_fileid[] = "Hatari printer_pdf.c";

#include <string.h>
#include "main.h"
#include "configuration.h"
#include "file.h"
#include "log.h"
#include "str.h"
#include "printer_pdf.h"
#include "pe/driver.h"
#include "pe/pe.h"

static pe_job *pPdfJob;
static char sPdfPath[FILENAME_MAX + 64];

static const struct { const char *name; pe_paper paper; } Papers[] =
{
	{ "A4 single sheet",      PE_PAPER_A4 },
	{ "Fanfold 12\"x240 mm",  PE_PAPER_A4_ENDLOS },
};
#define NUM_PAPERS ((int)(sizeof(Papers) / sizeof(Papers[0])))


int PrinterPdf_NumModels(void)
{
	int n = 0;
	while (pe_drivers()[n])
		n++;
	return n;
}

const char *PrinterPdf_ModelId(int idx)
{
	return pe_drivers()[idx]->id;
}

const char *PrinterPdf_ModelName(int idx)
{
	return pe_drivers()[idx]->name;
}

int PrinterPdf_FindModel(const char *id)
{
	int i;
	for (i = 0; pe_drivers()[i]; i++)
	{
		if (strcmp(pe_drivers()[i]->id, id) == 0)
			return i;
	}
	return 0;
}

int PrinterPdf_NumPapers(void)
{
	return NUM_PAPERS;
}

const char *PrinterPdf_PaperName(int idx)
{
	return Papers[idx].name;
}


/**
 * Split the configured PDF name into directory ("" = current directory) and
 * base name without ".pdf" extension.
 */
static void SplitName(char *dir, char *base, size_t size)
{
	const char *name = ConfigureParams.Printer.szPdfFileName;
	const char *sep = strrchr(name, PATHSEP);
	const char *alt = strrchr(name, '/');
	char *dot;

	if (alt > sep)
		sep = alt;
	if (sep)
	{
		size_t len = sep - name;
		if (len >= size)
			len = size - 1;
		memcpy(dir, name, len);
		dir[len] = '\0';
		name = sep + 1;
	}
	else
	{
		dir[0] = '\0';
	}
	Str_Copy(base, name[0] ? name : "hatari", size);
	dot = strrchr(base, '.');
	if (dot && dot != base)
		*dot = '\0';
}


bool PrinterPdf_CheckOutputDir(void)
{
	char dir[FILENAME_MAX], base[FILENAME_MAX];

	SplitName(dir, base, sizeof(dir));
	return dir[0] == '\0' || File_DirExists(dir);
}


bool PrinterPdf_IsOpen(void)
{
	return pPdfJob != NULL;
}


bool PrinterPdf_Open(void)
{
	char dir[FILENAME_MAX], base[FILENAME_MAX];
	const pe_driver *drv;
	pe_config cfg;
	int paper = ConfigureParams.Printer.nPaperType;

	if (pPdfJob)
		return true;

	drv = pe_driver_find(ConfigureParams.Printer.szPrinterModel);
	if (!drv)
		drv = pe_drivers()[0];
	if (paper < 0 || paper >= NUM_PAPERS)
		paper = 0;
	pe_config_init(&cfg, Papers[paper].paper);

	SplitName(dir, base, sizeof(dir));
	if (!pe_timestamp_path(sPdfPath, sizeof(sPdfPath), dir, base))
		return false;
	pPdfJob = pe_job_create(drv, &cfg, sPdfPath);
	if (pPdfJob)
		Log_Printf(LOG_INFO, "Printer: writing PDF to %s\n", sPdfPath);
	return pPdfJob != NULL;
}


void PrinterPdf_Byte(uint8_t Byte)
{
	if (pPdfJob)
		pe_job_write(pPdfJob, &Byte, 1);
}


void PrinterPdf_Close(void)
{
	int pages;

	if (!pPdfJob)
		return;
	pages = pe_job_finish(pPdfJob);
	pPdfJob = NULL;
	if (pages > 0)
		Log_Printf(LOG_INFO, "Printer: %d page(s) written to %s\n", pages, sPdfPath);
}
