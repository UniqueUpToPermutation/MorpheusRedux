#pragma once

#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <Engine/Resources/Resource.hpp>
#include <iostream>

namespace Morpheus {
	class MemoryInputStream : public std::istream
    {
    public:
		MemoryInputStream(const uint8_t* aData, size_t aLength):
			std::istream(&m_buffer), m_buffer(aData,aLength) {
			rdbuf(&m_buffer); // reset the buffer after it has been properly constructed
		}

    private:
		class MemoryBuffer: public std::basic_streambuf<char>
		{
		public:
			MemoryBuffer(const uint8_t* aData,size_t aLength)
			{
				setg((char*)aData,(char*)aData,(char*)aData + aLength);
			}
		};

    	MemoryBuffer m_buffer;
    };

	void Load(cereal::PortableBinaryInputArchive& ar, RawTexture* texture);
	void Save(cereal::PortableBinaryOutputArchive& ar, const RawTexture* texture);

	void Load(cereal::PortableBinaryInputArchive& ar, RawGeometry* geometry);
	void Save(cereal::PortableBinaryOutputArchive& ar, const RawGeometry* geometry);
}