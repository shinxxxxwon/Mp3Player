#include "stdafx.h"
#include "LameHelper.h"

settings_t::settings_t()
{
	//Setting the default values
	title = "";
	artist = "";
	album = "";
	comment = "";
	year = "";
	track = "";
	genre = "";
	albumart = NULL;
	
	channels = EC_STEREO;
	abr_bitrate = BR_128kbps;
	cbr_bitrate = BR_128kbps;
	quality = 5;
	enc_mode = EM_CBR;
	resample_frequency = SR_44khz; 
	in_samplerate = SR_44khz;
}


CLameHelper::CLameHelper()
{
	//Initialize to NULL, aids deletion/closing later
	for(int i = 0; i < MAX_THREAD_COUNT; i++)
	{
		m_hThread[i] = NULL;
		m_phSParam[i] = NULL;
	}
	
}

CLameHelper::~CLameHelper()
{
	//Destroy all declared objects
	for(int i = 0; i < MAX_THREAD_COUNT; i++)
	{
		if(m_hThread[i] != NULL)
			CloseHandle(m_hThread[i]);

		if(m_phSParam[i] != NULL)
			delete m_phSParam[i];
	}
}

//*****************************DECODEING*************************************************************
//****************************************************************************************************
void CLameHelper::Write16BitLowHigh(FILE * fp, int val)
{
    unsigned char bytes[2];
    bytes[0] = (val & 0xff);
    bytes[1] = ((val >> 8) & 0xff);
    fwrite(bytes, 1, 2, fp);
}

void CLameHelper::Write32BitLowHigh(FILE * fp, int val)
{
    unsigned char bytes[4];
    bytes[0] = (val & 0xff);
    bytes[1] = ((val >> 8) & 0xff);
    bytes[2] = ((val >> 16) & 0xff);
    bytes[3] = ((val >> 24) & 0xff);
    fwrite(bytes, 1, 4, fp);
}

void CLameHelper::WriteWaveHeader(FILE * const fp, int pcmbytes, int freq, int channels, int bits)
{
    int     bytes = (bits + 7) / 8;
    /* quick and dirty, but documented */
    fwrite("RIFF", 1, 4, fp); /* label */
    Write32BitLowHigh(fp, pcmbytes + 44 - 8); /* length in bytes without header */
    fwrite("WAVEfmt ", 2, 4, fp); /* 2 labels */
    Write32BitLowHigh(fp, 2 + 2 + 4 + 4 + 2 + 2); /* length of PCM format declaration area */
    Write16BitLowHigh(fp, 1); /* is PCM? */
    Write16BitLowHigh(fp, channels); /* number of channels */
    Write32BitLowHigh(fp, freq); /* sample frequency in [Hz] */
    Write32BitLowHigh(fp, freq * channels * bytes); /* bytes per second */
    Write16BitLowHigh(fp, channels * bytes); /* bytes per sample time */
    Write16BitLowHigh(fp, bits); /* bits per sample */
    fwrite("data", 1, 4, fp); /* label */
    Write32BitLowHigh(fp, pcmbytes); /* length in bytes of raw PCM data */
}

int CLameHelper::Decode(char* szMp3_in, char* szPcm_out)
{
	return Decode(szMp3_in, szPcm_out, NULL);
}

//the static function used for the thread
int CLameHelper::Decode_s(void* param)
{
	StaticParam_t* sp = (StaticParam_t*)param;
	char* szPcm_out = sp->pcm;
	char* szMp3_in = sp->mp3;
	WNDPROC callback_proc = sp->callback_proc;

	CLameHelper* lh = (CLameHelper*)sp->lhObj;
	return lh->Decode(szMp3_in, szPcm_out, callback_proc);
}


int CLameHelper::Decode(char* szMp3_in, char* szPcm_out, WNDPROC callback_proc)
{
	int read, i, samples;
	long m_lWavSize = 0; // use to count the number of mp3 byte read, this is used to write the length of the wave file
	long cumulative_read = 0;

	short int pcm_l[PCM_SIZE], pcm_r[PCM_SIZE];
	unsigned char mp3_buffer[MP3_SIZE];

	FILE* mp3 = fopen(szMp3_in, "rb");
	if(mp3 == NULL)
	{
		if(callback_proc != NULL)
		{
			callback_proc((HWND)GetModuleHandle(NULL), LH_ERROR, -1, NULL);
		}
		sprintf(errMsg, "FATAL ERROR: file '%s' can't be open for read. Aborting!\n", szMp3_in);
		errorHandler(errMsg);
		return -1;
	}
	fseek(mp3, 0, SEEK_END);
    long MP3_total_size = ftell(mp3);
    fseek(mp3, 0, SEEK_SET);

	
	FILE* pcm = fopen(szPcm_out, "wb");
	if(pcm == NULL)
	{
		if(callback_proc != NULL)
		{
			callback_proc((HWND)GetModuleHandle(NULL), LH_ERROR, -1, NULL);
		}
		sprintf(errMsg, "FATAL ERROR: file '%s' can't be open for write. Aborting!\n", szPcm_out);
		errorHandler(errMsg);
		return -1;
	}

	
	lame_t lame = lame_init();
	lame_set_decode_only(lame, 1);
	if(lame_init_params(lame) == -1)
	{
		if(callback_proc != NULL)
		{
			callback_proc((HWND)GetModuleHandle(NULL), LH_ERROR, -2, NULL);
		}
		sprintf(errMsg, "FATAL ERROR: parameters failed to initialize properly in lame. Aborting!\n", szPcm_out);
		errorHandler(errMsg);
		return -2;
	}

	hip_t hip = hip_decode_init();
	
	mp3data_struct mp3data;
	memset(&mp3data, 0, sizeof(mp3data));
	
	int nChannels = -1;
	int nSampleRate = -1;
	int mp3_len;

	if(callback_proc != NULL)
	{
		callback_proc((HWND)GetModuleHandle(NULL), LH_STARTED, NULL, NULL);
	}

	while((read = fread(mp3_buffer, sizeof(char), MP3_SIZE, mp3)) > 0)
	{
		mp3_len = read;
		cumulative_read += read * sizeof(char);
		do
		{
			samples = hip_decode1_headers(hip, mp3_buffer, mp3_len, pcm_l, pcm_r, &mp3data);
			m_lWavSize += samples;

			if(mp3data.header_parsed == 1)//header is gotten
			{
				if(nChannels < 0)//reading for the first time
				{
					//Write the header
					WriteWaveHeader(pcm, 0x7FFFFFFF, mp3data.samplerate, mp3data.stereo, 32); //unknown size, so write maximum 32 bit signed value
				}
				nChannels = mp3data.stereo;
				nSampleRate = mp3data.samplerate;
			}

			if(samples > 0 && mp3data.header_parsed != 1)
			{
				errorHandler("WARNING: lame decode error occured!");
				break;
			}

			if(samples > 0)
			{
				for(i = 0 ; i < samples; i++)
				{
					fwrite((char*)&pcm_l[i], sizeof(char), sizeof(pcm_l[i]), pcm);
					if(nChannels == 2)
					{
						fwrite((char*)&pcm_r[i], sizeof(char), sizeof(pcm_r[i]), pcm);
					}
				}
			}
			mp3_len = 0;

			if(callback_proc != NULL)
			{
				int percentage = (cumulative_read/MP3_total_size)*100;
				callback_proc((HWND)GetModuleHandle(NULL), LH_COMPUTED, percentage, NULL);
			}
		}while(samples>0);
	}

	i = (16 / 8) * mp3data.stereo;
    if (m_lWavSize <= 0) 
	{
       m_lWavSize = 0;
    }
    else if (m_lWavSize > 0xFFFFFFD0 / i) 
	{
        m_lWavSize = 0xFFFFFFD0;
    }
    else 
	{
        m_lWavSize *= i;
    }
	
    if(!fseek(pcm, 0, SEEK_SET))//seek back and adjust length
        WriteWaveHeader(pcm, (int) m_lWavSize, mp3data.samplerate, mp3data.stereo, 16);
	else
		errorHandler("WARNING: can't seek back to adjust length in wave header!");

	hip_decode_exit(hip);
	lame_close(lame);
	fclose(mp3);
	fclose(pcm);

	if(callback_proc != NULL)
	{
		callback_proc((HWND)GetModuleHandle(NULL), LH_DONE, NULL, NULL);
	}
	return 0;
}

void CLameHelper::errorHandler(char* msg)
{
	printf("%s\n", msg);
}
