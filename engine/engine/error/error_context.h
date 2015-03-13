#ifndef INC_ERROR_CONTEXT_H_
#define INC_ERROR_CONTEXT_H_


class CErrorContext
{
public:
	CErrorContext(const char *name, const char *data);
	~CErrorContext();
};

extern std::vector<const char *> _error_context_name;
extern std::vector<const char *> _error_context_data;

#endif
