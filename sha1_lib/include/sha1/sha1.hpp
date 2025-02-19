/*
    sha1.hpp - source code of

    ============
    SHA-1 in C++
    ============

    100% Public Domain.

    Original C Code
        -- Steve Reid <steve@edmweb.com>
    Small changes to fit into bglibs
        -- Bruce Guenter <bruce@untroubled.org>
    Translation to simpler C++ Code
        -- Volker Diels-Grabsch <v@njh.eu>
    Safety fixes
        -- Eugene Hopkinson <slowriot at voxelstorm dot com>
    Header-only library
        -- Zlatko Michailov <zlatko@michailov.org>
    C++ optimization
        -- Dan Machado <dan-machado@yandex.com>
*/

#ifndef SHA1_HPP
#define SHA1_HPP

#include <string>
#include <cstring>

class SHA1
{
	static constexpr size_t BLOCK_INTS = 16;  /* number of 32bit integers per SHA1 block */
	static constexpr size_t BLOCK_BYTES = BLOCK_INTS * 4;
	static constexpr size_t DIGEST_SIZE = 5;

	public:
		SHA1();
		~SHA1()=default;

		/**
		 * Function call operator to calculate the sha1sum of
		 * a string. Return the sha1sum of the given string
		 *
		 * @param cstr a null terminated c-string.
		 * 
		 * @return sha1sum of the parameter cstr.
		 *
		 * */

		std::string operator()(const char* cstr);

		/**
		 * Function call operator to calculate the sha1sum of
		 * a string. Return the sha1sum of the given string
		 *
		 * @param str const ref to a std::string.
		 * 
		 * @return sha1sum of str.
		 *
		 * */
		std::string operator()(const std::string& str);

		/**
		 * Function call operator to calculate the sha1sum of a file.
		 * It returns false if an error occurs while opening the file and
		 * set the paramenter hashSum as the empty string, otherwise it return
		 * true and it sets the sha1sum of the file in the paramenter hashSum.
		 *
		 * @param fileName the name of a valid file
		 * 
		 *	@param[out] hashSum reference to a std::string where to copy
		 *       the sha1sum of the file. If a error occurred, it is set
		 *       as the empty string
		 * 
		 * @return true if the sha1sum was calculated, false if an error occurs
		 *         while opening the file.
		 * */
		bool operator()(const char* fileName, std::string& hashSum);
		bool operator()(const std::string& fileName, std::string& hashSum);


		/**
		 * Partially calculate the sha1sum of its argument. The calculation
		 * is completed by calling final(). Consecutive calls to update()
		 * will have the effect of calculating the sha1 sum of the concatenated
		 * string of its arguments. For example:
		 * SHA1 sha1;
		 * sha1.update(str1);
		 * sha1.update(str2);
		 * sha1.update(str3);
		 * sha1.final();
		 * is the same than
		 * sha1.update(str1 + str2 + str3);
		 * sha1.final();
		 * which is the same than using the function operator
		 * sha1(str1 + str2 + str3) 
		 *
		 * @param cstr null terminated string
		 *
		 * */
		void update(const char* cstr);
		void update(std::string& str);

		/**
		 * Complete the calculation of the sha1sum initiated by calls to
		 * SHA1::update.
		 *
		 * @return return the sha1sum of the string (or strings)
		 *         fed by calls to SHA1::update.
		 * */
		std::string final();

		/**
		 *	Retrieve the description of the error that occurred when trying
		 * to get the sha1sum of a file.
		 * 	
		 * */
		std::string getError();

	private:
		uint32_t m_dataBlock[BLOCK_INTS];
		uint8_t m_buffer[BLOCK_BYTES];
		uint32_t digest[DIGEST_SIZE];
		uint32_t m_digest[DIGEST_SIZE];
		uint64_t transforms;
		size_t m_dataSize;
		int m_lastError;
		const uint8_t* m_bufferPtr;

		void reset();		
		void buffer_to_block()__attribute__((hot));;
		
		template<int N, int M>
		uint32_t rol(uint32_t value)__attribute__((always_inline));

		void blk(size_t i)__attribute__((always_inline));

		/*
		* R0, R1, R2, R3, R4 are the different operations used in SHA1
		*/
		template<int v, int w, int x, int y, int z>
		void R0(const size_t i)__attribute__((always_inline));
		
		template<int v, int w, int x, int y, int z>
		void R1(const size_t i)__attribute__((always_inline));
		
		template<int v, int w, int x, int y, int z>
		void R2(const size_t i)__attribute__((always_inline));
		
		template<int v, int w, int x, int y, int z>
		void R3(const size_t i)__attribute__((always_inline));
		
		template<int v, int w, int x, int y, int z>
		void R4(const size_t i)__attribute__((always_inline));

		template<int N>
		void charTo32();

		void computeHash(const char* dataPtr, const size_t dataSize);//__attribute__((hot));

		std::string fileSha1Sum(const char* fileName);

		/*
		* Hash a single 512-bit block. This is the core of the algorithm.
		*/
		
		void transform();
};

inline SHA1::SHA1()
{
	reset();
}

inline std::string SHA1::operator()(const char* cstr)
{
	reset();
	computeHash(cstr, std::strlen(cstr));
	return final();
}

inline std::string SHA1::operator()(const std::string& str)
{
	reset();
	computeHash(str.c_str(), str.length());
	return final();
}


inline bool SHA1::operator()(const char* fileName, std::string& hashSum)
{
	reset();
	hashSum=fileSha1Sum(fileName);
	return m_lastError==0;
}

inline bool SHA1::operator()(const std::string& fileName, std::string& hashSum)
{
	return operator()(fileName.c_str(), hashSum);
}


inline void SHA1::update(const char* cstr)
{
	computeHash(cstr, std::strlen(cstr));
}

inline void SHA1::update(std::string& str)
{
	computeHash(str.c_str(), str.length());	
}

inline std::string SHA1::getError()
{
	return std::strerror(m_lastError);
}

inline void SHA1::reset()
{
	/* SHA1 initialization constants */
	digest[0] = 0x67452301;
	digest[1] = 0xefcdab89;
	digest[2] = 0x98badcfe;
	digest[3] = 0x10325476;
	digest[4] = 0xc3d2e1f0;
	
	/* Reset counters */
	transforms = 0;
	m_dataSize=0;
	m_bufferPtr=m_buffer;
}

template<int N, int M>
inline uint32_t SHA1::rol(uint32_t value)
{
	return (value << N) | (value >> M);
}

inline void SHA1::blk(size_t i)
{
	 m_dataBlock[i]=rol<1, 31>(m_dataBlock[(i+13)&15] ^ m_dataBlock[(i+8)&15] ^ m_dataBlock[(i+2)&15] ^ m_dataBlock[i]);
}

template<int v, int w, int x, int y, int z>
inline void SHA1::R0(const size_t i)
{
	m_digest[z] =m_digest[z]+ ((m_digest[w]&(m_digest[x]^m_digest[y]))^m_digest[y]) + m_dataBlock[i] + 0x5a827999 + rol<5, 27>(m_digest[v]);
	m_digest[w] = rol<30, 2>(m_digest[w]);
}

template<int v, int w, int x, int y, int z>
inline void SHA1::R1(const size_t i)
{
	blk(i);
	R0<v,w,x,y,z>(i);
}

template<int v, int w, int x, int y, int z>
inline void SHA1::R2(const size_t i)
{
	blk(i);
	m_digest[z] =m_digest[z]+ (m_digest[w]^m_digest[x]^m_digest[y]) + m_dataBlock[i] + 0x6ed9eba1 + rol<5, 27>(m_digest[v]);
	m_digest[w] = rol<30, 2>(m_digest[w]);
}

template<int v, int w, int x, int y, int z>
inline void SHA1::R3(const size_t i)
{
	blk(i);
	m_digest[z] =m_digest[z]+ (((m_digest[w]|m_digest[x])&m_digest[y])|(m_digest[w]&m_digest[x])) + m_dataBlock[i] + 0x8f1bbcdc + rol<5, 27>(m_digest[v]);
	m_digest[w] = rol<30, 2>(m_digest[w]);
}

template<int v, int w, int x, int y, int z>
inline void SHA1::R4(const size_t i)
{
	blk(i);
	m_digest[z] =m_digest[z]+ (m_digest[w]^m_digest[x]^m_digest[y]) + m_dataBlock[i] + 0xca62c1d6 + rol<5, 27>(m_digest[v]);
	m_digest[w] = rol<30, 2>(m_digest[w]);
}

template<int N>
inline void SHA1::charTo32()
{
	m_dataBlock[N] = m_bufferPtr[4*N+3] | m_bufferPtr[4*N+2]<<8 | m_bufferPtr[4*N+1]<<16 | m_bufferPtr[4*N+0]<<24;
}


#endif /* SHA1_HPP */
