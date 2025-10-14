#ifndef FUNC_6513_H_
#define FUNC_6513_H_

double CalcEnergy( long long orig );
double CalcVrms( int orig );
double CalcIrms( int orig );
double CalcImrms( int orig );
double CalcInrms( int orig );
double CalcPower( int orig );

double CalcCosFi( int orig_w, int orig_var );

double CalcAngel( int orig_w, int orig_var );

double  CalcAngel( int orig_w );

double CalcFreq( int orig );

unsigned char _Hex2Bcd( unsigned char hex );

unsigned char Hex2Bcd_2Byte( long long hex, unsigned char *bcd );
unsigned char Hex2Bcd_3Byte( long long hex, unsigned char *bcd );

unsigned char Hex2Bcd_4Byte( long long hex, unsigned char *bcd );

unsigned char BigEndHex2Bcd_3Byte( long hex, unsigned char *bcd, double rate );


unsigned char BigEndHex2Bcd_4Byte( long hex, unsigned char *bcd, double rate );
float CalNuturalCurrent(    float ia_float,float ib_float,float ic_float,//(ABC各相电流)
                            float ph_atob_float,//(AB相电压相角)
                            float ph_atoc_float,//(CA相电压相角)
                            float AngA, float AngB, float AngC
                        );
//int Bcd2Hex_3Byte( unsigned char *data );
int BigEnd2LittleEnd( unsigned long hex ); 
time_t ConvertDmdTime( unsigned char *buf );
//jw---------------------------------
//bool RecalAnalogParam(char VmaxImax,double value,bool WriteFile);
//bool Read6513FileVmax(double *vmax_imax );
//bool Read6513FileImax(double *vmax_imax );
bool Read6513AnalogParam( );
bool Save6513ImaxVmax( int vmax, int imax );
bool NoiseFlag( float threshold, float newData, float *last5Data,int maxPointNum);
bool CompareVMAX(float voltage);
bool CompareIMAX(float current);
bool ComparePMAX(float power);

#endif
