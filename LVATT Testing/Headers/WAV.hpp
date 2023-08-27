#pragma once

#include <filesystem>
#include <fstream>
#include <bitset>

#include <NosLib/BinaryMath.hpp>

/* float to little endian short array */
inline void f2les_array(const float* src, short* dest, int count, int normalize)
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

	std::ofstream wavWriteStream(filePath, std::ios::binary);

	char* fileSize = nullptr;
	char* fmtSubChunkSize = nullptr;
	char* typeFormat = nullptr;
	char* channelCount = nullptr;
	char* sampleRateB = nullptr;
	char* byteRate = nullptr;
	char* blockAlign = nullptr;
	char* bitsPerSampleB = nullptr;
	char* dataSubChunkSize = nullptr;

	NosLib::BinaryMaths::ArithematicToByte<uint32_t>(4+(8+16)+(8+ dataSize * 2), &fileSize);
	NosLib::BinaryMaths::ArithematicToByte<uint32_t>(16, &fmtSubChunkSize);
	NosLib::BinaryMaths::ArithematicToByte<uint8_t>(1, &typeFormat);
	NosLib::BinaryMaths::ArithematicToByte<uint8_t>(channels, &channelCount);
	NosLib::BinaryMaths::ArithematicToByte<uint32_t>(sampleRate, &sampleRateB);
	NosLib::BinaryMaths::ArithematicToByte<uint32_t>((sampleRate*channels*bitsPerSample)/8, &byteRate);
	NosLib::BinaryMaths::ArithematicToByte<uint8_t>((channels*bitsPerSample)/8, &blockAlign);
	NosLib::BinaryMaths::ArithematicToByte<uint8_t>(bitsPerSample, &bitsPerSampleB);
	NosLib::BinaryMaths::ArithematicToByte<uint32_t>(dataSize*2, &dataSubChunkSize);

	wavWriteStream.write("RIFF", 4);
	wavWriteStream.write(fileSize, 4);
	wavWriteStream.write("WAVE", 4);

	/* fmt Sub Chunk */
	wavWriteStream.write("fmt ", 4);
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

	short* out = new short[dataSize];

	f2les_array(data,out, dataSize, 1);
	for (int i = 0; i < dataSize; i++)
	{
		wavWriteStream.write(reinterpret_cast<char*>(out+i), 2);
	}

	wavWriteStream.close();
}