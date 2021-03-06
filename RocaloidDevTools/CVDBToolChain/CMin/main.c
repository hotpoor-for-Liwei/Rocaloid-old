#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>

#include "../../../RUtil/RUtil.h"
#include "../../../RUtil/IO/FileUtil.h"
#include "../../../CVEDSP/DSPBase/Filter.h"
#include "../../../CVEDSP/DSPBase/Spectrum.h"
#include "../../../CVEDSP/DSPBase/ControlPointFilter.h"
#include "../../../CVEDSP/DSPBase/LPC.h"
#include "../../../CVEDSP/Algorithm/BaseFreq.h"
#include "../../../CVEDSP/Algorithm/PSOLA.h"
#include "../../../CVEDSP/Algorithm/Formant.h"
#include "../../../CVEDSP/IntrinUtil/Calculation.h"

#include "../../../RocaloidEngine/RFILE3/CVDB3/CVDB3IO.h"

//$ CMin FileName [-C / -V] [-F0] [-F1] [-F2] [-F3] [-L] [-o OutputDir]

void GetFileName(String* Dest, String* Path)
{
    int i;
    int End, Start;
    for(i = Path -> Data_Index; i > 0; i --)
        if(Path -> Data[i] == '.')
            break;
    End = i;
    for(; i > 0; i --)
        if(Path -> Data[i] == '/')
            break;
    Start = i;
    Mid(Dest, Path, Start + 1, End - Start - 1);
}

int GetMaxIndex(float* Dest, int Length)
{
    int i, record;
    float Max = - 9999;
    record = 0;
    for(i = 0; i < Length; i ++)
        if(Dest[i] > Max)
        {
            Max = Dest[i];
            record = i;
        }
    return record;
}

int Arg_LenAvaliable = 0;
float Arg_Len = 0;
int Arg_F0Avaliable = 0;
float Arg_F0 = 0;
int Arg_F1Avaliable = 0;
float Arg_F1 = 0;
int Arg_F2Avaliable = 0;
float Arg_F2 = 0;
int Arg_F3Avaliable = 0;
float Arg_F3 = 0;
int Arg_CVAvaliable = 0;
int Arg_C = 1;
int Arg_NextOutput = 0;
int main(int ArgQ, char** ArgList)
{
    int i;
    String_FromChars(OutputPath, ".");
    String_FromChars(Path, ArgList[1]);
    ArrayType_Ctor(String, ArgStrList);
    for(i = 0; i < ArgQ; i ++)
    {
        ArrayType_PushNull(String, ArgStrList);
        String_Ctor(ArgStrList + i);
        String_SetChars(ArgStrList + i, ArgList[i]);
        if(String_GetChar(ArgStrList + i, 0) == '-')
            UpperCase(ArgStrList + i, ArgStrList +i);
    }
    for(i = 1; i < ArgQ; i ++)
    {
        if(String_EqualChars(ArgStrList + i, "-L"))
        {
            Arg_LenAvaliable = 1;
            Arg_Len = CFloatStr(ArgStrList + i + 1);
            i ++;
        }else if(String_EqualChars(ArgStrList + i, "-F0"))
        {
            Arg_F0Avaliable = 1;
            Arg_F0 = CFloatStr(ArgStrList + i + 1);
            i ++;
        }else if(String_EqualChars(ArgStrList + i, "-F1"))
        {
            Arg_F1Avaliable = 1;
            Arg_F1 = CFloatStr(ArgStrList + i + 1);
            i ++;
        }else if(String_EqualChars(ArgStrList + i, "-F2"))
        {
            Arg_F2Avaliable = 1;
            Arg_F2 = CFloatStr(ArgStrList + i + 1);
            i ++;
        }else if(String_EqualChars(ArgStrList + i, "-F3"))
        {
            Arg_F3Avaliable = 1;
            Arg_F3 = CFloatStr(ArgStrList + i + 1);
            i ++;
        }else if(String_EqualChars(ArgStrList + i, "-C"))
        {
            Arg_CVAvaliable = 1;
            Arg_C = 1;
        }else if(String_EqualChars(ArgStrList + i, "-V"))
        {
            Arg_CVAvaliable = 1;
            Arg_C = 0;
        }else if(String_EqualChars(ArgStrList + i, "-O"))
        {
            Arg_NextOutput = 1;
        }else if(Arg_NextOutput == 1)
        {
            if(OutputPath.Data_Index > 0)
            {
                printf("Error: Multiple output directories specified.\n");
                return 1;
            }
            String_Copy(& OutputPath, ArgStrList + i);
            Arg_NextOutput = 0;
        }
    }

    printf("CVDBStudioMin\n");
    printf("------------------\n");
    SetSampleRate(44100);

    String SymbolName;
    String_Ctor(& SymbolName);
    GetFileName(& SymbolName, & Path);

    int WaveLen;
    float* Wave = (float*)malloc(sizeof(float) * 44100 * 10);
    PulseDescriptor PD;
    int32_t* Pulses = (int32_t*)malloc(4 * 10000);
    CVDB3 Output;

    //CSPR CNet;
    //CSPR_Ctor(& CNet);
    //String_FromChars(CSPRPath, "/tmp/x.cspr");
    //CSPR_Load(& CNet, & CSPRPath);
    //String_Dtor(& CSPRPath);

    WaveLen = LoadWaveAll(Wave, & Path);
    printf("Length: %d samples, %fs.\n", WaveLen, (float)WaveLen / SampleRate);

    float BaseFreq;
    if(Arg_F0Avaliable)
        BaseFreq = Arg_F0;
    else
        BaseFreq = GetBaseFrequencyFromWave(Wave + WaveLen / 2, 80, 1500, 13);
    printf("Fundamental Frequency: %fHz.\n", BaseFreq);

    ExtractPulsesByBaseFrequency(Pulses, & PD, Wave, BaseFreq, WaveLen);
    printf("%d pulses extracted, starts from %fs, VOT = %fs.\n",
           PD.Amount,
           (float)Pulses[0] / SampleRate,
           (float)Pulses[PD.VoiceOnsetIndex] / SampleRate);

    for(i = 1; i < PD.Amount; i ++)
        Pulses[i] -= Pulses[0];

    if(Arg_LenAvaliable)
        for(i = 1; i < PD.Amount; i ++)
            if(Pulses[i] > Arg_Len * SampleRate)
            {
                PD.Amount = i;
                break;
            }

    CVDB3_Ctor(& Output);
    memcpy(Output.Header.Identifier, "CVDB", 4);
    memset(Output.Header.Symbol, 0, 8);
    memcpy(Output.Header.Symbol, String_GetChars(& SymbolName), SymbolName.Data_Index + 1);
    Output.Header.CVDBVersion = 3;
    Output.Header.F0 = BaseFreq;
    Output.Header.PhoneType = Arg_CVAvaliable ? (Arg_C ? 'C' : 'V') : 'C';
    Output.Header.PulseNum = PD.Amount;
    Output.Header.VOI = PD.VoiceOnsetIndex;
    Output.Header.WaveSize = Pulses[PD.Amount - 1] + SampleRate / BaseFreq * 2;
    Output.PulseOffsets = (uint32_t*)realloc(Output.PulseOffsets, 4 * PD.Amount);
    Output.Wave = (float*)realloc(Output.Wave, sizeof(float) * Output.Header.WaveSize);
    memcpy(Output.PulseOffsets, Pulses, 4 * PD.Amount);
    memcpy(Output.Wave, Wave + Pulses[0], Output.Header.WaveSize * sizeof(float));
    Output.PulseOffsets[0] = 0;

    float PeriodSum = 0;
    int LPeriod = (Output.Header.PulseNum - Output.Header.VOI) / 3 + Output.Header.VOI;
    int HPeriod = (Output.Header.PulseNum - Output.Header.VOI) / 2 + Output.Header.VOI;
    PeriodSum = Output.PulseOffsets[HPeriod] - Output.PulseOffsets[LPeriod];
    Output.Header.F0 = 44100.0 / (PeriodSum / (HPeriod - LPeriod));
    printf("Adjusted F0: %f\n", Output.Header.F0);

    float* Formant = (float*)malloc(sizeof(float) * 1024);
    FormantDescriptor FormantFreq;
    FormantEnvelopeFromWave(Formant, Wave + WaveLen / 2, BaseFreq, 5000, 50, 10);

/*  Obsoleted codes for ANN-based formant extraction.

    int FIndex = - 1;
    float* LPC = (float*)malloc(sizeof(float) * 100);
    float* LPCS = (float*)malloc(sizeof(float) * 1024);
    LPCFromWave(LPC, Wave + WaveLen / 2, 1024, 50);
    SpectralEnvelopeFromLPC(LPCS, LPC, 50, 10);
    NormalizeSpectrum(LPCS, 105);
    if(BaseFreq < CNet.DividingFreq)
    {
        FeedForward_SetInput(& CNet.FFNetLow, LPCS);
        FeedForward_UpdateState(& CNet.FFNetLow);
        FIndex = GetMaxIndex(CNet.FFNetLow.Layers[2].O, CNet.ClassNum);
    }
    else
    {
        FeedForward_SetInput(& CNet.FFNetHigh, LPCS);
        FeedForward_UpdateState(& CNet.FFNetHigh);
        FIndex = GetMaxIndex(CNet.FFNetHigh.Layers[2].O, CNet.ClassNum);
    }
    free(LPC);
    free(LPCS);
    printf("F1: %f, F2:%f, F3:%f.\n",
           CNet.FormantClasses[FIndex].F1,
           CNet.FormantClasses[FIndex].F2,
           CNet.FormantClasses[FIndex].F3);
*/

    FormantFreq = AnalyzeFormantFromEnvelope(Formant, 1024);
    if(Arg_F1Avaliable)
        Output.Header.F1 = Arg_F1;
    else
        Output.Header.F1 = FormantFreq.F1;
    if(Arg_F2Avaliable)
        Output.Header.F2 = Arg_F2;
    else
        Output.Header.F2 = FormantFreq.F2;
    if(Arg_F3Avaliable)
        Output.Header.F3 = Arg_F3;
    else
        Output.Header.F3 = FormantFreq.F3;
    Output.Header.S1 = Boost_Sqr(Formant[(int)(Output.Header.F1 * 1024 / SampleRate)]) * 2;
    Output.Header.S2 = Boost_Sqr(Formant[(int)(Output.Header.F2 * 1024 / SampleRate)]) * 2;
    Output.Header.S3 = Boost_Sqr(Formant[(int)(Output.Header.F3 * 1024 / SampleRate)]) * 2;

    printf("F1 = %fHz, S1 = %f.\n", Output.Header.F1, Output.Header.S1);
    printf("F2 = %fHz, S2 = %f.\n", Output.Header.F2, Output.Header.S2);
    printf("F3 = %fHz, S3 = %f.\n", Output.Header.F3, Output.Header.S3);

    free(Formant);
    String_JoinChars(& SymbolName, ".cvdb");
    String_JoinChars(& OutputPath, "/");
    String_Join(& OutputPath, & SymbolName);
    CVDB3_Write(& OutputPath, & Output);

    printf("Saved as %s.\n", String_GetChars(& SymbolName));

    //CSPR_Dtor(& CNet);
    String_Dtor(& OutputPath);
    CVDB3_Dtor(& Output);
    free(Pulses);
    free(Wave);
    String_Dtor(& SymbolName);
    String_Dtor(& Path);
    ArrayType_ObjDtor(String, ArgStrList);
    ArrayType_Dtor(String, ArgStrList);
    return 0;
}

