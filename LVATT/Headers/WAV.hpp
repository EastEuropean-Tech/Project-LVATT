#pragma once

#include <filesystem>
#include <fstream>
#include <bitset>

#include "../NosLib/Byte.hpp"

#include "Common.hpp"

/* float to little endian short array */
inline void f2les_array(const float* src, uint16_t* dest, int count, int normalize)
{
	unsigned char* ucptr;
	float			normfact;
	int				value;

	normfact = normalize ? (1.0 * 0x7FFF) : 1.0;

	for (int i = 0; i < count; i++)
	{
		ucptr = (unsigned char*)&dest[i];
		value = lrintf(src[i] * normfact);
		ucptr[0] = value;
		ucptr[1] = value >> 8;
	};
}

/* will write data to Wav file */

void WriteData(const std::filesystem::path& filePath, float* data, const size_t& dataSize, const uint8_t& channels, const uint32_t& sampleRate)
{
	const uint8_t bitsPerSample = 16;

	if (dataSize % channels != 0)
	{
		throw std::invalid_argument("channels don't fit into data size (maybe the wrong channel count was picked)");
		return;
	}

	std::ofstream wavWriteStream(filePath, std::ios::binary | std::ios::trunc);

	/* create nullptr char arrays (char is 1 byte, so you can use it as bytes) */
	char* fileSize = nullptr;
	char* fmtSubChunkSize = nullptr;
	char* typeFormat = nullptr;
	char* channelCount = nullptr;
	char* sampleRateB = nullptr;
	char* byteRate = nullptr;
	char* blockAlign = nullptr;
	char* bitsPerSampleB = nullptr;
	char* dataSubChunkSize = nullptr;

	/* convert all data into binary in the format that WAV needs */
	NosLib::Byte::ArithematicToByte<uint32_t>(4+(8+16)+(8+ dataSize * 2), &fileSize);
	NosLib::Byte::ArithematicToByte<uint32_t>(16, &fmtSubChunkSize);
	NosLib::Byte::ArithematicToByte<uint8_t>(1, &typeFormat);
	NosLib::Byte::ArithematicToByte<uint8_t>(channels, &channelCount);
	NosLib::Byte::ArithematicToByte<uint32_t>(sampleRate, &sampleRateB);
	NosLib::Byte::ArithematicToByte<uint32_t>((sampleRate*channels*bitsPerSample)/8, &byteRate);
	NosLib::Byte::ArithematicToByte<uint8_t>((channels*bitsPerSample)/8, &blockAlign);
	NosLib::Byte::ArithematicToByte<uint8_t>(bitsPerSample, &bitsPerSampleB);
	NosLib::Byte::ArithematicToByte<uint32_t>(dataSize*2, &dataSubChunkSize);

	/* start writing all the header data into file */

	wavWriteStream.write("RIFF", 4);
	wavWriteStream.write(fileSize, 4);
	wavWriteStream.write("WAVE", 4);

	/* fmt Sub Chunk */
	wavWriteStream.write("fmt ", 4); /* yes, the space in that text has to be there */
	wavWriteStream.write(fmtSubChunkSize, 4);
	wavWriteStream.write(typeFormat, 2);
	wavWriteStream.write(channelCount, 2);
	wavWriteStream.write(sampleRateB, 4);
	wavWriteStream.write(byteRate, 4);
	wavWriteStream.write(blockAlign, 2);
	wavWriteStream.write(bitsPerSampleB, 2);

	/* data Sub Chunk */
	wavWriteStream.write("data", 4);
	wavWriteStream.write(dataSubChunkSize, 4);

	uint16_t* out = new uint16_t[dataSize];

	f2les_array(data,out, dataSize, 1);
	for (int i = 0; i < dataSize; i++)
	{
		wavWriteStream.write(reinterpret_cast<char*>(out+i), 2);
	}

	wavWriteStream.close();
}

/* little endian short to float array */
inline void les2f_array(const uint16_t* src, float* dest, int count, float normfact)
{
	short	value;

	for (int i = 0; i < count; i++)
	{
		value = src[i];
		dest[i] = ((float)value) * normfact;
	};
}

ArrayWrapper<float> ReadFile(const std::filesystem::path& filePath)
{
	std::ifstream wavReadStream(filePath, std::ios::binary);

	/* create char arrays (char is 1 byte, and so can be used as a byte array) */

	char fileSize[4];
	char fmtSubChunkSize[4];
	char typeFormat[2];
	char channelCount[2];
	char sampleRateB[4];
	char byteRate[4];
	char blockAlign[2];
	char bitsPerSampleB[2];
	char dataSubChunkSize[4];

	/* used for all text fields, I don't care about their data but you have to put is some where */
	char textValidation[4];

	/* read all the data from file */

	wavReadStream.read(textValidation, 4);
	//if (!NosLib::Byte::ByteCompare(textValidation, "RIFF", 4))
	//{
	//	printf("RIFF section doesn't line up\n");
	//	return 0;
	//}

	wavReadStream.read(fileSize, 4);
	wavReadStream.read(textValidation, 4);
	//if (!NosLib::Byte::ByteCompare(textValidation, "WAVE", 4))
	//{
	//	printf("format doesn't match, should be WAVE, instead got %s\n", textValidation);
	//	return 0;
	//}

	/* fmt Sub Chunk */
	wavReadStream.read(textValidation, 4);
	//if (!NosLib::Byte::ByteCompare(textValidation, "fmt ", 4) )
	//{
	//	printf("fmt section doesn't line up\n");
	//	return 0;
	//}

	wavReadStream.read(fmtSubChunkSize, 4);
	wavReadStream.read(typeFormat, 2);
	wavReadStream.read(channelCount, 2);
	wavReadStream.read(sampleRateB, 4);
	wavReadStream.read(byteRate, 4);
	wavReadStream.read(blockAlign, 2);
	wavReadStream.read(bitsPerSampleB, 2);

	/* data Sub Chunk */
	wavReadStream.read(textValidation, 4);
	//if (!NosLib::Byte::ByteCompare(textValidation, "data", 4))
	//{
	//	printf("Data section doesn't line up, instead got %s\n", textValidation);
	//	return 0;
	//}
	wavReadStream.read(dataSubChunkSize, 4);

	/* convert all the data we care about from byte to int */

	uint8_t channels = NosLib::Byte::ByteToArithematic<uint8_t>(channelCount);
	uint32_t sampleRate=  NosLib::Byte::ByteToArithematic<uint32_t>(sampleRateB);
	uint32_t dataSize = NosLib::Byte::ByteToArithematic<uint32_t>(dataSubChunkSize)/2;

	/* read the rest of the data into uint16_t (same size as float16) */

	uint16_t* in = new uint16_t[dataSize];
	for (int i = 0; i < dataSize; i++)
	{
		wavReadStream.read(reinterpret_cast<char*>(in + i), 2);
	}
	ArrayWrapper<float> outArray(dataSize);
	/* convert from uint16_t (float16) to float (float32) */
	les2f_array(in, outArray.data, dataSize, 1);

	return outArray;
}