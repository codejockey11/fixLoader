// fixLoader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <ctime>

#include "GeoMagLib.h"

CGeoMagnetic* geoMagnetic = nullptr;

FILE* fix;

errno_t err;

FILE* ofileFIX1;
FILE* ofileFIX2;
FILE* ofileFIX3;
FILE* ofileFIX4;
FILE* ofileFIX5;

char buffer[468];
char recordType[5];

char tilde = '~';
char newline[] = "\r\n";

static std::string fixId;
static std::string f;

char hemiLat;
char hemiLon;

double decimalLat;
double decimalLon;

char ll[25];;

char timeOut[10];
time_t now;
tm* ltm;

std::string result;

std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
	str.erase(0, str.find_first_not_of(chars));
	
	return str;
}

std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
	str.erase(str.find_last_not_of(chars) + 1);
	
	return str;
}

std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
	return ltrim(rtrim(str, chars), chars);
}

std::vector<std::string> split(std::string const & s, char delim)
{
	std::vector<std::string> result;
	
	std::istringstream iss(s);

	for (std::string token; std::getline(iss, token, delim); )
	{
		result.push_back(std::move(token));
	}

	return result;
}

void MakeField(std::string& str, int p, int l)
{
	str.clear();
	
	str.assign(&buffer[p], l);
	
	trim(str);
}

void WriteRecordTilde(std::string& str, FILE* ofile)
{
	fwrite(str.data(), str.length(), 1, ofile);

	fwrite(&tilde, 1, 1, ofile);
}

void WriteRecordNewline(std::string& str, FILE* ofile)
{
	fwrite(str.data(), str.length(), 1, ofile);

	fwrite(&newline, 2, 1, ofile);
}

void WriteLocation()
{
	if (fixId.length() > 5)
	{
		printf("%s\r\n", fixId.data());

		return;
	}

	WriteRecordTilde(fixId, ofileFIX1);

	//state
	MakeField(f, 34, 30);
	WriteRecordTilde(f, ofileFIX1);

	//region
	MakeField(f, 64, 2);
	WriteRecordTilde(f, ofileFIX1);

	//latitude
	MakeField(f, 66, 14);
	WriteRecordTilde(f, ofileFIX1);

	//longitude
	MakeField(f, 80, 14);
	WriteRecordTilde(f, ofileFIX1);

	//Mag Var
	MakeField(f, 66, 14);
	hemiLat = f.at(f.length() - 1);
	f.replace(f.length() - 1, 1, "0x00");
	auto partsLat = split(f, '-');

	decimalLat = atof(partsLat[0].data()) + (atof(partsLat[1].data()) / 60) + (atof(partsLat[2].data()) / 3600);
	if (hemiLat == 'S' || hemiLat == 'W')
	{
		decimalLat *= -1;
	}

	MakeField(f, 80, 14);
	hemiLon = f.at(f.length() - 1);
	f.replace(f.length() - 1, 1, "0x00");
	auto partsLon = split(f, '-');

	decimalLon = atof(partsLon[0].data()) + (atof(partsLon[1].data()) / 60) + (atof(partsLon[2].data()) / 3600);
	if (hemiLon == 'S' || hemiLon == 'W')
	{
		decimalLon *= -1;
	}

	geoMagnetic->CalculateFieldElements(decimalLat, decimalLon, 1200.0, 'F');

	memset(ll, 0x00, 25);
	sprintf(ll, "%14.2f", geoMagnetic->GeoMagneticElements->Decl);
	f.clear();
	f.append(ll);
	trim(f);

	WriteRecordTilde(f, ofileFIX1);

	//category
	MakeField(f, 94, 3);
	WriteRecordTilde(f, ofileFIX1);

	//usage
	MakeField(f, 213, 15);
	WriteRecordTilde(f, ofileFIX1);

	//nasId
	MakeField(f, 228, 5);
	WriteRecordTilde(f, ofileFIX1);

	//highArtcc
	MakeField(f, 233, 4);
	WriteRecordTilde(f, ofileFIX1);

	//lowArtcc
	MakeField(f, 237, 4);
	WriteRecordNewline(f, ofileFIX1);
}

void WriteNavaid()
{
	if (fixId.length() > 5)
	{
		return;
	}

	WriteRecordTilde(fixId, ofileFIX2);

	//navRelation
	MakeField(f, 66, 23);

	auto sr = split(f, '*');
	
	WriteRecordTilde(trim(sr[0]), ofileFIX2);
	WriteRecordTilde(trim(sr[1]), ofileFIX2);

	if (sr.capacity() == 3)
	{
		WriteRecordNewline(trim(sr[2]), ofileFIX2);
	}
	else
	{
		fwrite(&newline, 2, 1, ofileFIX2);
	}
}

void WriteILS()
{
	if (fixId.length() > 5)
	{
		return;
	}

	WriteRecordTilde(fixId, ofileFIX3);

	//ilsRelation
	MakeField(f, 66, 23);

	auto sr = split(f, '*');

	WriteRecordTilde(trim(sr[0]), ofileFIX3);
	WriteRecordTilde(trim(sr[1]), ofileFIX3);

	if (sr.capacity() == 3)
	{
		WriteRecordNewline(trim(sr[2]), ofileFIX3);
	}
	else
	{
		fwrite(&newline, 2, 1, ofileFIX3);
	}
}

void WriteRemarks()
{
	if (fixId.length() > 5)
	{
		return;
	}

	WriteRecordTilde(fixId, ofileFIX4);

	//remark
	MakeField(f, 166, 300);

	result.clear();

	for (int i = 0; i < (int)f.length(); i++)
	{
		if ((byte)f.at(i) == 126)
		{
			result.append("-");
		}
		else if (((byte)f.at(i) < 32) || ((byte)f.at(i) > 125))
		{
		}
		else
		{
			result.append(1, f.at(i));
		}
	}

	WriteRecordNewline(result, ofileFIX4);
}

void WriteCharting()
{
	if (fixId.length() > 5)
	{
		return;
	}

	WriteRecordTilde(fixId, ofileFIX5);

	//charting
	MakeField(f, 66, 22);
	WriteRecordNewline(f, ofileFIX5);
}

int main(int argc, char* argv[])
{
	err = fopen_s(&fix, "FIX.txt", "rb");

	if (err)
	{
		printf("%s\n", strerror(err));
		
		return 0;
	}

	fread_s(buffer, 468, 468, 1, fix);

	if (!feof(fix))
	{
		now = time(0);
		ltm = localtime(&now);
		sprintf(timeOut, "%4d.%03d", ltm->tm_year + 1900, ltm->tm_yday);

		geoMagnetic = new CGeoMagnetic(atof(timeOut), 'M');

		err = fopen_s(&ofileFIX1, "fixLocation.txt", "wb");
		err = fopen_s(&ofileFIX2, "fixNavaid.txt", "wb");
		err = fopen_s(&ofileFIX3, "fixIls.txt", "wb");
		err = fopen_s(&ofileFIX4, "fixRemarks.txt", "wb");
		err = fopen_s(&ofileFIX5, "fixCharting.txt", "wb");

		while (!feof(fix))
		{
			memset(recordType, 0x00, 5);
			
			memcpy(recordType, &buffer[0], 4);

			if (strcmp(recordType, "FIX1") == 0)
			{
				MakeField(fixId, 4, 30);
				WriteLocation();
			}

			if (strcmp(recordType, "FIX2") == 0)
			{
				WriteNavaid();
			}

			if (strcmp(recordType, "FIX3") == 0)
			{
				WriteILS();
			}

			if (strcmp(recordType, "FIX4") == 0)
			{
				WriteRemarks();
			}

			if (strcmp(recordType, "FIX5") == 0)
			{
				WriteCharting();
			}

			fread_s(buffer, 468, 468, 1, fix);
		}

		fclose(ofileFIX1);
		fclose(ofileFIX2);
		fclose(ofileFIX3);
		fclose(ofileFIX4);
		fclose(ofileFIX5);

		delete geoMagnetic;
	}

	fclose(fix);
}
