#ifndef INC_DATA_SAVER_H_
#define INC_DATA_SAVER_H_

#include <sys/stat.h>

// -------------------------------------------
// A generic accumulator data
class CDataSaver {
public:
	virtual ~CDataSaver() { }
	virtual void write(const void *data, size_t nbytes) = 0;

	template< class TPOD >
	void writePOD(const TPOD& pod) {
		write(&pod, sizeof(TPOD));
	}
};

// -------------------------------------------
// A class to accumulate data in memory buffer
class CMemoryDataSaver : public CDataSaver {
	typedef unsigned char u8;
	typedef std::vector< u8 > TBuffer;
	TBuffer buffer;

public:
	CMemoryDataSaver() {
	}
	void write(const void *data, size_t nbytes) override {
		size_t old_size = buffer.size();
		buffer.resize(old_size + nbytes);
		memcpy(&buffer[old_size], data, nbytes);
	}
	size_t size() const { return buffer.size(); }
	const void* data() const { return &buffer[0]; }

	bool saveToFile(const char* filename) {
		FILE *f = fopen(filename, "wb");
		if (!f)
			return false;
		size_t bytes_saved = fwrite(data(), 1, size(), f);
		assert(bytes_saved == size());
		fclose(f);
		return true;
	}

};


#endif
