#include "sha1/sha1.hpp"
#include <cstring>

#ifdef _LINUX
	// for mmap:
	#include <sys/mman.h>
	#include <sys/stat.h> 
	#include <fcntl.h>
	#include <unistd.h>
#else
	#include <cstdio>
#endif


void SHA1::computeHash(const char* dataPtr, size_t dataSize)
{
	size_t idx=0;
	size_t newDataSize=dataSize;
	size_t copyB=0;
	while(true){
		copyB=BLOCK_BYTES-m_dataSize;
		if(copyB>newDataSize){
			copyB=newDataSize;
		}
		if(copyB==BLOCK_BYTES){
			m_bufferPtr=reinterpret_cast<const uint8_t*>(dataPtr+idx);
		}
		else{
			m_bufferPtr=m_buffer;
			std::memcpy(m_buffer+m_dataSize, dataPtr+idx, copyB);
		}
		idx=idx+copyB;
		newDataSize=newDataSize-copyB;
		m_dataSize=m_dataSize+copyB;
		if(m_dataSize==BLOCK_BYTES){
			buffer_to_block();	  
			transform();
		}
		else {
			break;
		}
		m_dataSize=0;
	}	
}

#ifdef _LINUX

std::string SHA1::fileSha1Sum(const char* fileName)
{
	m_lastError=0;

	if(access(fileName, F_OK)<0){
		m_lastError=errno;
		return "";
	}
	if(access(fileName, R_OK)<0){
		m_lastError=errno;
		return "";
	}

	int fd= open(fileName, O_RDONLY);	
	struct stat sb;
	if(fstat(fd, &sb) == -1) {
		close(fd);
		m_lastError=errno;
		return "";
	}

	if(sb.st_size>0){
		void* data=mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if(data==MAP_FAILED){
			close(fd);
			m_lastError=errno;
			return "";
		}
		else{
			computeHash(static_cast<const char*>(data), sb.st_size);	
		}
		munmap(data, sb.st_size);
	}
	close(fd);
	return final();
}

#else // For windows

std::string SHA1::fileSha1Sum(const char* fileName)
{
	m_lastError=0;
	std::FILE* fd = std::fopen(fileName, "r");
   if(fd==nullptr){
		m_lastError=errno;
		return "";
	}

	char* bf=reinterpret_cast<char*>(m_buffer);
	while(true){
		m_dataSize = std::fread(bf, 1, BLOCK_BYTES, fd);

		if(m_dataSize != BLOCK_BYTES){
			break;
		}
	
		buffer_to_block();	  
		transform();		
	}
	std::fclose(fd);
	return final();
}

#endif

void SHA1::transform()
{
	std::memcpy(m_digest, digest, 5*sizeof(uint32_t));
	
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0<0, 1, 2, 3, 4>( 0);
	R0<4, 0, 1, 2, 3>( 1);
	R0<3, 4, 0, 1, 2>( 2);
	R0<2, 3, 4, 0, 1>( 3);
	R0<1, 2, 3, 4, 0>( 4);
	R0<0, 1, 2, 3, 4>( 5);
	R0<4, 0, 1, 2, 3>( 6);
	R0<3, 4, 0, 1, 2>( 7);
	R0<2, 3, 4, 0, 1>( 8);
	R0<1, 2, 3, 4, 0>( 9);
	R0<0, 1, 2, 3, 4>(10);
	R0<4, 0, 1, 2, 3>(11);
	R0<3, 4, 0, 1, 2>(12);
	R0<2, 3, 4, 0, 1>(13);
	R0<1, 2, 3, 4, 0>(14);
	R0<0, 1, 2, 3, 4>(15);
	R1<4, 0, 1, 2, 3>( 0);
	R1<3, 4, 0, 1, 2>( 1);
	R1<2, 3, 4, 0, 1>( 2);
	R1<1, 2, 3, 4, 0>( 3);
	R2<0, 1, 2, 3, 4>( 4);
	R2<4, 0, 1, 2, 3>( 5);
	R2<3, 4, 0, 1, 2>( 6);
	R2<2, 3, 4, 0, 1>( 7);
	R2<1, 2, 3, 4, 0>( 8);
	R2<0, 1, 2, 3, 4>( 9);
	R2<4, 0, 1, 2, 3>(10);
	R2<3, 4, 0, 1, 2>(11);
	R2<2, 3, 4, 0, 1>(12);
	R2<1, 2, 3, 4, 0>(13);
	R2<0, 1, 2, 3, 4>(14);
	R2<4, 0, 1, 2, 3>(15);
	R2<3, 4, 0, 1, 2>( 0);
	R2<2, 3, 4, 0, 1>( 1);
	R2<1, 2, 3, 4, 0>( 2);
	R2<0, 1, 2, 3, 4>( 3);
	R2<4, 0, 1, 2, 3>( 4);
	R2<3, 4, 0, 1, 2>( 5);
	R2<2, 3, 4, 0, 1>( 6);
	R2<1, 2, 3, 4, 0>( 7);
	R3<0, 1, 2, 3, 4>( 8);
	R3<4, 0, 1, 2, 3>( 9);
	R3<3, 4, 0, 1, 2>(10);
	R3<2, 3, 4, 0, 1>(11);
	R3<1, 2, 3, 4, 0>(12);
	R3<0, 1, 2, 3, 4>(13);
	R3<4, 0, 1, 2, 3>(14);
	R3<3, 4, 0, 1, 2>(15);
	R3<2, 3, 4, 0, 1>( 0);
	R3<1, 2, 3, 4, 0>( 1);
	R3<0, 1, 2, 3, 4>( 2);
	R3<4, 0, 1, 2, 3>( 3);
	R3<3, 4, 0, 1, 2>( 4);
	R3<2, 3, 4, 0, 1>( 5);
	R3<1, 2, 3, 4, 0>( 6);
	R3<0, 1, 2, 3, 4>( 7);
	R3<4, 0, 1, 2, 3>( 8);
	R3<3, 4, 0, 1, 2>( 9);
	R3<2, 3, 4, 0, 1>(10);
	R3<1, 2, 3, 4, 0>(11);
	R4<0, 1, 2, 3, 4>(12);
	R4<4, 0, 1, 2, 3>(13);
	R4<3, 4, 0, 1, 2>(14);
	R4<2, 3, 4, 0, 1>(15);
	R4<1, 2, 3, 4, 0>( 0);
	R4<0, 1, 2, 3, 4>( 1);
	R4<4, 0, 1, 2, 3>( 2);
	R4<3, 4, 0, 1, 2>( 3);
	R4<2, 3, 4, 0, 1>( 4);
	R4<1, 2, 3, 4, 0>( 5);
	R4<0, 1, 2, 3, 4>( 6);
	R4<4, 0, 1, 2, 3>( 7);
	R4<3, 4, 0, 1, 2>( 8);
	R4<2, 3, 4, 0, 1>( 9);
	R4<1, 2, 3, 4, 0>(10);
	R4<0, 1, 2, 3, 4>(11);
	R4<4, 0, 1, 2, 3>(12);
	R4<3, 4, 0, 1, 2>(13);
	R4<2, 3, 4, 0, 1>(14);
	R4<1, 2, 3, 4, 0>(15);

	/* Add the working vars back into digest[] */
	digest[0] =digest[0]+ m_digest[0];
	digest[1] =digest[1]+ m_digest[1];
	digest[2] =digest[2]+ m_digest[2];
	digest[3] =digest[3]+ m_digest[3];
	digest[4] =digest[4]+ m_digest[4];
	
	/* Count the number of transformations */
	transforms++;
}

void SHA1::buffer_to_block()
{
	charTo32<0>();
	charTo32<1>();
	charTo32<2>();
	charTo32<3>();
	charTo32<4>();
	charTo32<5>();
	charTo32<6>();
	charTo32<7>();
	charTo32<8>();
	charTo32<9>();
	charTo32<10>();
	charTo32<11>();
	charTo32<12>();
	charTo32<13>();
	charTo32<14>();
	charTo32<15>();
}

/*
 * Add padding and return the message digest.
 */
std::string SHA1::final()
{
	/* Total number of hashed bits */
	uint64_t total_bits = (transforms*BLOCK_BYTES + m_dataSize) * 8;

	if(m_dataSize<BLOCK_BYTES){
		std::memset(m_buffer+m_dataSize, 0, BLOCK_BYTES-m_dataSize);
		m_buffer[m_dataSize]=static_cast<char>(0x80);
		buffer_to_block();
	}

	if (m_dataSize+1 > BLOCK_BYTES - 8){
		transform();
		std::memset(m_dataBlock, 0, sizeof(uint32_t)*(BLOCK_INTS - 2));
	}

	/* Append total_bits, split this uint64_t into two uint32_t */
	m_dataBlock[BLOCK_INTS - 1] = static_cast<uint32_t>(total_bits);
	m_dataBlock[BLOCK_INTS - 2] = static_cast<uint32_t>(total_bits >> 32);
	transform();
	
	/* Hex std::string */
	char resultBuffer[41];
	for(size_t i = 0; i < DIGEST_SIZE; i++){
		sprintf(resultBuffer+8*i, "%08x", digest[i]);
	}
	reset();
	return resultBuffer;
}

