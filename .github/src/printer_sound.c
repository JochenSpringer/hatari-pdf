/*
  Hatari - printer_sound.c

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.

  Needle printer noise.  A short looped recording (printer_sound_data.c) is
  played on a second SDL audio device while data arrives for the PDF printer,
  so the sound never disturbs the emulated YM2149 / DMA audio.
*/
const char PrinterSound_fileid[] = "Hatari printer_sound.c";

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "configuration.h"
#include "printer_pdf.h"

extern const int16_t PrinterSound_Data[];
extern const unsigned PrinterSound_Samples;

static SDL_AudioDeviceID Dev;
static unsigned Pos;
static bool bPlaying, bFailed;

static void SDLCALL Callback(void *userdata, Uint8 *stream, int len)
{
	int16_t *out = (int16_t *)stream;
	int i, n = len / 2;
	(void)userdata;
	for (i = 0; i < n; i++)
	{
		out[i] = PrinterSound_Data[Pos];
		if (++Pos >= PrinterSound_Samples)
			Pos = 0;
	}
}

void PrinterSound_Start(void)
{
	if (bPlaying || bFailed || !ConfigureParams.Printer.bPrinterSound)
		return;
	if (!Dev)
	{
		SDL_AudioSpec want;
		if (!SDL_WasInit(SDL_INIT_AUDIO) && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
		{
			bFailed = true;
			return;
		}
		SDL_zero(want);
		want.freq = 16000;
		want.format = AUDIO_S16SYS;
		want.channels = 1;
		want.samples = 1024;
		want.callback = Callback;
		Dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
		if (!Dev)
		{
			bFailed = true;
			return;
		}
	}
	SDL_PauseAudioDevice(Dev, 0);
	bPlaying = true;
}

void PrinterSound_Stop(void)
{
	if (Dev && bPlaying)
		SDL_PauseAudioDevice(Dev, 1);
	bPlaying = false;
}

void PrinterSound_Close(void)
{
	PrinterSound_Stop();
	if (Dev)
		SDL_CloseAudioDevice(Dev);
	Dev = 0;
}
