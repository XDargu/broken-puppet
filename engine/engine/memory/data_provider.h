#ifndef INC_DATA_PROVIDER_H_
#define INC_DATA_PROVIDER_H_

// -------------------------------------------
class CDataProvider {
public:
  virtual ~CDataProvider() { }
  virtual const char* getName() const = 0;
  virtual void read(void *data, size_t nbytes) = 0;
  virtual bool isValid() const = 0;

  template< class TPOD >
  void read(TPOD& pod) {
    read(&pod, sizeof(TPOD));
  }
};

// -------------------------------------------
class CFileDataProvider : public CDataProvider {
  FILE *f;
  char filename[256];
public:
  CFileDataProvider(const char* afilename) {
    strcpy(filename, afilename);
    f = fopen(filename, "rb");
  }
  ~CFileDataProvider() {
    if (f)
      fclose(f);
  }
  const char* getName() const {
    return filename;
  }
  void read(void *data, size_t nbytes) {
    assert(isValid());
    auto bytes_read = fread(data, 1, nbytes, f);
    assert(bytes_read == nbytes);
  }
  bool isValid() const { return f != nullptr; }
};


#endif
